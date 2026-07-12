# Deep Interview Spec: Player ABP + 무기 장착 시스템

## Metadata
- Interview ID: w1-abp-weapon
- Rounds: 9
- Final Ambiguity Score: 20%
- Type: brownfield
- Generated: 2026-05-21
- Threshold: 20%
- Status: PASSED

## Clarity Breakdown
| Dimension | Score | Weight | Weighted |
|-----------|-------|--------|----------|
| Goal Clarity | 0.90 | 0.35 | 0.315 |
| Constraint Clarity | 0.78 | 0.25 | 0.195 |
| Success Criteria | 0.70 | 0.25 | 0.175 |
| Context Clarity | 0.75 | 0.15 | 0.113 |
| **Total Clarity** | | | **0.798** |
| **Ambiguity** | | | **20%** |

## Goal
길동 캐릭터 전용 ABP를 SB식(Locomotion BlendSpace + Montage overlay) 구조로 생성하고, C++ WeaponComponent로 창(Spear) 메시를 소켓에 부착하여 GA_WeaponTraceBase와 연동한다.

## Constraints
- 무기: 창(Spear) 단일, 항상 장착 (Equip/Unequip 없음)
- ABP 구조: Locomotion BlendSpace + DefaultSlot Montage + Layered Blend per Bone (Spine 기준, 공중 QTE 대비)
- 스켈레톤: 길동 고유 메시, IK Retargeter로 마네퀸 애니메이션 리타겟 완료
- 무기 부착: C++ WeaponComponent → SkeletalMeshComponent + "Weapon" 태그 → 소켓 부착
- 소켓: 길동 스켈레톤에 `weapon_r` 소켓 추가 필요 (Editor 작업)
- 확장성: 향후 무기 추가 시 새 몽타주 + GA 서브클래스 + WeaponComponent 메시/소켓 변경으로 ABP 수정 없이 확장
- 기존 GA_WeaponTraceBase의 태그 기반 탐색("Weapon") 및 소켓(Spear_Bottom/Spear_Tip) 유지

## Non-Goals
- 무기 교체 시스템 (W1 범위 밖)
- Equip/Unequip 애니메이션
- Linked ABP 레이어 분리
- 커스텀 스켈레톤 리타겟 (이미 완료)
- 전투 줌 카메라 (삭제됨)

## Acceptance Criteria
- [ ] PIE: 길동이 창을 들고 Idle/Walk/Run 애니메이션 정상 재생
- [ ] PIE: LMB으로 공격 몽타주 재생 (Montage가 상체/전체 덮어쓰기)
- [ ] PIE: 무기 트레이스가 창 메시 Spear_Bottom/Spear_Tip 소켓에서 정상 동작

## Assumptions Exposed & Resolved
| Assumption | Challenge | Resolution |
|------------|-----------|------------|
| 검 + 창 둘 다 쓸 것 | Content에 Sword_Animations도 있어서 | 창 단일로 확정 |
| 무기 탈착이 필요할 것 | SB 스타일 질문 | 항상 장착, 탈착 없음 |
| Linked ABP가 필요할 것 | 공중 QTE 상체 분리 | Layered Blend per Bone으로 해결, Linked ABP 불필요 |
| BP에서 무기 수동 부착 | 확장성 질문 | C++ WeaponComponent로 확정 |
| 리타겟이 안 됐을 것 | IK Retargeter 확인 | 이미 셋업 완료 |

## Technical Context

### 기존 인프라 (재사용)
- `GA_WeaponTraceBase` — 컴포넌트 태그 "Weapon"으로 메시 탐색, Spear_Bottom/Spear_Tip 소켓 트레이스
- `AT_WeaponTrace` — 프레임별 무기 트레이스 AbilityTask
- `AnimNotifyState_WeaponTrace` — 몽타주에서 트레이스 시작/종료 이벤트 발사
- `AnimNotifyState_CancelWindow` — 캔슬 윈도우 태그 부여
- `KDGameplayTags` — 기존 태그 17개 그대로 사용
- IK Retargeter: 마네퀸 → 길동 리타겟 완료

### 새로 만들 것
1. **UWeaponComponent** (C++) — SkeletalMeshComponent 소유, 소켓 부착, "Weapon" 태그
2. **ABP_Player** (BP) — 길동 스켈레톤 기준, Locomotion BlendSpace + DefaultSlot + Layered Blend per Bone
3. **BS_Locomotion** (BP) — Idle/Walk/Run BlendSpace (Speed 기준)
4. **weapon_r 소켓** — 길동 스켈레톤 오른손에 추가

### ABP 구조
```
[State Machine: Locomotion]
  ├─ Idle/Walk/Run → BS_Locomotion (Speed 기반)
  ├─ Jump_Start / Jump_Loop / Jump_End
  └─ (Output Pose)
       │
[Layered Blend per Bone] (Spine 기준)
  ├─ Base: Locomotion 출력
  ├─ Layer: DefaultSlot (Montage)
  └─ Output: Final Pose
```

### WeaponComponent 구조
```
UWeaponComponent : UActorComponent
  ├─ UPROPERTY: TObjectPtr<USkeletalMeshComponent> WeaponMesh
  ├─ UPROPERTY(EditDefaultsOnly): USkeletalMesh* WeaponMeshAsset
  ├─ UPROPERTY(EditDefaultsOnly): FName AttachSocketName = "weapon_r"
  └─ BeginPlay: 메시 생성 → 소켓 부착 → "Weapon" 태그 설정
```

## Ontology (Key Entities)
| Entity | Type | Fields | Relationships |
|--------|------|--------|---------------|
| PlayerCharacter | core domain | Mesh, ABP, Components | owns WeaponComponent, uses ABP_Player |
| WeaponComponent | core domain | WeaponMesh, SocketName, ComponentTag | attached to PlayerCharacter, found by GA |
| ABP_Player | supporting | StateMachine, BlendSpace, MontageSlot, LayeredBlend | drives PlayerCharacter animation |
| BS_Locomotion | supporting | Speed axis, Idle/Walk/Run | used by ABP_Player |
| GA_WeaponTraceBase | core domain | AttackMontage, TraceMode, Sockets | finds WeaponComponent by tag |
| Spear Mesh | supporting | Bottom/Tip sockets | owned by WeaponComponent |
| IK Retargeter | external | Mannequin→길동 mapping | enables animation reuse |

## Interview Transcript
<details>
<summary>Full Q&A (9 rounds)</summary>

### Round 1
**Q:** 흑길동의 주 무기가 뭐야?
**A:** 창(Spear) 단일
**Ambiguity:** 78% (Goal: 0.30, Constraints: 0.10, Criteria: 0.10, Context: 0.40)

### Round 2
**Q:** 창을 항상 들고 다니는지, 등에 차고 다니다가 전투 시 미는 건지?
**A:** 항상 장착 (SB 스타일)
**Ambiguity:** 66% (Goal: 0.50, Constraints: 0.30, Criteria: 0.10, Context: 0.45)

### Round 3
**Q:** ABP 스테이트 머신 구조 — Locomotion+Montage vs SM분리 vs Linked ABP?
**A:** 확장성 좋은 걸로
**Ambiguity:** 60% (Goal: 0.60, Constraints: 0.35, Criteria: 0.10, Context: 0.50)

### Round 4
**Q:** SB식으로 확정할까?
**A:** 공중 QTE 상체만 때리는 걸 구상중인데 SB식이 맞나?
**Ambiguity:** 53% (Goal: 0.75, Constraints: 0.40, Criteria: 0.10, Context: 0.55)
→ Layered Blend per Bone으로 SB식 내에서 해결 가능 확인

### Round 5
**Q:** 마네퀸 기준 전투 애니메이션 구체적으로 뭐가 있는지? 리타겟 상태?
**A:** 스켈레톤 메시는 길동, ABP는 마네퀸꺼
**Ambiguity:** 50% (Goal: 0.75, Constraints: 0.45, Criteria: 0.15, Context: 0.55)

### Round 6
**Q:** IK Retargeter 셋업 됐는지?
**A:** IK Retargeter 셋업됨
**Ambiguity:** 41% (Goal: 0.80, Constraints: 0.65, Criteria: 0.20, Context: 0.65)

### Round 7
**Q:** 무기 부착 방식 — BP수동 vs C++ WeaponComponent vs 이미 붙여놨는지?
**A:** 모듈성, 확장성 고려하면 어떤 방식?
**Ambiguity:** 36% (Goal: 0.85, Constraints: 0.70, Criteria: 0.25, Context: 0.70)
→ C++ WeaponComponent 추천, CLAUDE.md §1-2 전용 Component 패턴 부합

### Round 8
**Q:** 완료 판단 기준?
**A:** 기본 검증만 (Idle/Walk/Run + 공격 몽타주 + 무기 트레이스)
**Ambiguity:** 21% (Goal: 0.90, Constraints: 0.75, Criteria: 0.70, Context: 0.75)

### Round 9
**Q:** 길동 스켈레톤에 무기 부착용 소켓 있는지?
**A:** 없음/모르겠음
**Ambiguity:** 20% (Goal: 0.90, Constraints: 0.78, Criteria: 0.70, Context: 0.75)
→ Editor에서 weapon_r 소켓 추가 필요

</details>
