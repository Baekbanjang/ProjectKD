# 2026-08-25 — 리팩토링 A·E·B1: 중복 통합 + 죽은 코드 정리

## 0. 요약

전수 진단(하청 4개 조사 + Fable 판정) 후 확정 목록 중 A(소품)·E(공통화)·B1(콤보 통합)을 하청 실행. **작성자가 편집을 명시 허가한 세션** — 예외적으로 AI가 소스를 직접 수정했다.

소스 34파일 `+263/-291` (순감소 28줄). 빌드 3회 전부 통과. **PIE 미검증** (아래 §5).

| 묶음 | 내용 |
| --- | --- |
| A | 복붙 2줄 삭제 / `ActiveCameraCue` 잔류 리셋 / 회피 진입 DA화 / 캔슬 판단 이사 / 죽은 코드 8건 |
| E | 데미지 파이프라인 3종 → `KDAbilityStatics` / Self-GE 적용 → `GA_ActionBase::ApplySelfEffect` |
| B1 | 지상·공중 콤보의 노드 소비 중복 20줄 → `GA_PlayerMeleeAttackBase::ApplyComboNode` |

미착수 이월 = B2(`OnHitReceived` 4분해) · C1~C5(각자 기능 작업에 묶음). 진단 전문과 SB 구조 비교는 세션 대화 기록.

---

## 1. A 묶음 — 소품 5건

### A1 · 복붙 삭제
`KDPlayerAnimInstance.cpp:39-42` — `CachedVelocity`/`CachedActorRotation` 같은 대입 2회 → 1회.

### A2 · InstancedPerActor 잔류 리셋
`GA_PlayerExecution::ActivateAbility` 첫머리에 `ActiveCameraCue = FGameplayTag();`. 다른 GA 6개는 전부 지키던 패턴인데 이것만 빠져 있었다. Profile에 `CameraCueTag` 없는 처형이 생기면 이전 태그가 잔류하던 경로.

### A3 · 회피 진입 하드코딩 해소
- `ComboTreeDataAsset`에 `EvadeEntryId`(기본 `Evade`) / `JustEvadeEntryId`(기본 `JustEvade`) 칸 신설
- `ComboComponent::EnterEvadeNode(bool bPerfect)` 신설 — DA에서 ID를 읽어 `EnterNode(id, 0.8f)`
- `KDPlayerAbilityInputComponent`의 중복 2곳(`ConsumeBufferedInput`/`TryDodge`)이 이 한 줄 호출로 통합
- `0.8f`는 함수 안 상수로 유지 — `FComboNode.InputWindow` 값 채울 때(C1) 흡수 예정

### A4 · 캔슬 판단을 InputComponent로 (1안 채택)

**결정 기록** — 두 목록은 다른 개념이 맞다(작성자 확정):
```
MovementCancelableTags   이동 입력이 끊을 수 있는 것    Light / Heavy / Dodge / SprintAttack
AttackCancelableTags     캔슬 윈도우에서 다음 공격이 끊는 것   Light / Heavy / SprintAttack / CounterThrust
```
- `CounterThrust`가 이동 목록에 없는 것 = **의도** (반격 후딜은 이동으로 못 끊는다)
- 저장 방식 = **1안(컴포넌트 `UPROPERTY` 컨테이너 2개)**. 2안(GA 자기 태그 선언)은 기획 가독성이 떨어져 기각 — GA 수십 개 되면 재검토
- `Handle_Move`의 GAS 블록 → `TryMovementCancel()` 위임. **"Controller가 GAS 만지는 유일 지점" 예외가 소멸**했다
- 겸사: `bAttacking/bDodging/bCanCancel` 3종 조회 4곳 → 파일 내 `QueryComboGateState()` 헬퍼

⚠️ 컨테이너 기본값은 C++ 생성자가 단일 진실. BP에 안 내려오는 함정(볼트 `BP-네이티브컴포넌트_생성자값이_BP에_안먹힘`) 대상이라 PIE에서 1회 확인 필요.

### A5 · 죽은 코드 8건 삭제 (C++ 참조 0 + BP 그래프 전수 열람으로 검증)
```
SlopeControlCurve (카메라매니저) / IsWalking (캐릭터) / GetConfig · OnLockOnTargetChanged 델리게이트 (락온)
IsBeingExecuted (처형) / GetCurrentNodeId (콤보) / PlaySequenceAtActor (시네마틱) / IsHitStopActive (히트스톱)
+ 태그 GameplayCue.Combat.Execution / GameplayCue.Camera.Execution
```
- 유일 생존자 = `TriggerFovPunch` — `GCN_CounterThrust`가 BP에서 호출 중이라 유지
- `RequestHitStop`은 내부 사용 중이라 유지

---

## 2. E 묶음 — 공통화 2건

### E1 · 데미지 파이프라인 → `KDAbilityStatics` 3종

근접(`GA_MeleeTraceBase::OnWeaponHit`)과 총격(`GA_ShotBlast::ApplyHit`)의 히트 처리 30줄이 100% 동일했다. 세 가지 지식으로 분해해 statics로:

```cpp
IsFriendlyFire(AttackerASC, TargetASC)          // 팀 이분법 단일 정의처 — 팀 확장 시 여기 하나
ApplyDamageEffect(..., FinalAttackPower, Hit, SourceActor)  // Context+SetByCaller+ApplyToTarget, Context 반환
SendHitEvent(HitActor, Instigator, Tags, Context, KnockbackMul)  // Event.Combat.Hit 5필드 규약
```

- `KDProjectile`은 스펙 선생성 구조라 `IsFriendlyFire`+`SendHitEvent`만 재사용 (`ApplyGameplayEffectSpecToSelf`는 유지)
- 처형 칩뎀(`ExecutionComponent`)은 SetByCaller 없는 flat GE가 의도라 제외
- **`EventMagnitude` = 넉백 배수 계약이 이제 함수 시그니처로 강제된다** (종전엔 관례)

### E2 · Self-GE 적용 → `GA_ActionBase::ApplySelfEffect`

무적/블록/패링 GE를 자기에게 거는 3줄 골격 4곳(`GA_Dodge` 람다×3소비 / `GA_Parry`×2 / `GA_EnemyParry`) → protected 헬퍼 하나. `GA_Dodge`의 자체 람다는 삭제하고 헬퍼로 승격.

---

## 3. B1 — 콤보 노드 소비 통합

`GA_PlayerAttackBase`(지상)와 `GA_PlayerAirAttackBase`(공중)의 `ActivateAbility`에 같은 20줄이 두 벌 있었다 — `FComboNode`에 칸이 늘 때마다(계수·InputWindow·워프 거리 예정) 두 곳을 고쳐야 했고 한쪽만 고치면 에러 없이 갈라지는 구조.

```cpp
// GA_PlayerMeleeAttackBase (공통 조상)
const FComboNode* ApplyComboNode(FGameplayTag InputTag, EComboContext Context,
    TSubclassOf<UGameplayEffect> DefaultGE, float DefaultDamageMul, float DefaultKnockbackMul);
```

- 지상 41줄 → 14줄 / 공중 47줄 → 19줄. `bIsFinisher` 계산은 반환 노드로 공중 호출부에 유지
- `InputTag` 파라미터는 설계에서 빠졌던 것 — `ComboInputTag`가 부모가 아닌 각 자식 소속이라 인자로 받는다
- 두 벌 사이 유일한 차이 = 로그 문구(`Combo node` vs `Air combo node`) → Context 분기로 바이트 동일 보존

---

## 4. 검증

- 빌드 3회(A/E/B1 각 묶음 후) 전부 성공. A 묶음에서 하청이 자기 컴파일 에러 1건(`bCanCancel` 잔여 참조 3곳)을 잡아 수정 후 통과
- `C4996 AbilityTags deprecated` 경고 2건은 기존부터 있던 것(`GA_CounterThrust`/`GA_SprintAttack`, 미변경 파일)
- 에셋·Config 변경 0 / 커밋 0 (요청 대기)
- ⚠️ 하청 편집분 3파일이 LF 개행(`GA_PlayerAttackBase.cpp`·`KDAbilityStatics.h/.cpp`) — git이 커밋 시 CRLF로 정규화한다는 경고만, 기능 무관

## 5. 남은 것

**PIE 스모크 (미실행)**
```
지상 콤보 4타 / 공중 콤보          데미지·계수 종전과 동일한가 (E1·B1 회귀)
회피 → 콤보 합류 / 저스트 회피      Evade·JustEvade 진입 (A3)
공격 후반 이동 입력                 이동 캔슬 (A4) + BP에서 컨테이너 2개 값 확인
콤보 총격 + 조준 사격 / 화살 맞기    ShotBlast·Projectile 경로 (E1)
패링 홀드 / 퍼펙트 패링 / 적 패링    ApplySelfEffect (E2)
처형 2회 연속                      카메라 큐 잔류 (A2)
카운터 성공                        FOV 펀치 생존 확인
```

**커밋 제안 (3분할)**
```
[refactor] 캔슬 판단 InputComponent 이사 + 회피 진입 DA화 + 죽은 코드 정리   (A)
[refactor] 데미지 파이프라인·Self GE 공통화 - KDAbilityStatics / GA_ActionBase   (E)
[refactor] 콤보 노드 소비를 GA_PlayerMeleeAttackBase 로 통합   (B1)
```

**이월** — B2(`OnHitReceived` 4분해, SB ResultTable 축) / C1(콤보 값 채우기 — B1 완료로 선행조건 해소) / C2(넉백 RootMotionSource + KnockbackComponent) / C3(OneShot 층 제거) / C4(락온 LoS 통일) / C5(PostGEExec 헬퍼 분리)
