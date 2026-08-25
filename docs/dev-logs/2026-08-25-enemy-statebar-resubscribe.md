# 2026-08-25 — 적 상태 바가 재락온 후 안 움직이던 버그

락온을 한 번 풀었다 다시 걸면 그 적의 HP·Poise 바가 영영 갱신되지 않았다. **08-18 상태 바 구현 때부터 있던 버그**를 오늘 잡았다.

**대상** — `WBP_EnemyStateBar` (`/Game/SB_Style_GameProject/UI/Enemy/`)
**관련** — `2026-08-18-enemy-state-bar.md` (구현 당시 기록)

---

## 1. 증상 — "됐다 안됐다"

바는 뜨는데 때려도 값이 안 변한다. 그런데 어떤 적은 되고 어떤 적은 안 된다. 재현 조건이 안 잡혀 한동안 헤맸다.

## 2. 원인 — `Construct` 가 비어 있었다

```
Destruct   -> End Action x3      구독 끊기      있음
Construct  -> (연결 0개)          구독 잇기      없음
```

구독을 시작하는 곳이 `SetTarget` 하나뿐인데, 그건 C++ `AKDEnemyBaseCharacter::BeginPlay` 가 **스폰 때 1회만** 부른다.

```
스폰       SetTarget   구독 3개 시작
첫 락온    표시        갱신 O
락온 해제  Destruct    End Action x3 로 구독 전멸
재락온     Construct   연결이 없어 아무 일 X
                       SetTarget 은 다시 안 불림
           -> 바는 뜨는데 값이 영영 안 움직임
```

**적마다 락온 해제 이력이 달라서 "됐다 안됐다"로 보였다.** 08-18 검증 목록에 "해제 후 **재**락온"이 없어 그날 안 잡혔다.

## 3. 진단이 오래 걸린 이유 — 정적 분석 7가설 전부 기각

하청(sonnet) 으로 돌린 진단이 아래를 전부 기각했다. **코드는 처음부터 무죄였다.**

| 가설 | 판정 | 근거 |
| --- | --- | --- |
| `SetTarget` 타이밍 레이스 | 기각 | `ACharacter`/`APawn::BeginPlay` 가 pass-through 라 `AActor::BeginPlay` 의 컴포넌트 순회(`Actor.cpp:4748~4756`)가 항상 먼저. 배치·PIE시작·런타임스폰 셋 다 동일 |
| 빈 `Event On Targeted` 노드 | 기각 | 적 BP 6개 전수 조회 — 전부 없음. 08-18 의 궁수 2종 결함은 이후 제거됨 |
| `WaitForAttributeChanged` ASC 실패 | 기각 | 적 ASC 는 생성자 소유라 null 구간 없음 |
| 값 불변 시 델리게이트 스킵 | 기각 | `GameplayEffect.cpp:3765~3801` 에 `NewValue == OldValue` 게이트 없음 |
| B2 리팩토링 영향 | 기각 | diff 대조 — 어트리뷰트 차감 경로 변경 0 |

> 교훈 — **정적 분석이 전부 "정상"이라고 할 때는 정적 대상이 아닌 것을 의심한다.** 여기선 이벤트 그래프의 *비어 있는 노드* 였다. 없는 것은 grep 에 안 걸린다.

## 4. 엔진 실측 — 가시성 토글은 위젯 객체를 안 죽인다

수정안이 성립하려면 재표시 때 **같은 위젯 객체**가 돌아와야 한다(`TargetActor` 변수가 살아 있어야 함). 확인했다.

```
WidgetComponent.cpp:1303   IsVisible() && !Owner->IsHidden()
              :1348          ScreenLayer->AddComponent(this)      -> Construct
              :1523          ScreenLayer->RemoveComponent(this)   -> Destruct

SWorldWidgetScreenLayer.cpp:85   RemoveComponent = 캔버스 슬롯만 제거
WidgetComponent.cpp:749 / :1641  Widget = nullptr 은 EndPlay 와 WidgetClass 변경 때만
```

**`SetVisibility(false)` 로는 `UUserWidget` 이 안 죽는다.** 변수도 그대로 살아남는다.

## 5. 수정 — 노드는 안 옮기고 실행선만 갈아끼움

BP 는 노드 위치가 의미 없으므로 실행 핀 재연결만으로 끝난다. **끊기 3 / 잇기 3.**

```
SetTarget    Set TargetActor -> 시퀀스
  then_0       Get MaxPoise  -> Set MaxPoiseCache  -> Set Width Override   (끝)
  then_1       Get MaxHealth -> Set MaxHealthCache                          (끝)
  then_2       Get MaxShield -> Set MaxShieldCache                          (끝)
             1회면 되는 것만 남김

Construct    Is Valid(TargetActor) -> 시퀀스
  then_0       RefreshPoise  -> Wait(Poise)  -> Set AsyncPoise
  then_1       RefreshHealth -> Wait(Health) -> Set AsyncHealth
  then_2       Set Percent   -> Wait(Shield) -> Set AsyncShield -> Branch -> SizeBox_Shield 표시

Destruct     End Action x3   (변경 없음)
```

**현재값 재조회(`Get Float Attribute` -> `Refresh`)를 `Construct` 쪽에 둔 것이 중요하다.** `Wait` 는 변화 시점에만 오므로, 숨겨진 동안 깎인 체력이 재락온 순간 옛 값으로 보인다.

`Get Float Attribute` 는 순수 노드라 실행선이 없다. 자리에 그대로 두면 호출되는 경로에서 다시 계산된다.

## 6. ★ 새로 안 함정 4개

### 6-1. `Destruct` 에서 끊었으면 `Construct` 에서 이어야 한다

UMG 가 짝을 보장하는 쌍이다. 한쪽만 두면 재표시 때 죽는다. **`Wait for Attribute Changed` 를 쓰는 위젯 전부에 해당** — 플레이어 HUD 에서도 같은 함정을 밟는다.

### 6-2. BP 실행 입력 핀은 연결을 여러 개 받는다

새 선만 꽂으면 옛 선이 남아 **두 경로에서 다 실행된다.** 구독 노드면 그게 이중 구독이고, `Async` 변수는 나중 것으로 덮여 **먼저 것이 `End Action` 을 영영 못 받는다**(ASC 델리게이트에 붙은 채 잔류). 반드시 명시적으로 Break Link.

### 6-3. BP 디버거 핀 값 = 그 노드가 마지막으로 실행된 시점 값

1회성 줄기(`SetTarget`)의 `Get Float Attribute` 는 스폰 후 다시 안 돈다. 때려도 안 바뀌는 게 정상. **이걸 "값이 안 들어온다"로 읽으면 오진한다** — 실제로 오늘 그 갈림길에 섰다.

### 6-4. 디버그 오브젝트 드롭다운

BP 에디터 툴바에서 인스턴스를 안 고르면 브레이크포인트도 핀 값도 안 나온다. 적이 여럿이면 엉뚱한 개체를 보게 된다. 이것도 "됐다 안됐다"를 만든다.

## 7. 곁가지 — 적 DA 전수 실측으로 보류 13번이 닫혔다

```
DA_Sword_Bandit / Parry_Bandit / Arrow_Bandit / Axe_Elite / Dummy_Stagger
   PoiseDamageByAttack = { Ability.Player.Parry : 1.0 }     전부 이 한 줄뿐
DA_Dummy_Immortal
   PoiseDamageByAttack = { }                                 비어 있음
```

`CURRENT.md` 보류 13 = "python 조회 결과 비어 보이나 조회 한계일 수 있음" -> **조회 한계가 아니었다.** `tag_name` 으로 풀어 읽으니 실제로 키가 하나였다.

**결론 — 평타로 Poise 가 안 깎이는 건 현재 데이터 의도다.** 08-18 의 "공격 -> 노란 칸 감소 통과"는 그때 임시로 넣은 `Ability.Player.Light` 덕분이었고 그건 원복됐다(08-18 dev-log §9 에 "적 작업 때 되돌린다"고 적혀 있다).

> ⚠️ **UI 검증은 `BP_Bandit`(MaxHealth 200)으로 한다.** `DA_Dummy_Immortal` 은 99999 라 데미지 20 이 Percent 0.0002 = 198px 바에서 **0.04px**. 값은 정상인데 안 움직이는 걸로 보인다.

## 8. 남은 것

```
Poise 설계 결정    평타로도 깎을지 / 패링 전용 유지할지 (미결)
플레이어 HUD       착수 시 6-1 함정 선제 적용
```
