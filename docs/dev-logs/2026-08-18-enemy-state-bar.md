# 적 상태 바 HUD — 우리 첫 월드 스페이스 UI

락온한 적 머리 위에 HP / Poise 를 띄운다. Poise 는 칸(카운트) 방식.

**커밋** — 소스 `13ed120`(락온 델리게이트) `4b7da4a`(상태 바 + 주석 정리) / Content `cf21589`
**설계 근거** — 볼트 `notes/Reference/StellarBlade_UI_HUD_구조.md`

---

## 1. SB 실측이 설계를 두 번 바꿨다

### 처음 안 — 화면 상단 중앙 고정

JSON 덤프에서 `WB_MainHUD_EnemyStateBar` 의 치수·색은 나왔는데 **화면 어디에 뜨는지는 안 나왔다.** `Overlay_EnemyStateBar` 가 설계시점에 비어 있어서(런타임 생성) 위치를 알 수 없었다. 그래서 "락온 대상만 상단 중앙"으로 잡았다.

### 스크린샷을 보고 뒤집음

```
일반 적    적 머리 위 월드 스페이스     WB_MainHUD_EnemyStateBar   198 x 10, 포이즈 15칸
보스       화면 상단 중앙 고정          WB_MainHUD_EnemyInfo       1318 x 45, 스태미나 33칸
```

**상단 중앙은 보스 전용이었다.** `Overlay_EnemyStateBar` 가 비어 있던 이유가 여기서 풀렸다 — 적마다 런타임에 만들어 머리 위로 붙이기 때문.

### 색도 틀렸었다

HP 채움을 빨강으로 잡았는데 실물은 **흰색 / 회색**이다. 빨강은 **플레이어가 저체력일 때** 나오는 색이었다.

> 교훈 — JSON 덤프는 "무엇이 있나"를 주고, 스크린샷은 "어떻게 보이나"를 준다. 둘 다 봐야 한다.

---

## 2. 게이지는 도트 격자다 — 연속 바가 하나도 없다

SB UI 의 정체성. HP · Beta · Shield · SP 전부 도트.

```
브러시    Hp_Dot1_5x  15 x 15  +  Tiling: Both       플레이어
          Hp_Dot_12   10 x 10  +  Tiling: Both       적
```

`SizeBox_HP` 422 x 34 ÷ 15 = **28칸 x 2.27줄** → 플레이어 HP 가 2줄로 보이던 이유.

우리는 `T_Dot10`(10x10, 가운데 8x8 흰색 + 여백 1px)을 만들어 썼다. **흰색으로 만들어 Tint 로 색을 곱한다** — 노랑·빨강·회색이 한 장으로 나온다.

### 텍스처 임포트 설정 4개

```
Compression Settings   UserInterface2D (RGBA)
Texture Group          UI
Filter                 Nearest          ← 이거 빠지면 도트가 뭉갠다
Mip Gen Settings       NoMipmaps
```

---

## 3. Poise 를 칸(카운트)으로

### 코드 변경 0줄

GAS 어트리뷰트는 **타입이 항상 float** 다(`FGameplayAttributeData`). 못 바꾼다. 대신 **값을 정수로 쓰면 그게 곧 카운트**다.

```
전    MaxPoise 100,  차감 50    화면에 칸이 안 보임. 나눗셈 필요
후    MaxPoise 3,    차감 1     3 -> 2 -> 1 -> 0. 그대로 칸 수
```

적마다 칸 수도 `MaxPoise` 하나로 정해진다 — `Arrow 2 / Sword 3 / Parry 3 / Elite 5`.

위젯 배선도 짧아진다.

```
SetTarget      SizeBox_Poise -> Set Width Override ( MaxPoise x 10 )
RefreshPoise   ProgressBar_Poise -> Set Percent ( Poise / MaxPoise )
```

### 폭이 칸 수의 배수라 반쪽 도트가 안 생긴다

`MaxPoise 3` → 폭 30, 도트 10 → `Percent 0.667` 이면 채움 20px = 정확히 도트 2개.

---

## 4. 값 전달 = GAS 델리게이트. SB 보다 한 단계 위

SB 는 `WB_MainHUD_Master` 가 **매 프레임 게임을 긁어 자식에 push** 한다(`bClassRequiresNativeTick`). 위젯 133개가 각자 게임을 뒤지지 않게 한 것이다.

우리는 그럴 필요가 없다. **GAS 가 "값이 바뀌었다"를 알려주는 통로를 이미 갖고 있다.**

```
값 변화    WaitForAttributeChanged(Actor, Attribute, OnlyTriggerOnce=false)
첫 값      GetFloatAttribute(Actor, Attribute)      델리게이트는 바뀔 때만 온다
해제       EndAction()                              Destruct 에서
```

`UAbilityAsync` 는 어빌리티 전용이 아니다 — 클래스 주석이 못박아 놨다(`AbilityAsync.h:19`).

> "they can be executed from any blueprint like an actor and are not tied to a specific ability lifespan"

**결과** — SB 의 Master 가 "방송국"이라면 우리 위젯은 각자 구독한다. 폴링이 0이다.

---

## 5. 구조 — C++ 이 존재를 보장하고 BP 가 그린다

```
UUserWidget
 └ UKDEnemyStateBarWidget          C++, 헤더만. SetTarget(AActor*) 하나
    └ WBP_EnemyStateBar            BP. 구독과 그리기 전부

AKDEnemyBaseCharacter
 └ StateBarWidget (UWidgetComponent)   C++ 생성
      Screen / DrawAtDesiredSize / NoCollision / 초기 숨김
      Widget Class 지정만 BP
```

### 왜 C++ 인가

BP 로 하면 `BP_Bandit` 과 `BP_Axe_Elite` **두 계열에 각각** 붙여야 하고, 새 적을 만들 때 빠뜨리면 **에러도 로그도 없이 바만 안 뜬다.**

C++ 이 컴포넌트와 배선을 보장하고, **에셋 지정만 BP** 가 한다. C++ 이 BP 에셋 경로를 하드코딩하지 않게 하려는 것.

### 타이밍 — 재시도 로직이 필요 없다

```
PostInitializeComponents  ->  AI 빙의  ->  PossessedBy (ASC init + DA 스탯)
                          ->  BeginPlay
                                Super::BeginPlay()
                                  └ AActor::BeginPlay 가 컴포넌트 BeginPlay 를 먼저 돌림
                                       └ UWidgetComponent::BeginPlay -> InitWidget()
                                Super 뒤 = 위젯도 어트리뷰트도 준비 완료
```

근거 = `Actor.cpp:4748~4756` / `WidgetComponent.cpp:734~738` / `Pawn.cpp:132~152`

---

## 6. 막힌 지점 4개

### `SetTarget` 을 함수로 만들면 비동기 노드가 안 나온다

**BP 함수 그래프에는 Latent / Async 노드를 못 놓는다.** `Wait for Attribute Changed` 가 우클릭 목록에 아예 안 떴던 이유. `Delay` 를 함수에 못 넣는 것과 같다.

→ **커스텀 이벤트로** 만들어야 한다. `RefreshPoise` / `RefreshHealth` 는 계산만 하니 함수로 둬도 된다.

### 리페어런트 시 이름 충돌

BP 의 커스텀 이벤트 `SetTarget` 과 부모의 `BlueprintImplementableEvent SetTarget` 이 겹친다.
→ 커스텀 이벤트 노드를 **지우고** 우클릭 → `Event Set Target` 으로 다시 만든 뒤 노드 재연결.

### ★ 비어 있는 `Event On Targeted` 가 C++ 를 죽인다

`BP_Bandit` 에 **연결이 하나도 없는** `Event On Targeted` 노드가 있었다. `OnTargeted` 는 `BlueprintNativeEvent` 라 **BP 가 구현하면 C++ `_Implementation` 을 대체한다.** 빈 BP 가 덮어써서 `SetVisibility` 가 영영 안 불렸다.

```
Event BeginPlay      = ReceiveBeginPlay (BlueprintImplementableEvent)
                       AActor::BeginPlay 가 불러줌. C++ 를 대체 X
Event On Targeted    = BlueprintNativeEvent 구현
                       C++ _Implementation 을 대체 O
```

→ 노드를 지우거나, 노드 위 우클릭 → **부모 함수 호출 추가**(`Parent: On Targeted`).

### `Width Override` 가 부모 슬롯에 끌려 늘어난다

`SizeBox_Poise` 폭을 30 으로 잡았는데 도트가 20개 나왔다. `VerticalBox` 슬롯의 **`Horizontal Alignment` 기본값이 `Fill`** 이라 SizeBox 가 부모 폭(200)까지 늘어난 것.

→ 슬롯 `Horizontal Alignment = Left`. **`Width Override` 는 "원하는 크기"일 뿐 부모가 Fill 이면 무시된다.**

---

## 7. 실측 치수 (SB 원본)

| | 크기 | 분절 |
| --- | --- | --- |
| 플레이어 HP | 422 x 34 | — (도트 28 x 2줄) |
| 플레이어 Beta | 422 | 10칸 |
| 플레이어 Burst | — | 21칸 |
| 타키 게이지 | 1215 x 46 | — |
| 보스 HP | 1318 x 45 | 스태미나 33칸 |
| **일반 적 HP** | **198.2 x 10** | **포이즈 15칸 / 실드 4칸** |

색 — Poise `#EFFF74` / 락온 마크 `#CCFFFFFF` / 적 HP 채움 흰색

### 적 바는 3층이다

```
ProgressBar_HP                     HP
WB_MainHud_ShieldSmall_1~4         SH   4칸        ← 우리는 1차에서 생략
WB_MainHUD_EnemyStaminaSlot_1~15   포이즈 15칸
```

플레이어 바이탈(`BE / HP / SH`)과 같은 3층 구조. **실드는 "두 번째 체력"이 아니다** — `ShieldRegenPerSecond` 로 재생하고 `DamageReductionPerShieldBock` 로 피해를 깎으며 0이 되면 전용 리액션(`ReactionWhenShieldZero`)이 난다. 우리 `GA_Parry` 의 Block GE(가드 중 50% 경감)와 같은 목적을 다른 방식으로 하는 것.

---

## 8. 검증 (PIE)

```
락온 -> 적 머리 위에 바 표시            통과
락온 해제 -> 사라짐                     통과
MaxPoise 별 칸 수 (3 / 2 / 5)          통과
공격 -> 노란 칸 감소                    통과
칸 0 -> 경직 진입                       통과
경직 복귀 -> 칸 복원                    통과
```

마지막 항목이 `SetNumericAttributeBase` 도 어트리뷰트 변경 델리게이트를 쏜다는 확인이다(`GameplayEffect.cpp:3790`).

**HP 바는 안 움직인다** — 콤보 데미지 계수가 전부 0. 위젯 문제가 아니다.

---

## 9. 남은 것

```
패링 미동작           Poise 를 원래 패링으로 깎게 돼 있는데 패링 자체가 안 켜진다.
                      검증하려고 DA_Sword_Bandit 만 Ability.Player.Light 로 바꿔뒀다.
                      적 작업 때 되돌린다
Shield (SH) 바        적 바 3층 중 가운데. 전투 시스템 작업이라 나중
HP 바                 콤보 데미지 계수 0
플레이어 HUD          BE / HP / SH 3줄 + 좌하단. 아직 위젯 0개
칸별 연출             SB 는 칸마다 On_Ani / Off_Ani 를 단다.
                      우리는 ProgressBar 라 칸 단위 애니메이션이 안 된다.
                      juice 가 필요해지면 Image N개 방식으로 갈아탄다
```
