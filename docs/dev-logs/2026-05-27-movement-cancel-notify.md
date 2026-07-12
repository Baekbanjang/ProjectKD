# Movement Cancel Notify + SB 메커니즘 조사 (2026-05-27)

**상태**: ✅ ANS_MovementCancel 구현 + PIE 검증 통과. SB식 snappy 이동 캔슬 작동.

## 작업 요약
- 새 AnimNotifyState `ANS_MovementCancel` 추가 — 공격 후반에 활성, 이동 입력 시 어빌리티 캔슬
- 새 태그 `State.Combat.MovementCanCancel` 추가
- `KDPlayerController::Handle_Move` 캔슬 가드 추가
- ASC 접근 함정 (IAbilitySystemInterface) 발견 + 수정

---

## 배경 — A-2/3 입력 케이던스 PlayRate 조사 결과

작성자가 SB 플레이하면서 느낀 "입력 속도에 따라 몽타주 빨라짐/느려짐" 구현 전 인터넷 조사.

### 조사 결론
**PlayRate 동적 조정은 실무 표준 패턴 X**. 확인된 6개 출처 + SB GameFAQs 본문 + ResetEra 28개 댓글 통틀어 0건.

SB의 다이내믹 컴뱃 = 다음 3가지 메커니즘 조합:
1. **각 콤보 분기마다 inherent 다른 속도** — Quick/Incursion/Onslaught Montage 자체가 다른 길이/속도로 제작
2. **Stellar Blending** — mid-combo 입력으로 다른 분기 점프
3. **Offset (Dodge/Sprint Offset)** — 회피로 캔슬해도 콤보 체인 유지

작성자 직접 관찰은 **보류**. 인터넷 조사 = 2차 자료 한계로 SB 코드 디스어셈블 X. M1 끝나고 다른 메커니즘 다 적용해본 후 그래도 부족하면 PlayRate 1.0~1.2 미세 변동 재검토.

### 조사 출처
- vorixo GAS truth, tranek GASDocumentation, Druid Mechanics GAS Course
- IndieProfessor UE5 Melee, Unreal University UE5 Combo, Lyra Forum
- ExpertGameReviews SB Combo Mastery, GameFAQs SB Mechanics
- ResetEra Combat Design, Sekiro Medium 디자인 분석

자세한 분석: `memory/reference_combat_research_2026-05-27.md`

---

## SB 분석 폴더 인지 (바탕화면 스블/)

작성자가 CUE4Parse로 분석한 SB 자체 아키텍처 18개 .md 폴더 인지.

### 핵심 발견
- **SB는 GAS 안 씀** — GA_/GE_/GC_ 접두사 0개. MoveCurve 131개 + BehaviorTree 149개 커스텀 시스템
- **우리 프로젝트 = 13번 파일의 UE5+GAS 재설계안 거의 100% 실행 중** — 작성자가 SB 분석 결과를 우리 프로젝트에 매핑
- **타격감 3층 구조** (HitStop/BoneShake/VertexShake) — Layer 1 일부만, Layer 2/3 dev item 후보
- **MoveCurve 패턴** — 콤보 분기 다양화 시 CurveFloat로 데이터 드리븐 컨트롤 가능

자세한 인덱스: `memory/reference_sb_analysis_docs_2026-05-27.md`

---

## Movement Cancel 구현

### 의도
공격 모션 끝나면 → 사용자가 이동 입력하면 → 즉시 어빌리티 캔슬 + locomotion 진입.

문제: 현재 시스템은 Montage 완전 종료 (idle 정지 프레임 포함) 후에야 이동 입력 먹힘 → 답답함.

### 결정 — 새 노티 (옵션 A)

기존 `ANS_CancelWindow`와 의미 분리:
- `ANS_CancelWindow` = 어빌리티 캔슬 (Light/Heavy/Dodge로 체인)
- `ANS_MovementCancel` = 이동 캔슬 (locomotion 진입)

별도 노티로 두면 타이밍 컨트롤 + 미세 튜닝 자유도 ⬆️.

### 변경 파일

#### 1. KDGameplayTags.h/cpp
새 태그 `State.Combat.MovementCanCancel` 선언/정의. State_Combat_PerfectParryReady 다음에 배치.

#### 2. ANS_MovementCancel.h/cpp (신규)
경로: `Source/Project_KD/AbilitySystem/AnimNotify/ANS_MovementCancel.h/cpp`

- Parent: `UAnimNotifyState`
- NotifyBegin: ASC에 `State.Combat.MovementCanCancel` loose 태그 부여
- NotifyEnd: 태그 제거
- ASC 접근: `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner())` — IAbilitySystemInterface 활용

#### 3. KDPlayerController::Handle_Move
ControlledPawn null 체크 직후, AddMovementInput 전에 가드 추가:
```cpp
if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn))
{
    if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_MovementCanCancel))
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(GameplayTags::Ability_Mugong_Light);
        CancelTags.AddTag(GameplayTags::Ability_Mugong_Heavy);
        ASC->CancelAbilities(&CancelTags);
    }
}
```

#### 4. Montage 자산 7개에 노티 박기 (작성자 영역)
- AM_LightAttack_Hit1~4
- AM_HeavyAttack_Hit1~3
- 위치: WeaponTrace 노티 끝 ~ Montage 끝 (recovery 구간)

---

## 디버깅 함정 — ASC 접근 방식

### 초기 구현 실패
```cpp
// 안 됨 — Player ASC는 PlayerState에 있어서 Pawn에 컴포넌트로 안 박힘
ControlledPawn->FindComponentByClass<UAbilitySystemComponent>()
// → nullptr 반환 → if 블록 진입 X → 캔슬 안 됨
```

### 원인
- Player: ASC가 PlayerState에 있고, Pawn(ABaseCharacter)에는 **캐시 포인터(UPROPERTY)만**
- `FindComponentByClass`는 실제 컴포넌트만 검색 → 캐시 포인터는 못 찾음
- Enemy(KDEnemyBaseCharacter)는 Pawn-direct ASC라 `FindComponentByClass` 정상 작동 — 비대칭 함정

### 해결 — IAbilitySystemInterface 활용
ABaseCharacter는 `IAbilitySystemInterface` 구현 (`GetAbilitySystemComponent()` override). 이걸 거치는 canonical 방식:
```cpp
UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(ControlledPawn)
```
내부에서 IAbilitySystemInterface 거쳐 PlayerState ASC 정확히 반환.

### 회귀 방지 노트
**Player ASC 접근 시 `FindComponentByClass` 금지**. 항상 `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent` 또는 `IAbilitySystemInterface::GetAbilitySystemComponent` 사용. Enemy는 Pawn-direct라 둘 다 OK지만 일관성 위해 BPLibrary 권장.

---

## 아키텍처 관찰 — PlayerController 결합도

작성자가 의문 제기: KDPlayerController가 거의 모든 Handle_*에서 `Cast<APlayerCharacter>` 사용. 결합도 ⬆️.

### 현재 패턴 분류
- **무캐스팅** (Handle_Move, Handle_Look): Pawn 기본 메소드만 사용. 결합도 최소.
- **엔진 클래스 캐스팅** (Handle_Jump): `Cast<ACharacter>`. 표준 UE5 클래스, 프로젝트 종속 X.
- **프로젝트 캐릭터 캐스팅** (Handle_LightAttack/Heavy/Dodge/Parry/Sprint): `Cast<APlayerCharacter>` + `Try*` 호출. **결합도 ⬆️ — 새 캐릭터 빙의 또는 멀티 진입 시 문제 가능**.

### 이번 Movement Cancel은 패턴 1+ (디커플링)
- `Cast<APlayerCharacter>` 안 함
- `UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent` — ASC만 의존
- = 결합도 추가 없음

### 결합도 리팩토링 — M2 폴리싱 dev item
M2~M3에 별도 작업:
1. **Interface 기반 리팩토링** — Try* 메소드 유지, Controller에서 IAbilitySystemInterface 거침
2. **Lyra식 InputID 바인딩** — Enhanced Input + AbilityInputID Map (대규모 작업)

지금은 일관성 + Movement Cancel 빠른 검증 우선 → 그대로 유지.

---

## PIE 검증 결과

| 시나리오 | 입력 | 결과 |
|---|---|---|
| Light 1타 가만히 | 공격 후 멈춤 | Montage 자연 종료 ✅ |
| Light 1타 → swing 끝나고 이동 | 공격 후 즉시 W | 어빌리티 캔슬 + 즉시 locomotion ✅ ← 핵심 |
| Light 1타 swing 도중 이동 | 공격 직후 W | 캔슬 X (MovementCanCancel OFF) ✅ |
| Light → Heavy → 이동 | 콤보 중 W | Heavy 후반에 캔슬 + 이동 ✅ |

**작성자 확인**: "굿 잘된다잉" → 성공.

---

## 변경 파일

### 코드
- `Source/Project_KD/KDGameplayTags.h/cpp` — 새 태그 추가
- `Source/Project_KD/AbilitySystem/AnimNotify/ANS_MovementCancel.h/cpp` — 신규 클래스
- `Source/Project_KD/Player/KDPlayerController.cpp::Handle_Move` — 캔슬 가드 + BPLibrary 인클루드

### 자산
- AM_LightAttack_Hit1~4 + AM_HeavyAttack_Hit1~3 — ANS_MovementCancel 노티 추가

---

## 다음 dev item 후보

1. **콤보 분기 다양화** (Quick 식 빠른 광역 + Onslaught 식 느린 강공 + 차징 분기) — SB 메커니즘 정합
2. **Stellar Blending** — mid-combo 입력 변경으로 분기 점프
3. **타격감 3층** (HitStop/BoneShake/VertexShake) — SB 13번 설계안 B-3
4. **PlayerController 결합도 리팩토링** — Try* 메소드 IAbilitySystemInterface 패턴 (M2 폴리싱)
5. **PlayRate 동적 조정 재검토** (보류 항목) — 위 모든 거 해본 후 그래도 부족하면 1.0~1.2 미세 변동
