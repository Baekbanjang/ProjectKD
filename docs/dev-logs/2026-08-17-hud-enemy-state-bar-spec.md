# 2026-08-17 — 적 상태 바 HUD 설계 (SB 실측 기반) + 총구 이펙트

**코드 변경 0.** 에셋 = 총구 이펙트 배선(몽타주 16개) / 나머지는 조사·설계.

> ⚠️ 원래 `docs/design/` 에 둘 문서인데 그 폴더가 쓰기 권한 밖이라 dev-log 로 남긴다. 나중에 옮길 것.

---

## 1. 총구 이펙트 — 완료

### 결론 = 유료 팩 안 사도 된다

메모리 `reference_gun_asset_sources` 에 적혀 있던 예측("`NS_Hit` 계열 파라미터 조정으로 대체 가능성")이 **맞았다.**

```
NS_SB_Hit_Fire_Once   /Game/SB_Style_GameProject/Effect/
                      = NS_Hit_Fire_Once 복제 + Flare 이미터 끔
                        + Size_Spark / Size Min_Fire / Size Max_Fire 각각 절반 (작성자 작업)
배선   ANS_WeaponTrail  tag=Gun / socket=Muzzle / dur 0.10
       발사 노티보다 3프레임(0.05s) 앞
대상   16개 몽타주 / 25발 (지상 콤보 + 조준 사격 + 공중 1·3타)
트랙   Muzzle (신규, 검 트레일 트랙과 분리)
```

### ★ GC 로는 총구에 못 붙인다 (엔진 실측)

```cpp
// GameplayCueNotifyTypes.cpp:337  UE5.6  Plugins/Runtime/GameplayAbilities
TargetComponent = (TargetCharacter ? TargetCharacter->GetMesh() : TargetActor->GetRootComponent());
```

GC 의 소켓 부착은 **캐릭터 스켈레탈 메시만** 본다. 총 메시의 `Muzzle` 소켓을 가리킬 방법이 없다.
→ `ANS_WeaponTrail.cpp:22~46` 이 컴포넌트 태그로 무기 메시를 찾는 **유일한 경로**다. 검 트레일 27개가 이미 이 방식으로 돈다.

### PIE 결과 (1차, NS_Hit_Fire_Once 기준)

```
1  총구에서 나온다            ✅
2  타이밍이 늦다              → 3프레임 앞당김 (반동으로 튕길 때 나왔음)
3  베는 호 형상이 보인다       → 에셋이 검격 착탄용이라 당연. Flare 끄고 크기 절반으로 대응
```

2차(NS_SB_Hit_Fire_Once) PIE 는 **미검증.**

### 재사용할 방법 — Niagara 파라미터 캐기

Niagara 내부는 Python 에 안 열린다. **uasset 바이너리에서 이름표를 캘 수 있다.**

```bash
grep -aoE "User\.[A-Za-z0-9_ ]+" 에셋.uasset | sort -u
```

`NS_Hit_Fire_Once` 실측 = `Brightness` `Color` `Lifetime Spark` `Lifetime_Fire` `Lifetime_Flare`
`Size Max_Fire` `Size Min_Fire` `Size_Flare` `Size_Spark` `Spawn Count_Fire` `Spawn Count_Spark` `Velocity_*`

⚠️ **기본값은 이 방법으로 못 읽는다** — 에디터에서 봐야 한다.
⚠️ `SetVariableFloat` 는 `User.` 접두어 **없이** 넘긴다(검 트레일의 `Trail Width` 와 같은 형식).

---

## 2. HUD 설계 — 적 상태 바

### 왜 이걸 먼저 만드나

```
Health / MaxHealth      데미지 계수가 전부 0 이라 아직 안 줄어든다
Poise / MaxPoise        StaggerComponent 실동작 중 = 지금 바로 움직인다   ★
Stamina                 폐기 결정 (2026-07-28)
Dosul                   길동 잔재, 무효
Ammo                    어트리뷰트 자체가 없다
```

스태거·처형이 다 굴러가는데 **언제 터지는지 눈으로 볼 방법이 없다.** SB 도 따로 뺐다(`WB_MainHUD_EnemyStaminaSlot`).

### SB 원본 구조 (`WB_MainHUD_EnemyStateBar` 실측)

```
Overlay_Root
├─ HB_StaminaSlots     HorizontalBox   조각 15개, 슬롯 패딩 Left −0.5
├─ HorizontalBox_0     실드 4개
├─ SizeBox_0           198.2 × 10   → ProgressBar_HP   translate (1.9, −13)
├─ SizeBox_2           218.25 × 20                      translate (5, −30)
├─ Overlay_HpFx        2 slots                          translate (7.5, −30)
├─ SB_MoreShadow       그림자                            translate (0, −12)
├─ VerticalBox_0       텍스트 3
├─ Overlay_Pool        데미지 숫자 5개 풀링
└─ DebugInfo           개발용                            translate (245, 0)
```

| 요소 | 값 |
|---|---|
| HP 바 | `ProgressBar` **198.2 × 10** |
| HP 브러시 | **10 × 10** 도트, `DrawAs: Image`, **`Tiling: Both`** (배경 `Hp_Dot_12bg` / 채움 `Hp_Dot_12`) |
| 프레임 | `baseFlame` 212.5 × 32 |
| 그림자 | `HUD_MoreShadow` 256 × 256, 알파 0.5 |
| 피격 FX | `img_Fx` 90 × 90, 알파 0.3 |
| Poise 조각 | 아이콘 25 × 25 → 표시 **12 × 12**, **15개** |
| 색 | Poise 켜짐 **#EFFF74** / Poise 텍스트 #EFFF73 / 실드 텍스트 #A2C3B5 |

### JSON 에서 읽히는 설계 판단 5개

1. **프레임이 바보다 크다** — 바 198.2 / 프레임 212.5. 양쪽 7px 씩 넘겨 감싼다
2. **분절 눈금을 −0.5px 겹쳐** 붙여 이음매 제거
3. **그림자를 별도 레이어로**(알파 0.5) — 배경 밝기와 무관하게 읽히게
4. **데미지 숫자 5개 풀링** — 재사용
5. **피격 FX 를 바 위 −30px** 로 빼서 바를 안 가린다

### 우리 매핑

| SB | 우리 |
|---|---|
| Stamina (적 격파 게이지) | **`Poise` / `MaxPoise`** — 개념 동일 |
| HP | `Health` / `MaxHealth` |
| Shield | 개념 없음, 생략 |
| Damage 숫자 | 나중 |

### 1차 규격

```
위치     화면 상단 중앙, 락온 대상만 (해제 시 숨김)
HP 바    ProgressBar 198 × 10
Poise    분절 12 × 12 × 15개, HorizontalBox, 패딩 Left −0.5
색       Poise #EFFF74 / HP 색 미정
그림자   Border 알파 0.5 (텍스처 대신 단색으로 시작)
```

우리 **두 번째 화면 고정 HUD** 다(첫 번째 = `WBP_Crosshair`, 08-08). `WBP_LockOnReticle` 은 `UWidgetComponent` 라 월드 스페이스로 성격이 다르다.

---

## 3. UI 소재 — 결론

**텍스처 0장으로 시작한다.** `ProgressBar` + `Border` + 색상이면 1차 규격이 다 나온다.
나중에 10 × 10 도트 한 장만 얹으면 SB 질감이 난다. **브러시만 갈아끼우는 거라 지금 안 만들어도 나중 작업이 안 늘어난다.**

### 소재별 제작 경로 (비그림쟁이 기준)

| 대상 | 방법 |
|---|---|
| 바 타일 · 9-slice · 눈금 | **UMG 머티리얼**(노드) 또는 직접 그리기 |
| 프레임 · 장식 | 머티리얼 / `Border` 도형 조합 / Figma |
| 아이콘 (128px+) | AI 생성 · CC0 팩 (Kenney Sci-Fi) |
| 3D 표면 (레벨) | 텍스처 생성 AI — **M5 때** |

### ★ SB UI 텍스처는 일러스트가 아니다

```
Hp_Dot_12        10 × 10     점 하나
LockOn_Gauge_On  14 × 14     작은 도형
Bar_Shadow       32 × 32     그라디언트
Beta_cube_00~06  조각 7종     사각형 변형
Aim_frame_01    256 × 256    선 몇 개
```

전부 점·선·사각형·그라디언트다. **좋아 보이는 이유는 그림 실력이 아니라 배치·비율·색·절제**다.

### AI 가 맞는 것 / 안 맞는 것

| | AI 적합 | AI 부적합 |
|---|---|---|
| 크기 | 크다 (128px+) | 작다 (10~32px) |
| 반복 | 한 장으로 끝 | 타일링(seamless) |
| 정밀도 | 느슨해도 됨 | 픽셀 단위 |
| 예 | 아이콘 · 장식 프레임 · 배경 패널 · 시안 | **바 타일 · 9-slice · 눈금** |

⚠️ 힉스필드 **알파(투명 배경) 지원 여부는 여전히 미확인.** 문서에 언급이 0건이고 실제 생성 시험을 안 했다.
⚠️ 텍스처 생성 AI(3D AI Studio · Polycam · WithPoly 등)는 **PBR 3D 표면용**이라 UI 엔 안 맞는다(알파 없음, 1K~8K 출력, 불필요한 노멀·러프니스 맵). **레벨 작업(M5) 때 유효.**

### 참조 도구

**[Game UI Database](https://www.gameuidatabase.com/)** — Inspector Tool 이 화면에서 **색상 헥스·폰트 크기·시각 속성**을 뽑아준다. 그리지 않고 숫자만 가져오는 용도. 1,300 게임 / 55,500 스크린.
⚠️ **봇 접근 403** — 사람이 직접 봐야 한다. 대안 = Interface In Game.

### ⚠️ 저작권

SB UI 텍스처를 빌드에 그대로 넣으면 문제가 된다(우리가 SB 스타일을 표방하므로 더 직접적). **치수·비율·색 실측과 구조 학습은 정당**하나 **가져다 쓰기는 안 된다.**
같은 이유로 `sounds.spriters-resource.com` 도 레퍼런스·임시용까지만 — 출시 전 CC0/구매본 교체 필요.

---

## 4. 미결

- **분절 개수** 고정 15 vs `MaxPoise` 비례
- **HP 바 색** — SB 는 도트 텍스처가 색을 정한다. 우리는 단색이라 별도 결정
- **바인딩 방식** — BP 에서 ASC 델리게이트 구독 vs C++ 베이스 위젯 클래스(신규 `.h` = §0 승인 필요)
- **MCP 위젯 기능 한계 미검증** — `create_widget` / `add_widget_child` 는 있으나 앵커·바인딩·애니메이션까지 되는지 안 해봤다
- 총구 2차 PIE (`NS_SB_Hit_Fire_Once`) 미검증
