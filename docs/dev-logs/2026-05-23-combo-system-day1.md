# 콤보 시스템 구현 Day 1 — 진행 로그

**날짜**: 2026-05-23
**작업**: W1 우선순위 3-A 단계 3a~3c (콤보 시스템 구조)
**상태**: 단계 3c까지 완료. 단계 3d (GA_LightAttack) 부터 내일 재개.

---

## 오늘 완료 작업

### 단계 3a — 태그 2개 추가
`Source/Project_KD/KDGameplayTags.h/.cpp`에 다음 추가:
- `Input_Combo_Light` ← `"Input.Combo.Light"`
- `Input_Combo_Heavy` ← `"Input.Combo.Heavy"`

UComboComponent의 InputHistory 식별용. M1엔 hL 미도입이라 ChargedLight 제외.

### 단계 3b — UComboTreeDataAsset 클래스
경로: `Source/Project_KD/AbilitySystem/Combo/ComboTreeDataAsset.h/.cpp`

핵심 정의:
```cpp
USTRUCT(BlueprintType)
struct FComboBranch
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
    FName BranchName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
    TArray<FGameplayTag> InputSequence;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
    TObjectPtr<UAnimMontage> Montage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
    TSubclassOf<UGameplayEffect> DamageEffectClass;
};

UCLASS(BlueprintType)
class PROJECT_KD_API UComboTreeDataAsset : public UDataAsset
{
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combo")
    TArray<FComboBranch> Branches;
};
```

순수 데이터만. 매칭 로직은 ComboComponent가 담당 (책임 분리).
.cpp는 UE 자동 생성한 빈 상태 그대로.

**빌드 통과 확인됨**.

### 단계 3c — UComboComponent 클래스
경로: `Source/Project_KD/AbilitySystem/Combo/ComboComponent.h/.cpp`

핵심 API:
- `ProcessInput(FGameplayTag InputTag)` — C++ 전용. 정확 매칭 시 `const FComboBranch*` 반환, 그 외 `nullptr`
- `ClearHistory()` — BlueprintCallable. 외부 강제 리셋 (M2 F11 피격 끊김 연동)

UPROPERTY:
- `ComboTree` (UComboTreeDataAsset*) — BP에서 DA_ComboTree 할당
- `ComboResetTime` (float = 1.5f) — UPROPERTY 노출, 튜닝 가능

알고리즘 (3단계 매칭):
1. **정확 매칭**: InputHistory 길이 = 분기 길이 & 모든 원소 일치 → 분기 반환 + ClearHistory
2. **Prefix 매칭**: 분기 길이 > InputHistory 길이 & 시작 부분 일치 → nullptr (진행 중)
3. **미매칭**: 위 둘 다 아님 → InputHistory.Reset() + Add(InputTag) → nullptr (새 시퀀스 시작)

Timer 1.5s — `SetTimer(ResetTimerHandle, ..., ComboResetTime, false)`. 매 입력 시 재시작. 만료 시 ClearHistory.

**빌드 검증 미완** — P4V 충돌 해결 후 빌드 안 했을 가능성. 단계 3d 빌드 시 같이 확인.

---

## 콤보 시스템 동작 원리 (핵심 이해)

### nullptr 반환 = "지정한 몽타주만 재생"
```
좌클릭 → ProcessInput → nullptr (Prefix 진행 중)
  ↓
GA가 AttackMontage 변수를 안 바꿈
  ↓
부모 GA_WeaponTraceBase가 BP에서 박은 디폴트 AttackMontage 재생
  ↓
화면엔 단발 휘두르기 모션
```

### FComboBranch* 반환 = "분기 마무리 몽타주 재생"
```
정확 매칭 (예: 4타째 [L,L,H,H] = Incursion II)
  ↓
GA가 AttackMontage = MatchedBranch->Montage 교체
  ↓
부모가 분기 Montage 재생
  ↓
화면엔 Incursion II 마무리 모션
```

### Incursion II 완주 시뮬레이션
| 입력 | ProcessInput 반환 | GA 동작 | 재생 |
|---|---|---|---|
| L | nullptr (Prefix) | AttackMontage 그대로 | 디폴트 단발 |
| L | nullptr (Prefix) | AttackMontage 그대로 | 디폴트 단발 |
| H | nullptr (Prefix) | AttackMontage 그대로 | 디폴트 단발 |
| H | &Incursion_II (정확 매칭) | AttackMontage 교체 | Incursion II 마무리 |

**1~3타는 같은 디폴트 단발, 4타에만 다른 마무리 모션** = SB식 콤보 본질.

---

## 환경 이슈

### P4V "Delete or Replace Writable Files" 충돌
KDGameplayTags.h/.cpp가 체크아웃 안 된 채로 writable 수정 → P4가 sync 시 충돌 다이얼로그.
해결: Cancel → Reconcile Offline Work → 체크아웃 처리 → sync 재시도.

### 페어 변경 발견
필규가 KDGameplayTags에 `GameplayCue_Enemy_Telegraph_Parryable/Unblockable` 2개 추가. 충돌 없이 공존.

---

## 🌅 내일 재개 (단계 3d)

### 진입 전 확인
1. UComboComponent.cpp 빌드 통과 (단계 3c)
2. UE 에디터에서 ComboTreeDataAsset 인스턴스 생성 가능 (BlueprintType 동작 검증)

### 단계 3d — GA_LightAttack 핵심

**파일**: `Source/Project_KD/AbilitySystem/Abilities/GA_LightAttack.h/.cpp`
**부모**: `UGA_WeaponTraceBase`

**필수 신규 UPROPERTY** ⚠️
```cpp
// 1~3타째 단발용 디폴트 Montage. 매 활성화 시작에 부모 AttackMontage 복원에 사용.
UPROPERTY(EditDefaultsOnly, Category = "Action|Montage")
TObjectPtr<UAnimMontage> DefaultAttackMontage;
```

이유: `UGA_WeaponTraceBase::AttackMontage`는 InstancedPerActor 인스턴스 멤버. 이전 활성화에서 분기 Montage로 교체된 값이 다음 활성화에 잔류 → 매번 활성화 시작에 디폴트로 복원 필요.

**ActivateAbility 패턴 (개념)**
```cpp
const FComboBranch* Matched = ComboComponent->ProcessInput(Input_Combo_Light);
if (Matched && Matched->Montage)
{
    AttackMontage = Matched->Montage;
    if (Matched->DamageEffectClass) DamageEffectClass = Matched->DamageEffectClass;
}
else
{
    AttackMontage = DefaultAttackMontage;
}
Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
```

**ComboComponent 접근 방법** (b안 채택)
- PlayerCharacter에 UPROPERTY UComboComponent* 노출
- GA에서 `Cast<APlayerCharacter>(GetAvatarActorFromActorInfo())->ComboComponent` 접근

### 단계 3 잔여 흐름
- 3d: GA_LightAttack
- 3e: GA_HeavyAttack (동일 패턴, Input_Combo_Heavy)
- 3f: PlayerCharacter에 ComboComponent UPROPERTY + CreateDefaultSubobject
- 3g: KDPlayerController Handle 함수 1줄 (ComboComponent 알림)
- 3h: (승환) DA_ComboTree 5분기 셋업 + Montage 임시 할당
- 4: (승환) BP 마무리 (StartupAbilities 등록 + ComboTree 슬롯 + DefaultAttackMontage 슬롯)
- 5: PIE 검증
- 6: dev-log 마무리

### 5분기 데이터 (DA_ComboTree에 등록 예정)
1. Incursion I: [Light, Heavy, Heavy, Heavy]
2. Incursion II: [Light, Light, Heavy, Heavy]
3. Onslaught I: [Heavy, Heavy, Heavy]
4. Onslaught II: [Heavy, Light, Light, Heavy]
5. 순수 4타: [Light, Light, Light, Light]
