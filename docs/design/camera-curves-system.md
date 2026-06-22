# Camera SB Curve System — Project_KD

> Stellar Blade 정합 동적 카메라 + 이동 회전 관성. 2026-05-28 도입.
> 데이터 드리븐 (CurveFloat 외부화) — 디자이너가 코드 X로 튜닝.

## 핵심 원칙 — Editor 튜닝 우선, 코드 최소

SB 데이터(JSON exports 21165개에서 추출)를 그대로 재현하지 않고, **CurveFloat 슬롯**으로 노출 → BP/자산에서 직접 편집. 코드는 "커브 평가 + 결과 적용" 한 줄씩만.

## 아키텍처

| 구성 요소 | 책임 | 위치 |
|---|---|---|
| `AKDPlayerCameraManager` | Pitch 클램프 / FOV 동적 / Slope 슬롯(미구현) | `Source/Project_KD/Player/KDPlayerCameraManager.h/.cpp` |
| `APlayerCharacter` | TurnSpeedCurve 평가 / Sprint 3단계 | `Source/Project_KD/Player/PlayerCharacter.h/.cpp` |
| `CF_FovByCamDist` | 카메라-캐릭터 거리 → FOV | `Content/Curves/Camera/CF_FovByCamDist` |
| `CF_TurnSpeedByVelocity` | 캐릭터 속도 → 회전속도 | `Content/Curves/Movement/CF_TurnSpeedByVelocity` |

## 데이터 흐름

### FOV 동적 (벽 가까이 → 시네마틱 줌)

```
[ViewTarget Tick]
    ↓
AKDPlayerCameraManager::UpdateViewTarget
    ↓
거리 측정 (Pawn ↔ Camera POV)
    ↓
CF_FovByCamDist->GetFloatValue(거리)  → DesiredFOV
    ↓
OutVT.POV.FOV = DesiredFOV
```

### 회전 관성 (속도 = 둔함)

```
[Pawn Tick]
    ↓
APlayerCharacter::Tick
    ↓
GetVelocity().Size() = 현재 속도
    ↓
TurnSpeedCurve->GetFloatValue(속도) → TurnRate
    ↓
CharacterMovement->RotationRate = FRotator(0, TurnRate, 0)
```

### Sprint 3단계

```
Shift Press → StartSprint
    ↓
MaxWalkSpeed = SprintSpeed(900) + FullSprintTimer 4초 시작
    ↓
4초 경과 → EnterFullSprint
    ↓
MaxWalkSpeed = FullSprintSpeed(1100) "빵"
    ↓
Shift Release → StopSprint → MaxWalkSpeed = WalkSpeed(550), 타이머 정리
```

## 핵심 결정사항

### 1. `UpdateViewTarget`이 진입점 (UpdateCamera 아님)

UE5 PCM `DoUpdateCamera` 흐름에서 **NewPOV 스냅샷이 UpdateCamera 호출 전에 만들어짐** → `UpdateCamera`에서 POV.FOV 수정해도 적용 안 됨. POV 진짜 결정 진입점은 `UpdateViewTarget`. 자세한 함정 정리 = `memory/reference_ue5_pcm_camera_flow.md`.

### 2. Pitch 비대칭 클램프 -80° / +45°

SB Eve_CameraPitch 정합. 아래 시야는 관대(-80), 위 시야는 강제 제한(+45)으로 정수리 회피.

### 3. CurveFloat 외부화 (코드 상수 X)

- 디자이너가 BP/자산에서 직접 조정 (Editor 튜닝 우선)
- 새 변수 추가 시 코드 변경 무 (UPROPERTY 슬롯만 박음)
- 메모리 박힌 룰 `reference_ue5_curvefloat.md` 정합

### 4. Camera Lag OFF

`SpringArm->bEnableCameraLag = false`. 락온/타격감과 충돌 가능성 회피.

## 적용 셋팅 — Baseline

### KDPlayerCameraManager 생성자
```cpp
ViewPitchMin = -80.0f;
ViewPitchMax = 45.0f;
DefaultFOV   = 75.0f;
```

### BP_PlayerCharacter
| 항목 | 값 |
|---|---|
| FollowCamera FOV | 75 |
| CameraBoom TargetArmLength | 500 |
| CameraBoom SocketOffset | (0, 40, 70) |
| CameraBoom CameraLag / RotationLag | OFF |
| WalkSpeed | 550 |
| SprintSpeed | 900 |
| FullSprintSpeed | 1100 |
| FullSprintTriggerSec | 4.0 |

### CF_FovByCamDist
| Time (cm) | Value (FOV) |
|---|---|
| 100 | 40 |
| 200 | 50 |
| 350 | 65 |
| 500 | 75 |
| 600 | 75 |

### CF_TurnSpeedByVelocity
| Time (cm/s) | Value (deg/s) |
|---|---|
| 0 | 540 |
| 550 | 540 |
| 700 | 350 |
| 900 | 200 |
| 1100 | 100 |

→ 정지~Walk 빠른 회전 유지, Sprint 진입부터 관성 시작.

## 검증

- [x] 정지에서 360° 회전 → 빠르고 정확
- [x] 카메라 벽에 가까워짐 → FOV 자동 좁아짐 (시네마틱)
- [x] Sprint 진입 시 캐릭터 회전 둔해짐 체감
- [x] 4초 hold → "빵" 효과
- [ ] Sprint 떼면 부드러운 감속 (검증 대기)
- [ ] 종합 PIE — 다양한 거리·속도에서 자연스러움

## 미구현 / 후속

- **SlopeControlCurve** — 슬롯만 박힘, 평가 로직 미구현. M2 폴리싱 dev item
- **LockOnPitchCurve** — 락온 시 거리별 Pitch 자동 조정. Phase 2 (현재 락온은 Pitch 자유)
- **드론 카메라** — SB dronLocationData 패턴. M3+ 대규모 작업

## 관련 자료

- `memory/reference_sb_camera_data_2026-05-27.md` — SB 카메라 8개 커브 분석 (FOV/Pitch/FovByCamDist/TurnSpeedCurve/드론카메라)
- `memory/reference_ue5_curvefloat.md` — CurveFloat/Vector/Color/Table 사용 시점
- `memory/reference_ue5_pcm_camera_flow.md` — UpdateCamera vs UpdateViewTarget 함정
- `memory/reference_sb_feel_observations.md` — SB 조작감 관찰 매핑 (C-1/C-4 완료)
- `.omc/dev-logs/2026-05-28-camera-sb-curve-system.md` — 개발 진행 dev-log
