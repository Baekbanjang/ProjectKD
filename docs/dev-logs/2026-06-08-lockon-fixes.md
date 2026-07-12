# 락온 시스템 수정 3건 (2026-06-08)

락온(MMB 토글) 폴리싱 3건. 통합 인사이트 = `IKDTargetableInterface::GetLockOnPoint()` 1개로
#1 카메라 pitch 추적 / #2 공격 자동 조준 / #3 레티클 앵커를 한 신호로 공급(적이 자기 크기로 자급, 디커플링 유지).

---

## STEP 1 — 인터페이스 + 적 구현 ✅ (구현 완료, 빌드는 STEP 2와 묶음)

`GetLockOnPoint()` = 적 몸통 월드 좌표 공급. 세 요청 모두 이 좌표 1개에서 파생
(#1 상하 각도 / #2 좌우 방향 / #3 높이).

**변경 파일**
- `Interface/KDTargetableInterface.h` — `GetLockOnPoint()` BlueprintNativeEvent 추가.
  기본 `_Implementation` = `FVector::ZeroVector`(인터페이스 본체는 액터 아님 → 실좌표는 적이 책임).
- `Enemy/KDEnemyBaseCharacter.h/.cpp` — `GetLockOnPoint_Implementation` override.
  몸통 소켓(`spine_03`) 우선 → 없으면 캡슐 절반 높이로 근사. 적별 크기 자동 흡수.
  (`.cpp`에 CapsuleComponent.h/SkeletalMeshComponent.h 이미 인클루드 — 추가 불필요)

**메모**: 소켓명 `spine_03`은 실제 스켈레톤 확인 필요. 없으면 캡슐 폴백 작동(빌드는 안 깨짐).
`GetLockOnPoint()`는 STEP 2의 Tick/레티클에서 처음 호출되므로 빌드 검증은 STEP 2와 통합.

---

## STEP 2·3·4 — 레티클 + pitch + 공격 조준 ✅ (빌드 통과, PIE 3건 정상)

- `PlayerCharacter.h` — `GetLockOnComponent()` 인라인 getter.
- `LockOnComponent.cpp` Tick — #1 카메라 pitch+yaw 추적(기준점 폰 발→카메라 위치, pitch clamp -80~45).
  기존엔 pitch=`CurrentRot.Pitch`(안 건드림)라 상하 고정 안 됐던 게 근본 원인.
- `LockOnComponent.cpp EngageLockOn` — #3 레티클 Z = `GetLockOnPoint - ActorLocation`의 Z(적별 몸통 높이).
- `GA_WeaponTraceBase.cpp OnActivated` — #2 락온 중 공격 시 적 방향 yaw 스냅(±135 게이트, 뒤면 skip).
  **함정**: `OnActivated`은 GA_WeaponTraceBase 최초 선언 hook → `.h`에서 `override` 금지(부모에 없음).
  subclass(GA_EnemyBasicAttack)만 override 사용.

**PIE 결과**: 상하고정·레티클 적응·공격 조준 3건 모두 정상.

### 후속 폴리싱(2026-06-08 추가 요청)
1. 높이 차 큰 적(건물 위 등) 락온 시 카메라 붐 충돌→FovByDistanceCurve가 FOV 좁힘→시야 답답.
   → 락온 중 동적 FOV 무시 검토(KDPlayerCameraManager).
2. `GetLockOnPoint` 소켓명 하드코딩(`spine_03`) → BP에서 적별 지정 가능하게 UPROPERTY 노출.
