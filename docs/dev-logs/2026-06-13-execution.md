# 페이탈 어택 / 처형 (Execution) — 2026-06-12~13

## 요청
적을 처형하는 페이탈 어택. 연출=페어 듀엣 애니(플레이어 피니셔+적 피격 동기 재생). 트리거=적 포이즈 깨질 때(Staggered)에 강공 명중. 카메라=전용 시네마틱(별도 진행).

## 핵심 발견 — 처형 시스템 70% 이미 존재 (적/피격자 측)
`StaggerComponent` + `ExecutionComponent` + `ExecutionProfile` 이미 구현됨. 빠진 것 = 플레이어 측 연출(피니셔 몽타주 + 시네 카메라).

## 신호 흐름 (핸드셰이크)
```
F키 → TryExecute() → 가짜 Event.Combat.Hit (InstigatorTags=Ability.Mugong.Heavy/Execution)
  → 적 ExecutionComponent::OnHitReceived → HandleExecution
       · ExecutionInstigator 캐시 (★신규 1줄)
       · OnExecutionBegin.Broadcast() (기존, 적 피격 몽타주)
       · Event.Combat.ExecutionStarted 재전송 (★신규, Instigator=적, Target=플레이어)
  → 플레이어 GA_PlayerExecution 자동 발동 (GameplayEvent 트리거)
       · State.Combat.Invulnerable + State.Camera.Cinematic 부여
       · MotionWarp → 적 정면(GrappleDistance)
       · AddGameplayCue(CameraCueTag) → 시네 카메라 (현재 비움=스킵)
       · 피니셔 몽타주 재생
  → 몽타주 끝 → OnCleanup: 두 태그 제거 + RemoveGameplayCue
```
- **왜 적이 재전송**: 처형 시작 판정(Staggered+강공)이 적 측에 이미 있음 → 적이 단일 심판, 1발 쏴서 플레이어 깨움.
- **왜 GameplayEvent**: 코드베이스 표준 `SendGameplayEventToActor`(§1-2 정합). GameplayMessageSubsystem은 사용처 0.

## 신규/수정 클래스
| 구분 | 클래스 | 경로 | 상속 | 책임 |
|---|---|---|---|---|
| 신규 | `UGA_PlayerExecution` | `AbilitySystem/Abilities/Player/Execution/` | `UGA_ActionBase` | GameplayEvent 자동발동 → i-frame+시네태그 → 적 정면 워프 → 카메라 큐 → 피니셔 몽타주 → OnCleanup 원복 |
| 신규 | `UPlayerExecutionProfile` | `AbilitySystem/Abilities/Player/Execution/` | `UPrimaryDataAsset` | 피니셔 데이터(`FinisherMontage`/`CameraCueTag`/`GrappleDistance`=150) |
| 신규 | `AGCN_ExecutionCamera` | `AbilitySystem/GameplayCues/` | `AGameplayCueNotify_Actor` | OnActive=시네 카메라 / OnRemove=원복 |
| 수정(파트너) | `UExecutionComponent` | `Combat/` | — | `ExecutionInstigator` 캐시 + `Event.Combat.ExecutionStarted` 재전송 2줄 |

## 신규 태그
- `Ability.Mugong.Execution`, `Event.Combat.ExecutionStarted`, `GameplayCue.Camera.Execution`
- 재사용: `State.Combat.Invulnerable`, `State.Camera.Cinematic`, `GameplayCue.Combat.Execution`(적 측)

## 입력 = B안 F키 전용 (2026-06-13)
- F키 = 임시 전용 처형키. 강공(우클릭)은 일반 강공 그대로.
- 신규 `TryExecute()`(KDPlayerCharacter, BlueprintCallable): 락온/최근접 적 → Staggered 게이트 → 가짜 `Event.Combat.Hit` 1발(강공 GA 안 켜니 헤비 몽타주 안 나감).
- 입력 바인딩 = C++ `KDPlayerController`: `IA_Execute` UPROPERTY + `Handle_Execute` → `PC->TryExecute()`. (BP는 IA_Execute 에셋+IMC F키 매핑+Class Defaults 할당만)
- 데미지/포이즈 중복 X: 가짜 이벤트는 데미지 GE 없음(처형 데미지=ExecutionProfile), 적 OnHitReceived는 경직 중 early-return.

## 거리 3종 (헷갈리지 말 것)
- `ExecutionRange` = 발동 게이트 (`KDPlayerCharacter.h`, 기본 250, EditAnywhere)
- `GrappleDistance` = 워프 착지 오프셋 (`PlayerExecutionProfile`, 150)
- `LockOnRadius` = 후보 탐색 (`LockOnConfig`, ~1000)

## 맞물림 = 수동 Motion Warping 확정
CAS(정밀하나 모션당 셋업 부담) vs 수동 워프 vs 참조메시. → **수동 워프**(처형 모션 자주 갈아끼울 예정, 모션당 잡일 최소). 새 처형 = Profile에 몽타주+GrappleDistance만(C++ 0).
- 에셋: `GhostSamurai_Execution##`=Root(공격자 다가감) / `Executed##`=Inplace(적 제자리).
- 맞물림 테스트 = Sequencer 스크럽(게임 안 켜고). GrappleDistance 기준값 = 150~180(길동-밴디트 가로 간격 ≈180 기준).

## 검증 (PIE)
- ✅ 처형 발동 성공: 적 경직 → F → 헤비 몽타주 없이 피니셔+적 피격 몽타주 재생.
- ✅ F키 입력 동작(컨트롤러 바인딩 추가 후).
- 미완: 시네마틱 카메라(레벨 시퀀스, 순수 BP) = 진행 중 → 별도 dev-log.
- 튜닝: GrappleDistance 300→150, AM_killdong_Execution01 루트모션 확인.
