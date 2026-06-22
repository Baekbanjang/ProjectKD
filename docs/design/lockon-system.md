# LockOn System — Project_KD

> Souls/Stellar Blade식 자동 락온 (Phase 1). 2026-05-28 도입.
> Deep Interview 9R 결정 → 9 step 구현. Phase 1 = 자동 전환 only, 수동/Pitch curve는 Phase 2.

## 핵심 원칙 — 디커플링/확장성/독립성

| 원칙 | 적용 |
|---|---|
| **디커플링** | `Cast<APlayerCharacter>` 금지 → `Implements<UKDTargetableInterface>` 게이트. 적이 자가 자격 판정 |
| **확장성** | 새 적/오브젝트 추가 시 인터페이스만 박으면 락온 가능. LockOn 코드 무변경 |
| **독립성** | 카메라 회전 영향 = LockOnComponent 본인이 ControlRotation 조정. CameraManager 무지 |

## 아키텍처

| 구성 요소 | 책임 | 위치 |
|---|---|---|
| `ULockOnComponent` | 토글 / 후보 검색 / 활성·해제 / Tick 검증·추적 | `Source/Project_KD/Combat/LockOnComponent.h/.cpp` |
| `ULockOnConfig` | 파라미터 묶음 (Radius/Cone/LoS/InterpSpeed/Reticle) | `Source/Project_KD/Combat/Data/LockOnConfig.h` |
| `IKDTargetableInterface` | 적 자격 판정 + 락온 알림 (CanBeTargeted / OnTargeted) | `Source/Project_KD/Interface/KDTargetableInterface.h` |
| `State.Character.LockOn` 태그 | GAS 상태 표시 (다른 시스템이 락온 인지) | `KDGameplayTags.h` |
| `WBP_LockOnReticle` | 락온 마커 위젯 (Screen space) | `Content/UI/WBP_LockOnReticle` |
| `DA_LockOnConfig_Default` | Config 인스턴스 | `Content/Combat/Data/` |

## 데이터 흐름

### 입력 → 락온 활성화

```
MMB Press → IA_LockOnToggle(Started)
    ↓
AKDPlayerController::Handle_LockOnToggle
    ↓
Cast<APlayerCharacter>(GetPawn())
    ↓
APlayerCharacter::ToggleLockOn
    ↓
LockOnComponent->ToggleLockOn
    ↓
[bIsLockedOn?] → No → FindBestTarget → EngageLockOn(Target)
              → Yes → DisengageLockOn
    ↓
APlayerCharacter가 bUseControllerDesiredRotation/bOrientRotationToMovement 토글
```

### FindBestTarget — 5 게이트 필터

```
[OverlapMultiByObjectType (ECC_Pawn, Radius=1000cm)]
    ↓ N개 raw 후보
[게이트 1] null / Owner / Seen 중복 제거
    ↓
[게이트 2] Implements<UKDTargetableInterface> + CanBeTargeted()
    ↓
[게이트 3] 시야콘 — Dot(CamForward, ToCand) >= Cos(±45°)
    ↓
[게이트 4] LoS — LineTraceByChannel(Visibility) 벽 없음
    ↓
[게이트 5] 최단거리 (DistSquared 비교)
    ↓
BestTarget 1개 또는 nullptr
```

### EngageLockOn

```
LockedTarget = Target
bIsLockedOn = true
    ↓
ASC->AddLooseGameplayTag(State.Character.LockOn)
    ↓
IKDTargetableInterface::Execute_OnTargeted(Target, true)  ← 적이 자기 ABP에 알림
    ↓
WidgetComponent를 Target의 RootComponent에 부착 + SetVisibility(true)
    ↓
RelativeLocation(0, 0, 80) — 머리 위
```

### Tick — 검증 + 카메라 추적

```
[ULockOnComponent::TickComponent]
    ↓
bIsLockedOn? No → return
    ↓
IsTargetStillValid?  No → DisengageLockOn → FindBestTarget → 있으면 EngageLockOn
                     Yes ↓
[Yaw 보간]
    ToTarget = Target.Loc - Owner.Loc
    DesiredRot.Yaw = ToTarget.Rotation().Yaw  (Pitch/Roll = Current 유지)
    InterpedRot = FMath::RInterpTo(CurrentRot, DesiredRot, DeltaTime, Config->CameraInterpSpeed)
    ↓
PC->SetControlRotation(InterpedRot)
    ↓
[자동 처리]
- SpringArm이 ControlRotation 따라 회전 → 카메라 적 추적
- bUseControllerDesiredRotation=true이므로 캐릭터도 ControlRotation 따라 회전 → 몸 적 봄
```

### DisengageLockOn

```
IKDTargetableInterface::Execute_OnTargeted(PrevTarget, false)
    ↓
ASC->RemoveLooseGameplayTag(State.Character.LockOn)
    ↓
WidgetComponent → Owner로 attach 복귀 + SetVisibility(false)
    ↓
bIsLockedOn = false / LockedTarget = nullptr
```

## 핵심 결정사항

### 1. 카메라 추적 = LockOnComponent 본인 책임 ★

처음엔 `AKDPlayerCameraManager::UpdateViewTarget` 확장 안으로 갔다가 **디커플링 룰 위반** 지적받고 피벗. CameraManager가 LockOnComponent 클래스를 알아야 하는 결합. 현재 안은:

- LockOnComponent.cpp TickComponent에서 `PC->SetControlRotation` 직접 호출
- SpringArm/캐릭터가 ControlRotation 자동 따라옴
- CameraManager는 락온 시스템 존재 자체 모름

**부수 효과**: BP 마이그레이션 가능성 확보 (CameraManager UpdateViewTarget는 C++ virtual 못 override).

### 2. Pitch 자유 (Phase 1)

Yaw만 타겟 방향 보간, Pitch는 사용자 마우스 그대로. **Phase 2에 LockOnPitchCurve** (거리별 자동 Pitch) 도입 예정 — SB LockOnPitchCurve 정합.

### 3. 캐릭터도 타겟 바라봄 (Souls 표준)

`bUseControllerDesiredRotation=true` + `bOrientRotationToMovement=false` 동시 토글. 락온 ON 시 strafe(게걸음) 이동. Deep Interview 결정 #4.

### 4. 자동 전환만 (수동 X)

타겟 사망 / 거리 초과 / LoS 잃음 → IsTargetStillValid false → 자동 해제 + 인근 다른 적 자동 락온. 수동 휠 스크롤 전환은 Phase 2.

### 5. 인터페이스 패턴 vs Cast

`Cast<AKDEnemyBaseCharacter>` 안 함. `Implements<UKDTargetableInterface>` + `Execute_CanBeTargeted`. 적 종류 무한 확장 가능. 메모리 `feedback_decoupling_design.md` 정합.

## 적용 셋팅

### ULockOnConfig 슬롯 (DataAsset 편집)
| 항목 | 값 | 비고 |
|---|---|---|
| LockOnRadius | 1000 cm | 자동 해제 거리와 동일 |
| ViewConeAngle | 90° | ±45° |
| bUseLineOfSightCheck | true | 벽 뒤 적 제외 |
| CameraInterpSpeed | 5 | SB식 중간 — 4~6 권장 |
| ReticleWidgetClass | `WBP_LockOnReticle` | 머리 위 마커 |

### 적 BP 셋업 (각 적마다)
1. Class Settings → Implemented Interfaces → Add → `KDTargetableInterface`
2. Compile
3. `Can Be Targeted` 함수 자동 생성 → **Return Value = True 체크** (디폴트 false 함정)
4. `On Targeted` Event 노드 추가 (Phase 1 본문 비워둠, Phase 2 LockOn_BS 연동 예정)

### 입력 셋업
- `IA_LockOnToggle` (Digital, Started) — `Mouse Wheel Button` (MMB) 매핑
- `IMC_Default`에 추가
- `BP_KDPlayerController.IA_LockOnToggle` 슬롯 채움

## 검증 — PIE 5 시나리오

- [x] **시나리오 1**: MMB → 1000cm 안 가장 가까운 적 락온 + 마커 표시
- [ ] **시나리오 2**: 카메라/캐릭터가 타겟 향해 부드럽게 회전 (InterpSpeed 5)
- [ ] **시나리오 3**: 적 사망 → 자동으로 근처 다른 적 전환 (없으면 해제)
- [ ] **시나리오 4**: 거리 1000cm 초과 → 즉시 해제
- [ ] **시나리오 5**: 시야 잃음 (장애물 뒤) → 즉시 해제

추가 검증:
- 락온 중 마우스 Pitch 자유 (Yaw만 자동)
- 락온 중 strafe (게걸음) 자연스러움
- 재토글 → 해제 + 회전 모드 복원 (bOrientRotationToMovement=true)

## 트러블슈팅 메모

### 함정 1 — BP `Can Be Targeted` 디폴트 False

`BlueprintNativeEvent` 인터페이스를 BP가 오버라이드하면 BP 값이 이김. BP 자동 생성된 함수의 Return Value는 **체크박스 unchecked (false)**. C++ `_Implementation`이 `return true`라도 BP가 false면 false.

→ **인터페이스 박은 직후 Can Be Targeted Return Value = True 체크 필수.**

### 함정 2 — Overlap 25개 후보, BestTarget=None

OverlapMultiByObjectType는 콜리전 컴포넌트 단위로 결과 반환. 한 액터에 Capsule + Mesh + Weapon Socket 등 다중 Pawn-channel 콜리전이 있으면 같은 액터가 여러 번 반환. `Seen` Set으로 dedup. 진단 시 `Seen.Num()` 확인.

### 함정 3 — `class ULockOnComponent;` 전방선언

PlayerCharacter.h에서 `#include "Combat/LockOnComponent.h"` 대신 전방선언 + .cpp에서 include. 헤더 의존성 최소화.

## 미구현 / 후속 (Phase 2)

| 항목 | Phase | 비고 |
|---|---|---|
| 수동 타겟 전환 (휠 스크롤) | 2 | Souls 표준 |
| LockOnPitchCurve 거리별 Pitch | 2 | SB Eve_LockOnPitch 정합 |
| DataTable 진화 (필터 여러 프리셋) | 2 | 잡몹/엘리트/보스 다른 Config |
| 작은 적 자동 제외 | 2 | SB C-3 — 너무 작은 적 후보 제외 |
| 컨텍스트 분기 (전투 자세에만) | 2 | 평소엔 카메라 자유, 전투 시만 락온 가능 |
| 패드 입력 지원 | 2 | RB/L3 등 |
| ABP LockOn_BS 연동 | 2 | OnTargeted Event → 적 ABP 락온 자세 |

## 관련 자료

- `memory/project_lockon_system_2026-05-28.md` — Deep Interview 9R 결정 + 9 step 진행
- `memory/feedback_design_principles_first.md` — 디커플링/확장성/독립성 자가 점검 (Step 5 피벗 사건 학습)
- `memory/reference_sb_camera_data_2026-05-27.md` — SB LockOnPitchCurve 등 카메라 데이터
- `docs/design/camera-curves-system.md` — 카메라 시스템 (Phase 2 LockOnPitchCurve 통합 예정)
