# 콤보 시스템 구현 Day 2 — 진행 로그

**날짜**: 2026-05-24
**작업**: W1 우선순위 3-A 단계 3d / 3e (GA_LightAttack / GA_HeavyAttack)
**상태**: 빌드 통과. 작성자 영역(BP 셋업 + DA_ComboTree) 진입 대기.

---

## 오늘 완료 작업

### 단계 3d / 3e — GA_LightAttack / GA_HeavyAttack 클래스 추가

- 경로: `Source/Project_KD/AbilitySystem/Abilities/GA_LightAttack.h/.cpp` + `GA_HeavyAttack.h/.cpp`
- 부모: `UGA_WeaponTraceBase`
- 패턴: `ActivateAbility` 오버라이드 → ComboComponent::ProcessInput 호출 → 정확 매칭 시 분기 Montage/GE 교체, 그 외 디폴트 단발/GE 복원 → `Super::ActivateAbility`
- 입력 태그: Light=`Input_Combo_Light`, Heavy=`Input_Combo_Heavy`
- **신규 UPROPERTY 2개** (양 GA 동일):
  - `DefaultAttackMontage` (1~3타 단발용)
  - `DefaultDamageEffectClass` (1~3타 데미지 GE용)

### 빌드
- UE5.6 빌드 통과 (작성자 확인 2026-05-24).

---

## 핵심 결정 3건 (작성자 confirm)

### 1) 별도 클래스 2개 (vs 공통 베이스 통합)
- `UGA_LightAttack` / `UGA_HeavyAttack` 분리. ProcessInput 태그 1줄만 차이.
- 사유: dev-log Day 1 명세 일관, M1 스코프엔 L/H만이라 통합 추상화는 YAGNI(§3). M2 hL 도입 시 통합 재검토.

### 2) `DefaultDamageEffectClass` UPROPERTY 신규 추가 ★
- 매 활성화 시작에 `AttackMontage`와 함께 디폴트로 복원.
- 사유: `DamageEffectClass`도 GA의 InstancedPerActor 인스턴스 멤버라 분기 매칭 후 다음 활성화에 잔류 → 1타째에 강한 GE 적용되는 버그 차단.
- 처음엔 옵션 A(부모 디폴트 그대로 두기)로 권장했다가, GE도 Montage와 같은 InstancedPerActor 대상이라는 점 짚어 옵션 B로 변경.

### 3) 단계 3g 폐기
- 원래 dev-log Day 1 계획: KDPlayerController에서 ComboComponent에 입력 알림 1줄 추가.
- 폐기 사유: ProcessInput을 GA::ActivateAbility 단일 진입점에서만 호출(중복 호출/활성화 실패 시 시퀀스 어긋남 방지). PlayerController 추가 1줄 불필요.

---

## 도메인 메모 — 향후 GA 추가/콤보 설계 시 필요

### A. InstancedPerActor 잔류 (모든 GA)
- GA 객체는 ASC당 1개 인스턴스만 생성되고 활성화는 그 인스턴스를 켜고 끄는 것.
- 멤버 변수는 활성화 사이에 살아남음 → **변경 가능한 멤버는 매 ActivateAbility 시작에 디폴트로 명시 리셋 필요**.
- 비유: 캐릭터가 들고 다니는 "공격 매뉴얼 책 1권". 활성화 = 책 펴기. 책 안 포스트잇(=멤버 변수)은 책 덮어도 그대로 있음.
- 우리 GA 패턴: `AttackMontage = DefaultAttackMontage; DamageEffectClass = DefaultDamageEffectClass;` 매번 시작에 호출.

### B. Prefix 겹침 함정 (콤보 분기 설계)
- `UComboComponent::ProcessInput`은 정확 매칭 검사를 prefix 매칭보다 먼저 실행하고, 매칭 시 즉시 ClearHistory.
- 따라서 짧은 분기와 그 prefix로 시작하는 긴 분기를 같이 등록하면 짧은 게 먼저 발동, 긴 분기는 영원히 못 닿음.
- 예: `[L, L]` 등록 시 `[L, L, *, *]` 시리즈 전부 죽음.
- 설계 가이드(SB식): 짧은 분기와 prefix 겹치는 긴 분기를 같이 두지 않는다. 길이 무관, 각 분기는 첫 분기점에서 갈라지는 입력을 가져야 함.

---

## 🌅 다음 진입점 — 작성자 영역 (BP 셋업)

### 단계 3h — DA_ComboTree 5분기 셋업
| 분기 | InputSequence | Montage 슬롯 | DamageEffect 슬롯 |
|---|---|---|---|
| Incursion I  | `[L, H, H, H]` | (임시) | (임시) |
| Incursion II | `[L, L, H, H]` | (임시) | (임시) |
| Onslaught I  | `[H, H, H]`    | (임시) | (임시) |
| Onslaught II | `[H, L, L, H]` | (임시) | (임시) |
| 순수 4타    | `[L, L, L, L]` | (임시) | (임시) |

Prefix 겹침 검증 OK — 첫 분기점이 L/H로 깨끗하게 갈라짐.

### 단계 4 — BP 마무리
- `BP_GA_LightAttack` (Parent: `UGA_LightAttack`) 생성. AbilityTags = `Ability.Mugong.Light`. 슬롯 셋업:
  - `DefaultAttackMontage` ← 1타 단발 휘두르기
  - `DefaultDamageEffectClass` ← 약한 데미지 GE
- `BP_GA_HeavyAttack` 동일, AbilityTags = `Ability.Mugong.Heavy`.
- `PlayerCharacter` BP의 `ComboComponent`에 `DA_ComboTree` 할당.
- `KDPlayerState` StartupAbilities에 두 GA 등록.

### 단계 5 — PIE 검증
- L 1회 → 디폴트 단발 + 1.5s 대기 후 InputHistory 자동 리셋
- L,L,L,L → 4타째 "순수 4타" 마무리 모션
- L,H,H,H → Incursion I 마무리
- L,L,H,H → Incursion II 마무리
- H,H,H → Onslaught I 마무리
- 임의 시퀀스(H,L,H) → 마지막 입력 H만 살리고 새 prefix
