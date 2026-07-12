# GAS i-frame / Parry 윈도우 구현 패턴 조사

**날짜**: 2026-05-22
**조사 트리거**: W1 우선순위 3-A (GA 4종 구현) 진행 중, GE_DodgeInvincible / GE_ParryWindow 를 **Duration GE** 로 만들지 **AnimNotifyState** 로 만들지 결정 필요.
**결론**: **Duration GE + GA 인터럽트 콜백에서 명시적 제거** (Lyra/Tranek 권장). i-frame/Parry = GE, 캔슬 입력 윈도우 = ANS 로 두 차원 분리.

---

## 핵심 결과 요약

| 출처 | 권장 패턴 |
|---|---|
| **Lyra (Epic 공식 GAS 레퍼런스)** | Duration GE + GrantedTags. ANS 기반 태그 부여 사례 없음 |
| **Tranek GASDocumentation (커뮤니티 바이블)** | LooseTag 가능하지만 비복제 + `NotifyEnd` 미보장 → GE 권장 |
| **현업 컨벤션** | Lyra, GASShooter, 대부분 액션 게임 → GE 중심 |

### ANS 단독 방식의 결정적 약점

**Tag Leak 버그**: 몽타주가 피격 등으로 인터럽트될 때 `NotifyEnd` 가 호출되지 않아 `State.Invincible` 태그가 ASC 에 영구히 남음. UE 포럼에서 반복 보고된 알려진 문제. 캔슬이 자유로운 액션 게임 (소울·오공·세키로 류) 일수록 위험.

### 권장 보강 패턴 (필수)

```cpp
// GA_Dodge::ActivateAbility()
ActiveInvincibleGEHandle = ApplyGameplayEffectToOwner(GE_DodgeInvincible_Class, ...);

// MontageTask 의 OnCompleted / OnInterrupted / OnCancelled / OnBlendOut 4콜백 모두에서:
if (ActiveInvincibleGEHandle.IsValid())
    AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveInvincibleGEHandle);
```

이중 안전망: Duration 자동 만료 + GA 종료 시 명시적 제거 → Leak 0%.

---

## 두 차원 분리 (GE × ANS)

| 차원 | 메커니즘 | 책임 |
|---|---|---|
| **속성·상태 윈도우** | Duration GE | 무적/패링/자원 효과 (GA 라이프사이클 종속) |
| **캔슬 입력 윈도우** | AnimNotifyState | 몽타주 안에서 다음 입력을 받을 시점 (이미 `ANS_CancelWindow` 존재) |

오공·세키로 류 "공격 도중 회피 캔슬, 회피 도중 공격 캔슬, 연속 회피로 무적 연장" 시나리오 전부 두 차원 분리로 자연스럽게 표현됨.

---

## Duration GE vs AnimNotifyState 트레이드오프

| 항목 | Duration GE | AnimNotifyState |
|---|---|---|
| 몽타주 인터럽트 시 태그 정리 | 안전 — `EndAbility`/`OnInterrupted`에서 명시적 `RemoveActiveGE` 가능 | 위험 — `NotifyEnd` 미보장, 영구 태그 leak 가능 |
| 복제(Replication) 지원 | GE 복제 내장 (멀티 자동) | LooseTag 비복제, 수동 복제 필요 |
| 애니메이션 타이밍 정밀도 | GE Duration 조정 또는 몽타주 Event 트리거 | 타임라인 직관적, 애니메이터 직접 제어 |
| 구현 복잡도 | 중간 (GE CDO 셋업 + EndAbility 후처리) | 낮음 (ANS 클래스 + 2개 콜백) |
| UE5.3+ 호환성 | `Target Tags Gameplay Effect Component` 사용 필요 (UE5.6 룰) | 변경 없음 |
| 현업 컨벤션 | Lyra, GASShooter, Tranek 문서 모두 GE 중심 | 빠른 프로토타입에 한정 |

---

## Parry 데미지 인터셉트 — 단순화 결정

조사 결과 두 옵션:
- **A.** `GE_Damage_Physical.cpp` 생성자에 `ReqComp->IgnoreTags.AddTag(State_Combat_Parrying)` 1줄 추가 (Invulnerable과 완전 동일 패턴)
- **B.** `UGameplayEffectCustomApplicationRequirement` 서브클래스 신규 생성 → `CanApplyGameplayEffect()` 오버라이드

CLAUDE.md §3 YAGNI + 사변적 클래스 금지 → **A 채택**. 1회용에 클래스는 과잉.

---

## UE5.6 호환성 주의

- UE5.3 이전: `UGameplayEffect::GrantedTags` 직접 사용 가능
- **UE5.3+/UE5.6**: `GrantedTags` deprecated → `UTargetTagsGameplayEffectComponent` 사용 필수
- GE CDO 에서 `AddComponent<...>()` 직접 호출 금지 (CLAUDE.md §2 #7) → **`CreateDefaultSubobject` + `GEComponents.Add` 패턴**
- 기존 코드 참조: `Source/Project_KD/AbilitySystem/Effects/GE_Damage_Physical.cpp:28-31`

---

## 외부 참고 자료 (링크 모음)

### Lyra / Epic 공식
- [Lyra Health and Damage — X157 Dev Notes](https://x157.github.io/UE5/LyraStarterGame/Health-and-Damage/)
- [Abilities in Lyra — Epic 공식 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/abilities-in-lyra-in-unreal-engine)
- [Gameplay Effects for the GAS — Epic 공식 문서](https://dev.epicgames.com/documentation/en-us/unreal-engine/gameplay-effects-for-the-gameplay-ability-system-in-unreal-engine)

### 커뮤니티 바이블 (Tranek)
- [tranek/GASDocumentation — GitHub](https://github.com/tranek/GASDocumentation)
- [Tranek GAS Issue #136 — LooseTag vs GE](https://github.com/tranek/GASDocumentation/issues/136)

### UE Forum 논의
- [GAS Gameplay Effect doesn't remove its tag after duration — UE Forum](https://forums.unrealengine.com/t/gas-gameplay-effect-doesnt-remove-its-tag-after-has-duration/141098)
- [How can I implement a Block and Parry system? — UE Forum](https://forums.unrealengine.com/t/how-can-i-implement-a-block-and-parry-system/1908594)
- [GAS and Animation Notifications/Notifies — UE Forum](https://forums.unrealengine.com/t/gameplay-ability-system-gas-and-animation-notifications-notifies/540119)

### UE5.6 GE Component 가이드
- [How to use Gameplay Effect Components in UE5 — Quod Soler](https://www.quodsoler.com/blog/how-to-use-gameplay-effect-components-in-unreal-engine-5)
- [Making Sense of Gameplay Effect Durations — Quod Soler](https://www.quodsoler.com/blog/making-sense-of-gameplay-effect-durations)

---

## 다음 액션 (W1 P3-A 구현 순서 반영)

1. ✅ 태그 2개 추가 (`State_Combat_Parrying`, `SetByCaller_Stamina`) — 완료
2. **GE 3종 구현** — `GE_StaminaCost` / `GE_DodgeInvincible` / `GE_ParryWindow` (UE5.6 `UTargetTagsGameplayEffectComponent` 패턴)
3. GA_LightAttack
4. GA_HeavyAttack
5. GA_Dodge (**4콜백 명시적 GE 제거 패턴 첫 적용**)
6. GA_Parry (WaitGameplayEvent 패턴)
7. `Event_Parry_Success` 태그 추가 + `GE_Damage_Physical.cpp` Parrying reject + Event 발송 2줄
8. (승환) BP/StartupAbilities/Montage 임시 할당
9. PIE 검증 + dev-log 작성
