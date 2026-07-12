# SB식 데이터 드리븐 카메라 + Sprint 3단계 + 관성 회전 (2026-05-28)

**상태**: 🔄 진행 중 — Step 1~5 완료, Step 6/7/8 + 다른 dev item 남음.

## 작업 요약

### 완료 항목
1. ✅ **AKDPlayerCameraManager** 신규 C++ 클래스 (Custom PlayerCameraManager)
2. ✅ **Pitch 비대칭 클램프** -80°/+45° (SB Eve_CameraPitch 정합)
3. ✅ **FovByCamDist 동적 FOV** — 거리별 FOV 자동 조정 (DMC식)
4. ✅ **TurnSpeedCurve** — 속도별 회전 관성 표현
5. ✅ **Sprint 3단계** — Walk(550) → Run(900) → Full Sprint(1100), 4초 hold 시 즉시 빵
6. ✅ **Static BP defaults** — FOV 75, ArmLength 500, SocketOffset (0,40,70), Camera Lag OFF

### 핵심 기술 발견 — UE5 PCM 카메라 흐름 함정
`UpdateCamera`에서 `ViewTarget.POV.FOV` 수정 = 적용 안 됨. NewPOV 스냅샷이 UpdateCamera 전에 만들어져서 우리 변경 무시.

해결: **`UpdateViewTarget(OutVT, DeltaTime)` override** — POV 계산의 진짜 진입점.

자세한 함정 정리: `memory/reference_ue5_pcm_camera_flow.md`

---

## 적용 셋팅 — Baseline + Final

### KDPlayerCameraManager 생성자
```cpp
ViewPitchMin = -80.0f;   // SB식 비대칭 — 아래 시야 관대
ViewPitchMax = 45.0f;    // 위 시야 강제한 (정수리 회피)
DefaultFOV = 75.0f;      // SB Eve_CameraFov 정합
```

### BP_PlayerCharacter
| 항목 | Before | After |
|---|---|---|
| FollowCamera FOV | 90 | **75** |
| CameraBoom TargetArmLength | 350 | **500** |
| CameraBoom SocketOffset | (0,0,70) | **(0,40,70)** |
| CameraBoom Camera Lag | ON | **OFF** |
| CameraBoom bEnableCameraRotationLag | ON | **OFF** |
| CharacterMovement RotationRate | 540 (고정) | **TurnSpeedCurve 동적** |
| WalkSpeed | 550 | 550 (유지) |
| SprintSpeed | 900 | 900 (유지) |
| **FullSprintSpeed** (신규) | - | **1100** |
| **FullSprintTriggerSec** (신규) | - | **4.0** |

### CurveFloat 자산

**CF_FovByCamDist** (Content/Curves/Camera/):
| Time (cm) | Value (FOV) |
|---|---|
| 100 | 40 |
| 200 | 50 |
| 350 | 65 |
| 500 | 75 |
| 600 | 75 |

**CF_TurnSpeedByVelocity** (Content/Curves/Movement/):
| Time (cm/s) | Value (deg/s) |
|---|---|
| 0 | 540 |
| 550 | 540 |
| 700 | 350 |
| 900 | 200 |
| 1100 | 100 |

→ 정지~걷기 빠른 회전 유지, Sprint 진입부터 관성 시작 (작성자 의도).

---

## 코드 변경 위치

### 신규 파일
- `Source/Project_KD/Player/KDPlayerCameraManager.h`
- `Source/Project_KD/Player/KDPlayerCameraManager.cpp`

### 수정 파일
- `Source/Project_KD/Player/PlayerCharacter.h` — TurnSpeedCurve UPROPERTY + FullSprintSpeed/Sec + bFullSprintActive + FullSprintTimerHandle + EnterFullSprint 선언
- `Source/Project_KD/Player/PlayerCharacter.cpp` — Tick에 TurnSpeedCurve 평가, StartSprint/StopSprint Full Sprint 처리, EnterFullSprint 함수 추가

---

## 트러블슈팅 기록

### 함정 1 — Rider "Surface Heuristics" 빌드 실패
**증상**: "Succeeded: False, Errors: 0" 1초만에 끝남.
**원인**: Rider가 C# 프로젝트로 빌드 시도. UE5는 UBT(UnrealBuildTool) 사용해야 함.
**해결**: Configuration **Project_KDEditor Win64 Development Editor** 선택.

### 함정 2 — Microsoft.Cpp.Default.props 없음
**증상**: "D:\Microsoft.Cpp.Default.props 못 찾음" 빌드 에러.
**원인**: Visual Studio C++ Workload 미설치 또는 경로 인식 실패.
**해결**: VS Installer → "Game development with C++" + "Desktop development with C++" 워크로드 추가.

### 함정 3 — UpdateCamera에서 POV.FOV 수정 무시됨
**증상**: 디버그 로그는 DesiredFOV=40인데 `showdebug camera`는 FOV=75 유지.
**원인**: DoUpdateCamera 흐름에서 NewPOV 스냅샷이 UpdateCamera 호출 전에 만들어짐 → 우리 변경 무시.
**해결**: `UpdateViewTarget(OutVT, DeltaTime)` override로 이전.

### 함정 4 — Constructor에 return 타입 없다는 Rider 경고
**증상**: Rider 빨간 줄 — "AKDCameraManager()는 반환 타입을 가져야 합니다".
**원인**: Rider 정적 분석 false positive. 실제 컴파일러는 정상 처리.
**해결**: 무시. 또는 File → Invalidate Caches로 인덱싱 재실행.

---

## PIE 검증 결과 (작성자 확인)

| 시나리오 | 결과 |
|---|---|
| Pitch 클램프 (-80/+45) | ✅ 작동 |
| FOV 75 평상시 | ✅ 적용 |
| 카메라 벽 가까이 → FOV 좁아짐 | ✅ 동적 FOV 작동 |
| 정지~걷기 빠른 회전 | ✅ 자연스러움 |
| Sprint 진입 시 관성 시작 | ✅ 둔해짐 체감 |
| Sprint 4초 빵 효과 | (검증 필요) |
| Sprint 떼면 부드러운 감속 | (검증 필요) |

작성자 코멘트: **"잘 동작한다잉"** ✅

---

## 남은 작업 — 우선순위 순

### Step 6/7/8 (카메라 작업 마무리, 1시간 내)
- **Step 6**: SlopeControl 슬롯 (선택) — M2 dev item으로 보류 가능
- **Step 7**: 종합 PIE 검증 (4초 빵 효과, Sprint 감속, 통합 점검)
- **Step 8**: 메모리 갱신 (SB feel observations C-1/C-4 완료 마킹)

### M1 폴리싱 남은 dev item
1. **M-4 닷지 방향 카메라 forward 기준** — C++ 1줄 변경 (GA_Dodge::ResolveDodgeDirection)
2. **M-3 닷지에 MovementCancel 노티 추가** — 자산만, 4개 Montage
3. **A-1 공격 몽타주 단축** — Light/Heavy Rate Scale + Blend
4. **A-7 Light 4타 모션 검토** — 작성자 자산 작업

### M2 dev item (별도 작업)
1. **콤보 분기 다양화** — Quick/Onslaught 식 빠른/느린 분기 자산
2. **타격감 3층** (HitStop/BoneShake/VertexShake) — SB 13번 설계안 B-3
3. **Stellar Blending** — mid-combo 분기 점프
4. **LockOn 시스템** — SB LockOnPitchCurve 활용
5. **SlopeControlCurve 실제 적용**
6. **ABP 180도 회전 모션** — M-1 SB 관찰
7. **PlayerController 결합도 리팩토링** — Cast<APlayerCharacter> 제거

### M3+ 대규모 작업
1. **드론 카메라 시스템** — SB dronLocationData 패턴
2. **컨텍스트별 카메라 본** — SB CameraBone 패턴 (액션별 시네마틱)
3. **시네마틱 카메라 트랙** — SBCameraDollyTrackData 패턴
4. **절단 시스템** — ProceduralMesh + 버텍스 스키닝
5. **PlayRate 동적 조정 재검토** (보류 항목)

---

## 회귀 방지 노트

### 새 카메라 효과 추가 시
- POV 수정 = **UpdateViewTarget**에서 (UpdateCamera는 후처리/디버그만)
- 회전 클램프 = `ViewPitchMin/Max` UPROPERTY로
- CameraShake = `StartCameraShake()` 시스템

### CurveFloat 패턴 확장
- 새 동적 카메라/이동 매개변수 → CurveFloat로 (페어 룰 §3 "Editor 튜닝 우선")
- 슬롯은 KDPlayerCameraManager 또는 PlayerCharacter에 UPROPERTY
- Tick 또는 UpdateViewTarget에서 평가
- 디자이너가 BP/자산에서 직접 조정

### Sprint 3단계 흐름
- StartSprint → SprintTimer + FullSprintTimer
- 4초 후 FullSprintTimer 만료 → EnterFullSprint → MaxWalkSpeed 즉시 빵
- StopSprint → 둘 다 정리 + 부드러운 감속

---

## 페어 정합 검증

- **§1-1 안티 패턴**: 싱글톤 X, Manager는 UE5 표준 PCM 상속 ✅
- **§1-2 데이터 드리븐**: Camera/Movement 파라미터 CurveFloat로 외부화 ✅
- **§3 YAGNI + Editor 튜닝 우선**: 정적 값 BP defaults, 동적은 Curve, 코드는 최소 ✅
- **§5-1 도메인**: Player/카메라 = 작성자 단독 결정 OK ✅

---

## 메모리 박힌 reference
- `reference_sb_camera_data_2026-05-27.md` — SB 카메라 JSON 데이터
- `reference_sb_feel_observations.md` — SB 조작감 관찰 (C-1/C-4 적용 완료)
- `reference_ue5_curvefloat.md` — CurveFloat 패턴 가이드
- `reference_ue5_pcm_camera_flow.md` — PCM UpdateCamera 함정 트러블슈팅

---

## 다음 세션 진입점

작성자 결정 필요:
- **A**: Step 7/8 마무리 → 카메라 작업 완전 종료 → 새 dev item
- **B**: M-4 닷지 카메라 forward (C++ 1줄, 빠름) 먼저 처리
- **C**: 콤보 분기 다양화 (자산 多, M1~M2 폴리싱)
- **D**: 타격감 3층 도입 (SB 13번 설계안)

현재 카메라 작업은 80% 완성, 코어 동작 작성자 확인됨.
