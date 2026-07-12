# Perfect Parry 데미지 분기 + GameplayCue 마이그레이션 (2026-05-26)

**상태**: ✅ M1 Phase H (Perfect Parry 데미지 분기 + 시각/슬로우) + Phase I (모든 액션 GA Super::ActivateAbility) 완료. SlowMo는 GameplayCue로 최종 아키텍처.

## 작업 요약

- Phase H: AS_Combat 데미지 분기 + Event broadcast (직접 캐스팅 X, 디커플링)
- Phase I: 3개 액션 GA (`GA_WeaponTraceBase`, `GA_Parry`, `GA_Dodge`)에 `Super::ActivateAbility` 호출 추가
- BP_GA_Parry 이벤트 그래프 작성 (Wait Gameplay Event + Play Montage and Wait + SlowMo)
- AM_Parry_R Montage 생성 (베이스 `AS_Parry_R_Seq`)
- SlowMo 잠금 버그 → **GameplayCue 마이그레이션**으로 해결

---

## Phase H — AS_Combat 데미지 분기

### 결정
- 데미지 처리 위치: `AS_Combat::PostGameplayEffectExecute` (IncomingDamage 게이트웨이)
- 디커플링 원칙: AS_Combat → PlayerCharacter 직접 캐스팅 ❌ → **GameplayEvent broadcast** ✅
- 새 태그: `Event.Combat.PerfectParryTriggered`

### 변경
`AS_Combat.cpp::PostGameplayEffectExecute`:
```cpp
// Perfect Parry — 데미지 0 + GameplayEvent 발행 (listener BP가 처리).
if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_PerfectParryReady))
{
    FGameplayEventData EventData;
    EventData.EventTag = GameplayTags::Event_Combat_PerfectParryTriggered;
    EventData.Instigator = Data.EffectSpec.GetContext().GetInstigator(); // 공격자 보존 (M2 카운터 대비)
    EventData.Target = ASC->GetAvatarActor();
    ASC->HandleGameplayEvent(EventData.EventTag, &EventData);
    return;
}

// 일반 Parry — 50% 감소.
float FinalDamage = LocalDamage;
if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying))
{
    FinalDamage *= 0.5f;
}
// 이후 Defense 경감 + Health 차감 (FinalDamage)
```

### 핵심
- **B-3 Instigator 보정**: 처음엔 `GetAvatarActor()`로 설정해서 broadcast에 "방어자=공격자"로 박혔음. 공격자 actor 정보 손실 → M2 카운터 작업 시 다시 추적 필요. `Data.EffectSpec.GetContext().GetInstigator()`로 원본 공격자 보존.

---

## Phase I — 모든 액션 GA에 Super::ActivateAbility 호출

### 의도
모든 액션 GA의 `ActivateAbility` 끝에 `Super::ActivateAbility(...)` 호출 추가 → BP `Event ActivateAbility` 발동 → BP 그래프에서 VFX/Sound/Camera 효과 자유 추가 가능. 페어 룰 §1-1 정합 (GA가 UI/Sound/Camera 직접 호출 금지 → BP 그래프 활용 OK).

### 변경
- `GA_WeaponTraceBase.cpp:66` — Super 호출 + 주석
- `GA_Parry.cpp:65` — Super 호출 + 주석
- `GA_Dodge.cpp:90` — Super 호출 + 주석

각 1줄 추가. 자식 BP (BP_GA_Light/Heavy/EnemyBasic/Parry/Dodge) 모두 자동 정합.

---

## Perfect Parry 시각 — BP_GA_Parry 이벤트 그래프

### 최종 구조
```
[Event ActivateAbility]
        ↓
[Wait Gameplay Event]   ← To Actor 변형 X (같은 ASC 청취 — GAS 정석)
  - Event Tag: Event.Combat.PerfectParryTriggered
  - Only Trigger Once: ☐ / Only Match Exact: ☑
        ↓ (Event Received)
[Sequence]
  ├─ Then 0 → [Play Montage and Wait]
  │              - Montage: AM_Parry_R
  │              - Stop When Ability Ends: ☐ (false) ★
  │              On Completed/BlendOut/Interrupted/Cancelled (4핀) → [End Ability]
  │
  └─ Then 1 → [Execute Gameplay Cue On Owner]
                 - Tag: GameplayCue.Combat.PerfectParry.SlowMo
              → [End Ability]
```

### 핵심 결정 이유
- **`Wait Gameplay Event`** (To Actor X) — 같은 ASC 청취. To Actor는 다른 actor 이벤트 들을 때만.
- **`Play Montage and Wait`** with `Stop When Ability Ends = false` — Perfect 발동 직후 E 떼도 AM_Parry 끊김 방지 (Anim Instance에 위탁).
- **PlayMontageAndWait 4핀 모두 EndAbility** — Epic ARPG/Lyra 정석 패턴. 어떤 종료 경로(완료/방해/캔슬/블렌드아웃)도 GA 정상 종료 보장.
- **`Execute Gameplay Cue On Owner`** (다음 섹션 상세) — SlowMo 잠금 버그 해결.

---

## SlowMo 잠금 버그 + GameplayCue 마이그레이션

### 이전 버그 (간헐 발생)
초기 구현: BP의 `Set Global Time Dilation(0.3) → Delay(0.15) → Set Global Time Dilation(1.0)` 체인.

**문제**: 사용자가 SlowMo 0.5초 안에 E를 떼면 → `TryParryStop` → `CancelAbilities` → GA 종료 → **BP latent node(Delay) 강제 종료** → `SetGlobalTimeDilation(1.0)` 영원히 안 불림 → 시간 0.3 영구 잠김.

### 1차 시도 — PlayerCharacter 헬퍼 (반려)
`StartSlowMotion(Scale, RealDurationSec)` 함수 PlayerCharacter에 추가. World Timer 사용으로 GA 캔슬 무관 → 작동은 함.

**반려 사유**:
- 페어 룰 §1-2 "비주얼/오디오 효과 → GameplayCue" 가이드 비껴감
- PlayerCharacter 비대화 (이미 Sprint + 콤보 헬퍼 + 어빌리티 트리거 + 컴포넌트 호스트 책임 다중)
- §1-1 "Pawn 500줄 초과 금지" 압박

### 최종 — GameplayCue 마이그레이션 ✅
- 새 태그: `GameplayCue.Combat.PerfectParry.SlowMo` (KDGameplayTags.h:64 / cpp:44)
- 새 BP: `BP_GC_SlowMoPerfectParry` (parent `GameplayCueNotify_Actor`)
  - **OnExecute 함수**: `Set Global Time Dilation(0.3)` → `Set Timer by Function Name("ResetTimeDilation", 0.15)`
  - **ResetTimeDilation 함수**: `Set Global Time Dilation(1.0)` → `Destroy Actor`
- BP_GA_Parry Then 1: `Execute Gameplay Cue On Owner(GameplayCue.Combat.PerfectParry.SlowMo)` → `End Ability`

### 왜 작동하나
GA → ASC 통해 Cue broadcast → ASC의 Cue Manager가 **별개 액터(BP_GC_SlowMoPerfectParry) World에 spawn** → spawn된 액터의 OnExecute 호출 → Timer는 그 액터 소유.

**핵심**: Cue Actor는 GA와 라이프사이클 완전 분리. GA 캔슬돼도 Cue Actor는 World에 살아남아 Timer 끝까지 진행 → SlowMo 100% 복귀 보장.

### GAS 라이프사이클 표준 (학습)
| 컴포넌트 | 라이프사이클 | 용도 |
|---|---|---|
| GA | 매우 짧음 (어빌리티 발동~종료) | 행동 로직 |
| GE | 짧음~중간 (Duration 또는 Instant) | 속성 변경, 태그 |
| **Cue** | 단발 (Cue Actor 살아있는 동안) | **시각/오디오 효과** |

→ "잠깐 발동 후 자체 정리되는 시각 효과" = Cue가 정확히 그 용도.

---

## PIE 검증 결과

| 시나리오 | 입력 | 결과 |
|---|---|---|
| 가드 X | 적이 때리도록 둠 | 정상 데미지 ✅ |
| 일반 Parry | E Hold + 0.15s 후 hit | 데미지 × 0.5 ✅ |
| Perfect Parry | E 누른 직후 hit | 데미지 0 + AM_Parry + 슬로우 ✅ |
| 슬로우 복귀 (Hold) | Perfect 후 E 계속 누름 | 0.5s 후 1.0 복귀 ✅ |
| **슬로우 복귀 (즉시 뗌)** | Perfect 발동 직후 E 뗌 | 0.5s 후 1.0 복귀 ✅ ← 이전 버그 해결 |
| **슬로우 복귀 (중간 뗌)** | Perfect 후 0.2s만에 E 뗌 | 0.5s 후 1.0 복귀 ✅ |
| 연속 트리거 | 짧은 시간 2회 Perfect Parry | 마지막 Cue Actor가 1.0 복귀 보장 ✅ |

---

## 변경 파일

### 코드
- `Source/Project_KD/AbilitySystem/Attributes/AS_Combat.cpp` — Perfect/일반 Parry 분기 + Instigator 보정
- `Source/Project_KD/AbilitySystem/Abilities/GA_WeaponTraceBase.cpp` — Super 호출
- `Source/Project_KD/AbilitySystem/Abilities/Player/GA_Parry.cpp` — Super 호출
- `Source/Project_KD/AbilitySystem/Abilities/Player/GA_Dodge.cpp` — Super 호출
- `Source/Project_KD/KDGameplayTags.h/cpp` — `Event.Combat.PerfectParryTriggered` + `GameplayCue.Combat.PerfectParry.SlowMo` 추가
- `Source/Project_KD/Player/PlayerCharacter.h/cpp` — 임시 BIE 제거 + 임시 StartSlowMotion 추가했다가 롤백 (최종 변경 없음, Phase I C++ 호환만)

### 자산
- `AM_Parry_R` — Perfect Parry 시각 모션 (베이스 `AS_Parry_R_Seq`)
- `BP_GC_SlowMoPerfectParry` — GameplayCueNotify_Actor 자식
- `BP_GA_Parry` — 이벤트 그래프 (Wait Gameplay Event + Play Montage and Wait + Execute Gameplay Cue)
- `GE_PerfectParryWindow` — 기존 자산 활용 (Duration 0.15s, State.Combat.PerfectParryReady 부여)

---

## 회귀 방지 노트

- **GameplayCue 패턴**: 향후 시각/오디오 효과 추가 시 동일 패턴 (Cue 태그 → BP_GC_xxx → OnExecute) 재사용. PlayerCharacter나 GA에 직접 박지 말 것.
- **PlayMontageAndWait 4핀**: 새 액션 GA 만들면 OnCompleted/BlendOut/Interrupted/Cancelled 4핀 모두 EndAbility로 묶기 (Lyra/ARPG 정석).
- **GameplayEvent broadcast**: AS_Combat → PlayerCharacter 직접 캐스팅 패턴 다시 쓰지 말 것. EventData.Instigator는 항상 `Data.EffectSpec.GetContext().GetInstigator()`로 공격자 보존.
- **Cue Actor `Set Timer by Function Name`**: 함수 이름 문자열 오타 주의 (대소문자 일치 필수). 오타 있으면 Timer 등록은 되지만 발동 시 함수 못 찾아 시간 영구 잠김 → 디버깅 막막.

---

## 다음 진입점

1. **B-1 클린업** (선택) — `GA_Parry.h:38-39` `PerfectParryWindowSec` 미사용 변수 삭제 (페어 룰 §3 YAGNI). 1줄.
2. **M1 통합 검증 게이트** — 콤보(L/H/LHLH) + Parry(일반/Perfect) + Dodge(일반/Perfect) 통합 PIE. 한 시스템 추가가 다른 시스템 깨뜨리지 않았는지 회귀 확인.
3. **M2 진입 의존성**:
   - 적 AI / 적 GA (페어 영역) — 적 GA에 `State.Combat.EnemyAttackHitWindow` 태그 부여하는 ANS 추가하면 Perfect Dodge 자연 작동
   - Beta Energy 어트리뷰트 (AS_Player) + Perfect Parry 회복
   - 적 Posture 시스템 + Perfect Parry 자세 깎기
   - 카운터 공격 GA + IA_Counter 입력 (Custom Time Dilation 패턴 = 플레이어만 정상 속도, 적은 슬로우)

---

## 페어 노트

- 페어(필규) 통지: AS_Combat 게이트웨이 + `Event.Combat.PerfectParryTriggered` 태그 + `GameplayCue.Combat.PerfectParry.SlowMo` 추가
- 페어 룰 §5-3 "200줄+ CL" 해당 (오늘 6개 코드 파일 + BP/태그 변경)
- Parry/Dodge 도메인 = 작성자(승환) 단독 결정 OK (페어 룰 §5-1)
- M2 진입 결정은 페어 합의 필요
