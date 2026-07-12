# 콤보 시스템 Day 3 (2026-05-25)

**상태**: ✅ M1 라이트/헤비 콤보 기반 셋업 + PIE 검증 완료

## 작업 요약

- Phase A 옵션 B2 코드 확장 (1·2·3타 모션 다양화)
- Phase B-0 무기 소켓 X2 셋업 (GripPoint 자동 정렬)
- Phase B-1~5 BP/Montage/DA/PlayerState 셋업
- Phase D 디버깅 4건 fix
- Phase E Heavy 추가 + Light/Heavy 헬퍼 통일

---

## Phase A — 코드 확장 (옵션 B2)

**의도**: 1·2·3타가 화면상 같은 모션 반복은 SB식 액션 게임으로 단조로워서 거부 → 모션 다양화.

### 변경
- `ComboComponent.h`: `GetInputHistoryLength()` 인라인 getter 추가 (1줄)
- `GA_LightAttack.h/cpp`, `GA_HeavyAttack.h/cpp`:
  - `DefaultAttackMontage` (TObjectPtr) → **`DefaultAttackMontages` 배열** (TArray<TObjectPtr>)
  - `ActivateAbility`에서 InputHistory 길이로 `(Length - 1) % Num` 인덱싱
  - Combo nullptr 가드 (EnemyAttack 등 비-플레이어 사용 케이스 안전)

### 결과
- 1타 → DefaultAttackMontages[0], 2타 → [1], 3타 → [2], 분기 매칭 → DA Montage 교체
- LHL 시나리오: Light_Hit1 → Heavy_Hit2 → Light_Hit3 (각 GA의 자기 배열 인덱싱)

---

## Phase B-0 — 무기 소켓 X2 셋업

**의도**: 캐릭터 SK 프리뷰 어긋남 우회 + 새 무기 자동 정렬.

### 결정
- 패턴: 캐릭터 측 `weapon_r` 표준 트랜스폼 + 무기 측 `GripPoint` 소켓 + WeaponComponent 자동 정렬 코드
- 프리뷰 vs 게임 메시 분리: 캐릭터 SK Preview Asset = `Spear` 스태틱 (애님 가늠용) / 게임 메시 = `SKM_Woldo` (GripPoint 코드 정렬)

### 변경
- **SKM_killdong weapon_r 표준 트랜스폼**: `(-8.045, 3.435, 1.084) / (80.285, -54.321, -154.147)`
- **SKM_Woldo `GripPoint` 소켓** 추가
- **WeaponComponent.cpp::BeginPlay**:
  - RegisterComponent 순서 변경 (Attach 전에 Register)
  - GripPoint 자동 정렬 1줄: `WeaponMesh->SetRelativeTransform(GripLocal.Inverse())`

### 미래 마이그레이션
- M2 진입 전 SKM_Woldo origin DCC 재임포트로 X1 가능하면 GripPoint 코드 제거 가능
- 페어 MD 표준 룰 추가 보류 (`project_pending_pair_md_updates.md` 메모리)

---

## Phase B-1~5 — BP/Montage/DA/PlayerState 셋업

### 자산
- **Montage**: `AM_LightAttack_Hit1/2/3/Hit4` (각 ANS_WeaponTrace + ANS_CancelWindow 박음)
- **DA**: `DA_LightCombo` (이름 헷갈리는데 일단 유지, 정정 권장: DA_ComboTree)
  - 인덱스 0: Pure4 `[L,L,L,L]` → AM_LightAttack_Hit4
  - 인덱스 1: LHLH_Finisher (Phase E에서 추가)
- **BP**: `BP_GA_LightAttack` — DefaultAttackMontages 배열 + 활성화 소유 태그 `State.Combat.Attacking`
- **BP_PlayerCharacter**: ComboComponent에 DA 바인딩
- **BP_KDPlayerState**: StartupAbilities 등록

---

## Phase D — 디버깅 4건

### D-1. ABaseCharacter Tick OFF ★ (가장 결정적)
**원인**: `ABaseCharacter.cpp:6`에서 `PrimaryActorTick.bCanEverTick = false` → PlayerCharacter Tick 호출 안 됨 → InputBuffer Consume 영원히 안 일어남.

**조치**: `PlayerCharacter` 생성자에 `PrimaryActorTick.bCanEverTick = true;` override.

**진단 우회 길**: 매 Tick 로그 안 찍히는 걸 확인 → Tick 호출 자체 안 됨 결론.

### D-2. Tick의 Light Consume 조건 누락
**원인**: 조건이 `!bDodging || bCanCancel`로 Attacking 검사 빠짐. 1타 진행 중 매 Tick Consume → TryActivate 실패 → 입력 소실.

**조치**: `(!bAttacking && !bDodging) || bCanCancel` + `CancelAbilities(Light/Heavy/Dodge)` 추가.

### D-3. TryLightAttack 중복 호출
**원인**: 디버그 로그 추가하면서 `TryActivateAbilitiesByTag` 2번 호출. 1타 성공 후에도 두 번째 호출 실패 → 가짜 Push 발생.

**조치**: `bActivated` 변수 재사용 (`!bActivated`로 if 분기).

### D-4. InputBuffer BufferTimeWindow 너무 짧음
**원인**: 디폴트 0.2s. Montage 1.83s 길이 + CancelWindow 0.5s 후 열림 → 사용자가 빠르게 누른 입력이 CancelWindow ON 전에 만료.

**조치**: BP_PlayerCharacter → InputBuffer 컴포넌트 → BufferTimeWindow 0.2 → **0.5**.

---

## Phase E — Heavy 추가 + Light/Heavy 헬퍼 통일

### 자산
- **Heavy Montage**: `AM_HeavyAttack_Hit1/2/3` + LHLH 마무리 Montage
- **BP_GA_HeavyAttack**: BP_GA_LightAttack 복제 → Parent를 UGA_HeavyAttack로 변경 → 슬롯 교체
- **DA_LightCombo 인덱스 1**: LHLH_Finisher `[L,H,L,H]`
- **BP_KDPlayerState StartupAbilities**: BP_GA_HeavyAttack 추가

### 코드 — 헬퍼 함수로 통일 ★
**의도**: Light/Heavy 블록이 거의 완전 대칭이라 페어 룰 §3 정합 (2회+ 반복 함수 분리). Light 블록의 CancelTags에 Heavy 누락 버그도 통일로 해결.

**`PlayerCharacter.h` 추가**:
```cpp
private:
    void TryConsumeAndActivate(
        UAbilitySystemComponent* ASC,
        bool bCanCancel,
        const FGameplayTag& InputTag,
        const FGameplayTag& AbilityTag);
```

**`PlayerCharacter.cpp` 추가** (const 안 붙임 — side effect 있는 액션 함수):
```cpp
void APlayerCharacter::TryConsumeAndActivate(
    UAbilitySystemComponent* ASC, bool bCanCancel,
    const FGameplayTag& InputTag, const FGameplayTag& AbilityTag)
{
    if (!InputBuffer || !ASC) return;
    if (!InputBuffer->TryConsume(InputTag)) return;

    if (bCanCancel)
    {
        FGameplayTagContainer CancelTags;
        CancelTags.AddTag(GameplayTags::Ability_Mugong_Light);
        CancelTags.AddTag(GameplayTags::Ability_Mugong_Heavy);
        CancelTags.AddTag(GameplayTags::Ability_Mugong_Dodge);
        ASC->CancelAbilities(&CancelTags);
    }

    FGameplayTagContainer ActivateTags;
    ActivateTags.AddTag(AbilityTag);
    ASC->TryActivateAbilitiesByTag(ActivateTags);
}
```

**Tick의 두 블록 → 한 블록**:
```cpp
if ((!bAttacking && !bDodging) || bCanCancel)
{
    TryConsumeAndActivate(ASC, bCanCancel, GameplayTags::Input_Action_Light, GameplayTags::Ability_Mugong_Light);
    TryConsumeAndActivate(ASC, bCanCancel, GameplayTags::Input_Action_Heavy, GameplayTags::Ability_Mugong_Heavy);
}
```

**Dodge 블록은 별개 유지** — 의미 분리 (회피 단발, Light만 캔슬).

---

## PIE 검증 결과

| 시나리오 | 결과 |
|---|---|
| L 1회 → Hit1 | ✅ |
| L → L → L → L → Pure4 마무리 | ✅ |
| L 5연타 → Pure4 후 5번째 다시 Hit1 (ClearHistory) | ✅ |
| H 1회 → Heavy_Hit1 | ✅ |
| H → H → Heavy_Hit2 (다른 모션) | ✅ |
| L → H → L → H → LHLH_Finisher | ✅ |
| 4타 마무리 직후 1타 → 약한 GE만 (강한 GE 잔류 X) | ✅ |
| RootMotion으로 캐릭터 이동 + 카메라 추적 | ✅ |
| 공격 중 이동 입력 무시 | ✅ |

---

## 변경 파일 (P4 submit 39개)

### 코드
- `Source/Project_KD/AbilitySystem/Combo/ComboComponent.h` (getter 추가)
- `Source/Project_KD/AbilitySystem/Abilities/Player/GA_LightAttack.h/cpp` (배열 + Activate 인덱싱)
- `Source/Project_KD/AbilitySystem/Abilities/Player/GA_HeavyAttack.h/cpp` (동일 패턴)
- `Source/Project_KD/Combat/WeaponComponent.cpp` (GripPoint 자동 정렬)
- `Source/Project_KD/Player/PlayerCharacter.h/cpp` (Tick override, TryConsumeAndActivate 헬퍼)

### 자산
- AM_LightAttack_Hit1/2/3/Hit4
- AM_HeavyAttack_Hit1/2/3
- AM_HeavyAttack_LHLH_Finisher (또는 임시 자산)
- DA_LightCombo
- BP_GA_LightAttack, BP_GA_HeavyAttack
- BP_PlayerCharacter, BP_KDPlayerState
- SKM_killdong (weapon_r 트랜스폼)
- SKM_Woldo (GripPoint 소켓)
- ABP_PlayerCharacter (Slot 'DefaultSlot' 확인)

---

## 회귀 방지 노트

- ABaseCharacter 상속받는 새 캐릭터 추가 시 Tick 필요하면 생성자에 `bCanEverTick = true` 명시
- Tick에 InputBuffer Consume 추가 시 헬퍼 `TryConsumeAndActivate` 재사용 (M2 GA_Charged 등 동일 패턴)
- BP_GA 새로 만들면 활성화 소유 태그에 `State.Combat.Attacking` 잊지 말기
- Light 블록 CancelTags 누락 같은 대칭성 버그 → 헬퍼로 강제 (혼자 만지면 안 됨)

---

## 다음 진입점

1. **5분기 마무리 Montage 폴리싱** — Incursion_I/II, Onslaught_I/II 전용 모션 (LHLH 임시 자산 재활용 중)
2. **또는 M2 진입** — Dodge/Parry (Phase F~G 시작) + hL(차지 약공) 도입 (분기 4종 추가)
3. **DA_LightCombo → DA_ComboTree 이름 정정** (선택)

**Day 4 작업 결정 (2026-05-25 합의)**: Phase F (Dodge) 진입. SB식 Perfect Dodge 메커니즘 구현.
- Dodge = 회피 (대쉬 동일 개념)
- Perfect Window: SB 표준 150ms 시작, 조정
- Stamina: 일반 닷지 소모 / Perfect 닷지 무소모

---

## 페어 노트

- 페어(필규) 통지 — 큰 변경(39 파일) + WeaponComponent/PlayerCharacter 같은 공유 파일 손댐
- 페어 룰 §5-3 "200줄+ CL 트리거" 해당
- Submit 후 페어 채널 ping
