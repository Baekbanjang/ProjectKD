# 플레이어 기본 이동 애니메이션 (로코모션 ABP) — SB 정합 (2026-05-30)

**상태**: ✅ 코어 완성 — PIE 검증 통과. 폴리싱/Slot 분할은 후속.

## 작업 요약

### 완료 항목
1. ✅ **UKDPlayerAnimInstance** C++ — ABP 데이터 공급 (이전 세션 빌드, 이번 세션 ABP 연동)
2. ✅ **노출 변수 9개** — GroundSpeed / VelocityX / VelocityY / bIsLockedOn / bIsInBattleStance / bIsInAir / bIsSprinting / bIsFullSprinting + BattleStanceHoldSec(EditAnywhere)
3. ✅ **PlayerCharacter sprint getter 2개** — IsSprinting() / IsFullSprinting() 인라인 (.h 유일 변경)
4. ✅ **BlendSpace 3종** — Peaceful/Battle(1D Speed) + LockOn(2D VelocityX/Y)
5. ✅ **ABP_Player 배선** — Locomotion SM(지상 3상태) + Main States SM(점프/낙하/착지) 재사용
6. ✅ **LockOn/Walk 10개 killdong 리타겟** — 저속 strafe (이번 세션 작업자 추가)

### 스레드 분리 (결정 #7)
- `NativeUpdateAnimation` (게임스레드): Velocity/ActorRotation 스냅샷, IsFalling, sprint getter, ASC 태그(LockOn/Attacking) → LastCombatTime 갱신
- `NativeThreadSafeUpdateAnimation` (워커): GroundSpeed = Size2D(), UnrotateVector로 로컬 분해 (VelocityY=X 전후, VelocityX=Y 좌우)

### Battle 전환 (디커플링)
ASC `State.Combat.Attacking` 태그 게임스레드 감시 → LastCombatTime 갱신 → `bIsInBattleStance = (now - LastCombatTime) < 3.0s`. PlayerCharacter.h 무변경.

## 아키텍처 결정

### 1. LeanAmount 드롭 — Peaceful/Battle은 1D Speed
플랜 초안은 Free 2D(Speed+Lean)였으나 최종 구현은 LeanAmount 미구현 → 1D Speed 단순화. Lean은 후속.

### 2. Airborne = BlendSpace 아님
공중은 vertical 축 노출 없음 → 단일 시퀀스. 템플릿 Main States SM(Jump/Fall/Land)이 이미 처리하므로 재사용. Locomotion SM은 지상 3상태만.

### 3. Main States SM 재사용 (삭제 X)
ABP 템플릿의 Main States = 점프/낙하/착지 래퍼. Locomotion SM 캐시 포즈를 Locomotion 상태가 소비하는 2단 ALS 구조. 삭제하면 공중 깨짐.

## 핵심 함정 (작업 중 발견)

### 함정 1 — C++ 상속 변수 ABP 패널 미표시
부모 클래스 UPROPERTY는 기본 숨김. "내 블루프린트" 톱니 → "상속된 변수 표시" 체크.

### 함정 2 — 템플릿 BP 변수 vs C++ 변수 혼동
옛 템플릿 `GroundSpeed_0`/`ShouldMove`/`IsFalling`(BP 자체 변수)은 잔재. 반드시 C++ 변수(GroundSpeed/VelocityX·Y/Is in Air) 사용.

### 함정 3 — 속도축 1100 (900 아님)
BS Speed 축 최대 = FullSprintSpeed 1100. 900은 Sprint 중간값. 코드(PlayerCharacter.h) 확인 후 정정.

## 적용 셋팅

### 속도 (PlayerCharacter.h)
| 항목 | 값 |
|---|---|
| WalkSpeed | 550 |
| SprintSpeed | 900 |
| FullSprintSpeed | 1100 (BS Speed 축 최대) |

### BlendSpace 샘플 배치
- Peaceful 1D: 0=Idle, 550=Run, 900/1100=Sprint(RateScale)
- Battle 1D: 0=Idle_Combat, 550=Run_Combat, 900/1100=Sprint
- LockOn 2D(±550): center=Idle, 8방향 Run_Combat strafe

### ABP 전환
- Locomotion SM: Peaceful↔Battle↔LockOn, 우선순위 LockOn>Battle, Duration 0.15~0.2s
- Main States SM: Is in Air==true → 공중 진입

## PIE 검증 결과
- [x] 정지/이동/Sprint → GroundSpeed + sprint 플래그
- [x] 공격 → Battle 3초 → Peaceful 복귀
- [x] 락온 → LockOn BS, 8방향 게걸음 양호
- [x] 점프 → 공중 포즈

## 후속 (TODO)
- 기본 이동(Peaceful) 자연스러움 폴리싱 — BS 샘플 보간 / 중간 Walk 샘플
- Slot 미리 분할 — 상하체 Layered Blend Per Bone / 공격·도술·피격 슬롯
- LockOn BS 2단(저속/고속) — Walk strafe 리타겟 완료분 활용
- LeanAmount (Free 2D Lean)
- 피격 기반 Battle 갱신 (hit/stagger 태그)
- Move 루트 Combat 중복본 정리 / BS_Player_Ariborne 오타

## 산출물
- `Source/Project_KD/Player/KDPlayerAnimInstance.h/.cpp`
- `Source/Project_KD/Player/PlayerCharacter.h:96-97` (getter)
- `Content/.../Animation/Sequence/BlendSpace/BS_Player_Loco_*`
- `Content/.../Animation/ABP_Player`
- `docs/design/player-locomotion-system.md`
- `memory/reference_player_locomotion_assets.md`
