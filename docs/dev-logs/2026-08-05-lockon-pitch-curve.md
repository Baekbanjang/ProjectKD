# 2026-08-05 — 락온 시점 고정 버그 + 거리별 피치 커브 (SB 방식)

## 증상

락온을 걸면 **시점이 특정 위치로 빨려들어 고정**됐다. 마우스로 밀어도 도로 끌려왔다.
카메라 레일 작업(2026-08-03) 이후 생긴 것으로, 그때 락온 쪽을 같이 안 봤다.

## 근본 원인 — 피치의 뜻이 바뀐 걸 락온이 모른다

카메라 개편 이후 `ControlRotation.Pitch`를 읽는 곳은 **딱 하나**다.

```cpp
// KDSpringArmComponent.cpp:74 — 레일 눈금
const float Pitch = FRotator::NormalizeAxis(OwnerController->GetControlRotation().Pitch);
const float Alpha = FMath::Clamp((Pitch - PitchAtStart) / (PitchAtEnd - PitchAtStart), 0.f, 1.f);
```

`bUseControllerRotationPitch = false` / `bInheritPitch = false` — 즉 피치는 **"어디를 보나"가 아니라 "레일 위 어디에 서나"** 를 뜻하게 됐다. 실제 시선 각도는 `UpdateLookRotation`이 따로 만든다.

그런데 락온은 옛 뜻 그대로 **적을 본 각도**를 거기에 넣고 있었다.

```cpp
const float ClampedPitch = FMath::ClampAngle(LookRot.Pitch, -80.f, 45.f);
const FRotator DesiredRot(ClampedPitch, LookRot.Yaw, CurrentRot.Roll);
```

### 이게 뱅뱅 도는 고리가 된다

```
적을 본 각도 → 피치 → 레일에서 카메라 위치 이동
      ↑                              ↓
      └──── 카메라가 움직였으니 각도 재계산 ────┘
```

BP_SBPlayer 실측 스플라인(`-1.2/0/514` → `-382/40/117` → `-52/0/-83`)으로 수렴 계산.
**시작 피치가 -80이든 +44이든 전부 같은 한 점으로 빨려든다.**

| | 카메라 뒤 | 카메라 높이 |
|---|---|---|
| 평상시 (alpha 0.5) | 382 | **+117** |
| 락온 중 (alpha 0.75) | 229 | **-57** |

**카메라가 174cm 주저앉아 캡슐 중심 아래(무릎 높이)에 선다.** 수렴점이 하나뿐이라 `RInterpTo` 속도 5로 계속 도로 끌려온다 = "아예 고정".

## SB는 어떻게 했나 (덤프 실측)

`Eve_CameraPitch` = `t 0 → -89° / t 1 → +45°`. **우리 `PitchAtStart`/`PitchAtEnd`와 숫자까지 같다.**
SB에서도 피치는 레일 눈금이고, 그래서 **락온 피치를 계산하지 않고 커브에서 꺼내 쓴다.**

`/Game/System/Camera/LockOnPitchCurve` (게임 전체 공용, 캐릭터 BP 아님)

| Time (거리) | Value (각도) |
|---|---|
| -0.0005745888 | **-31.849575** |
| 1.0234942 | -25.940084 |
| 14.973207 | **-15.025459** |

키 시간이 어중간한 값 = **디자이너가 에디터에서 손으로 끌어 찍은 것.**
단위 표기는 덤프에 없다. SB 락온 사거리가 1700유닛(17m)이고 커브가 14.97에서 끝나므로 **미터로 보는 게 거의 확실하다(추론).**

### SB 락온 담당 나눔

| 무엇 | 어디에 붙어있나 | 값 |
|---|---|---|
| 피치 | **전역** (`System/Camera`) | 위 커브 |
| 요 자동회전 속도 | `SBSpringArmComponent` | 5°→0 / 90°→32 / 135°→32 / 160°→0 |
| 적 고르기 점수 | **캐릭터 BP** | `LockOnPointCurveByDir` × `...ByDistance` |
| 진입·해제 블렌드 | `SBCameraComponent` | 0→5 / 0.9→1 / 1→0.4 |
| 조준점 | **표적 액터** | `SBAimTargetComponent` (위치만 든 씬 컴포넌트) |

- **정면 5도 안쪽 = 자동회전 0** (안 건드림 = 미세 떨림 방지)
- **160도 이상 = 자동회전 0.** 해제가 아니다 — 해제는 거리 `1700 → 75` / `1701 → 0`
- 적 고르기 = 각도점수 × 거리점수 → **정면의 좀 먼 적이 옆의 가까운 적을 이긴다**

## 해결

### 1단계 — 락온이 상하를 안 건드린다

```cpp
const FRotator DesiredRot(CurrentRot.Pitch, LookRot.Yaw, CurrentRot.Roll);
```

`ClampedPitch` 삭제. 첫 칸이 "지금 값 → 지금 값"이라 `RInterpTo`가 상하에 대해 무동작이 된다.
**이것만으로 주저앉음이 사라지고 마우스 상하가 돌아온다.**

### 2단계 — 거리별 커브 (SB 이식)

`ULockOnConfig`에 칸 2개.

```cpp
class UCurveFloat;   // 전방 선언 필수

UPROPERTY(EditDefaultsOnly, Category = "LockOn|Target")
TObjectPtr<UCurveFloat> LockOnPitchCurve;   // 적까지 거리(cm)별 카메라 상하 각도

UPROPERTY(EditDefaultsOnly, Category = "LockOn|Camera", meta = (ClampMin = "0.1", ClampMax = "10.0"))
float PitchInterpSpeed = 1.5f;
```

`ULockOnComponent::TickComponent` — 좌우 보간 뒤 상하만 따로.

```cpp
FRotator InterpedRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, Config->CameraInterpSpeed);

if (Config->LockOnPitchCurve)
{
    const float Dist = FVector::Dist(OwnerPawn->GetActorLocation(), TargetPoint);
    const float CurrentPitch = FRotator::NormalizeAxis(CurrentRot.Pitch);
    InterpedRot.Pitch = FMath::FInterpTo(CurrentPitch,
        Config->LockOnPitchCurve->GetFloatValue(Dist), DeltaTime, Config->PitchInterpSpeed);
}
PC->SetControlRotation(InterpedRot);
```

**커브 미지정이면 `if`를 건너뛰고 1단계 동작 그대로.**

### ★ `FInterpTo`에 각도를 그냥 넣으면 안 된다

`GetControlRotation().Pitch`는 **0~360으로 나온다.** 아래로 25도면 `335`.

```cpp
// PlayerCameraManager.cpp:1065
ViewRotation.Pitch = FRotator::ClampAxis(ViewRotation.Pitch);   // 0~360으로 저장
```

`FInterpTo`는 그냥 뺄셈이라 `-25.94 - 335 = -360.94`로 잡혀 **카메라가 한 바퀴 돈다.**

```cpp
// UnrealMathUtility.h:1593
const RetType Dist = Target - Current;                        // 각도 처리 없음

// UnrealMath.cpp:2687 — RInterpTo는 다르다
const FRotator Delta = (Target - Current).GetNormalized();     // 한 바퀴 처리 있음
```

→ **`FRotator::NormalizeAxis`로 펴서 넣는다.**

### 왜 상하만 `FInterpTo`로 갈랐나

`RInterpTo`는 세 칸을 통째로 받고 **속도 인자가 하나뿐**이다. 좌우 5.0 / 상하 1.5로 갈라야 해서 상하를 빼냈다. 빼내는 순간 각도 그릇을 벗어나므로 `NormalizeAxis`가 따라붙는다.

**상하를 느리게 하는 게 의도다.** 1.5면 한 프레임에 2.5%만 당기니 그 사이 마우스 입력이 살아남고, 손을 떼면 그제야 커브 값으로 돌아온다.

## 에셋

```
/Game/SB_Style_GameProject/Camera/Curves/CF_LockOnPitchByDistance   (신규)
/Game/SB_Style_GameProject/UI/DA_LockOnConfig_Default               (Lock On Pitch Curve 지정)
```

커브 키 3개 — SB 값을 cm로 환산.

| Time (cm) | Value |
|---|---|
| 0 | -31.85 |
| 102 | -25.94 |
| 1497 | -15.03 |

현재 `LockOnRadius = 1000`이라 1497 뒤는 안 쓰인다. 1000 지점 값은 약 -19도.

## 검증 (PIE 통과)

| 항목 | 결과 |
|---|---|
| 락온 중 마우스 상하 조작 | ✅ 먹힌다. 놓으면 커브 값으로 복귀 |
| 좌우가 적을 따라가는가 | ✅ 금방 돌아옴 |
| 카메라가 주저앉는가 | ✅ 사라짐 |

계산상 기대치 — 붙으면 `뒤 371 / 높이 +204`, 멀면 `뒤 373 / 높이 +61`.
**거리가 370쯤에서 안 무너지는 게 핵심.** 고장났을 땐 229/-57까지 갔다.

---

# 이어서 — 좌우 자동회전 속도 커브 + 사거리 (같은 날)

## 왜

좌우를 **항상 같은 속도(5)로** 당기고 있었다. 이미 적을 보고 있어도 계속 미세하게 건드린다.
SB는 `SBSpringArmComponent`의 `YawAutoRotateSpeedByDirDiff`로 **각도별 속도**를 준다.

| 벗어난 각도 | SB 속도 |
|---|---|
| 5° | **0** |
| 90° | 32 |
| 135° | 32 |
| 160° | **0** |

**핵심은 숫자가 아니라 양 끝의 0이다.** 정면 = 미세 떨림 방지, 등 뒤 = 확 휘둘리지 않게.
(160°는 **해제가 아니다.** 해제는 거리 `1700 → 75` / `1701 → 0`)

## ★ 함정 — `RInterpTo`에 속도 0을 넘기면 스냅한다

```cpp
// UnrealMath.cpp:2680
if( InterpSpeed <= 0.f )
{
    return Target;          // "가만히"가 아니라 즉시 목표로 순간이동
}
```

정면에 오는 순간 카메라가 적한테 딱 붙어버린다. **의도와 정반대.**
→ **0 구간은 `RInterpTo` 호출 자체를 건너뛴다.**

## 코드

```cpp
// 어긋난 각도가 클수록 빨리 따라감 - 정면과 등 뒤는 0이라 안 건드림
float YawSpeed = Config->CameraInterpSpeed;
if (Config->YawSpeedByAngle)
{
    const float YawDiff = FMath::Abs(FRotator::NormalizeAxis(LookRot.Yaw - CurrentRot.Yaw));
    YawSpeed = Config->YawSpeedByAngle->GetFloatValue(YawDiff);
}

FRotator InterpedRot = CurrentRot;
if (YawSpeed > 0.f)   // 0을 넘기면 가만히가 아니라 즉시 스냅 - 생략
{
    const FRotator DesiredRot(CurrentRot.Pitch, LookRot.Yaw, CurrentRot.Roll);
    InterpedRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, YawSpeed);
}
```

`NormalizeAxis`로 차이를 내므로 표현이 0~360이든 -180~180이든 상관없고, `Abs`라 커브는 **한쪽만** 그리면 된다.

**외삽은 신경 안 써도 된다** — `FRealCurve` 기본이 `RCCE_Constant`(`RealCurve.h:122`)라 첫 키 앞·마지막 키 뒤가 평평하다. 설령 Linear로 만들어도 음수가 나올 뿐이고 `> 0.f` 가드에 걸려 똑같이 건너뛴다.

## SB 32를 그대로 쓴 근거

우리 상시값 5보다 6배라 처음엔 과해 보였는데, 실제 구간을 계산하면 겹친다.

| 벗어난 각도 | 커브가 주는 속도 |
|---|---|
| 20° | **5.6** ← 기존 상시 5와 사실상 같다 |
| 45° | 15 |
| 90° | 32 |

**평소 추적감은 그대로고 크게 벌어졌을 때만 확 잡아챈다.** SB 소비처 수식은 미상이라 단위가 다를 가능성은 남지만, 이 대조가 맞아떨어지므로 같은 종류로 판단.

## 에셋

```
/Game/SB_Style_GameProject/Camera/Curves/CF_LockOnYawSpeedByAngle   (신규)
```
`5→0` / `90→32` / `135→32` / `160→0`

`DA_LockOnConfig_Default` — 커브 지정 + **`Lock On Radius` 1000 → 1700**(SB값).
사거리를 늘려서 피치 커브의 `1497` 키가 그제야 살아난다. 이전엔 1000에서 잘려 안 쓰였다.

## 검증

- PIE 체감 이상 없음 (승환)
- **좌우 옆걸음 = 불편함 없음** → 조준 기준점(`GetCameraLocation()`)은 **그대로 둔다.** 랙이 회전으로 새는 양이 약 3.6도로 카메라 때(6.3도)의 절반이라 안 거슬리는 수준

## 남은 것

| | 상태 |
|---|---|
| ~~좌우 조준 기준점이 랙 먹은 실측 위치~~ | ✅ 재봤고 안 거슬림. **안 고치기로** |
| ~~좌우 자동회전 속도 커브~~ | ✅ 완료 |
| ~~사거리 1000 → 1700~~ | ✅ 완료 |
| 적 고르기 점수제 (각도×거리) | 폴리싱 단계로 (승환 결정) |
| 진입·해제 블렌드 커브 | 폴리싱 단계로 (승환 결정) |
| `GetLockOnPoint`의 `HalfHeight * 0.5` 갈래 | 적 작업 때. `LockOnSocketName` 채우면 안 탐 |
| 락온 중 이동속도 감속 | SB는 Run 500 → **280**(44% 감속). 개념 자체가 없음. 폴리싱 |

## 참조

- 트러블슈팅: `카메라-레일눈금에_락온이_시선각도를_넣음`
- 관련 dev-log: `2026-08-03-camera-rail-look-rotation` (피치가 레일 눈금이 된 작업)
- SB 원본: `notes/Reference/StellarBlade_Player_Analysis.md` §4-1~4-3
