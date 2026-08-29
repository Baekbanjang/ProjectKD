# 2026-08-30 — 가드 로코모션 + 붕괴 (4단계 ②·③)

## 진행

08-28 패링 재설계의 후속. **가드 중 이동 불가 → 가드 로코 PSD 로 전환 + 실드 0에서 붕괴 신호 + 회복 지연**.

계획 전문 = `docs/handoffs/CURRENT.md` §2026-08-28 ②③

## 1단계 — 가드 로코모션 (몽타주 → 모션매칭)

### 종전 구조 (폐기)

```
가드 진입      AM_SB_Block_Start      0.417s
가드 유지      AM_SB_Block_Loop       10.0s(무한 재생) · DefaultSlot 전신
               └ 루트모션 켜짐 = 로코모션과 충돌
가드 타격 반응  AM_SB_Block_Hit        0.833s
```

루프 재생이 지속 자세를 맡고 있어 **가드 중 이동이 불가능**했다.

### 신규 구조 (완성)

```
가드 진입      AM_SB_Block_Start      0.417s    유지 (일회성 동작)
가드 유지      PSD 3종으로 교체       —          대기 중 자세 선택
  ├ PSD_SB_Idles_Block         1클립   정지 상태
  ├ PSD_SB_Loco_Block         16클립   이동 중
  └ PSD_SB_Stops_Block        16클립   감속/정지 전환
가드 타격 반응  AM_SB_Block_Hit        0.833s    유지
```

**PSD가 가드 자세를 선택.** `CHT_LocomotionDatabase` 에 `bIsBlocking` 컬럼 추가 — 위 3행 참조.

배선 = `KDPlayerAnimInstance::bIsBlocking` (라인 65) 이 `State_Combat_Parrying` 태그를 매 프레임 읽는다.

★ **ABP 배선 불필요.** Chooser의 입력 핀은 `KDPlayerAnimInstance` 하나뿐이고,
CHT가 그 안의 프로퍼티를 이름으로 직접 읽는다. "핀에 연결한다"는 초기 계획이 오해였다.

## 2단계 — 가드 이동속도

### 증상 & 원인

**가드 중 이동이 3배로 미끄러진다.**

`RefreshMaxWalkSpeed()` (라인 255~283) 에 락온·조준 조건만 있고 가드가 없어
기본값 `BaseWalkSpeed = 500` 이 그대로 나갔다. 루프 몽타주가 덮고 있던 동안엔 안 보였을 뿐.

### 해결

```cpp
if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Parrying))
{
    Speed = FMath::Min(Speed, BlockMoveSpeed);
}
```

신규 멤버: `BlockMoveSpeed = 167.f`

**클립 실측:** `AS_Walk_Block_Loop` 16개 포즈 = **167 cm/s**

```
PSD_SB_Loco_Block  평상시 로코모션 167 cm/s 기준점  (Start 계열: 96~108)
                   165 속도 안에 없는 값은 보간 대상이 되지 못함
```

★ **모션매칭은 속도를 조정하지 않고 고른다.** ABP의 `root_motion_mode = ROOT_MOTION_FROM_MONTAGES_ONLY` —
로코모션 경로는 루트모션을 아예 뽑지 않고, CMC가 속도를 정하면 PSD가 거기 맞는 클립을 고를 뿐이다.
DB에 없는 속도는 만들어낼 수 없다.

대조: **락온 280은 자연스러운 이유** = PSD에 167대 18개 + 422대 18개가 있어 그 범위를 보간.
가드 DB는 상한이 167이라 500은 범위 밖 → 미끄러짐.

곁다리: 지난번 `Walk_Block` 32개에 루트모션을 켠 작업(몽타주) = 로코모션 용도로는 무효
(`MONTAGES_ONLY` 때문). 몽타주 용도로서는 해가 없다.

## 3단계 — 상호 배타 (조준 · 가드 · 락온)

### C++ 태그 차단

가드와 조준이 동시에 활성화되는 걸 방지.

```
GA_Parry          ActivationBlockedTags += State.Combat.Aiming
GA_AimMode        ActivationBlockedTags += State.Combat.Parrying
```

**BP Class Defaults에 설정.** 프로젝트가 이미 그 방식이고(`KDGameplayTags.h:74` 주석),
빌드 없이 조정 가능.

### ToggleLockOn 조건 (조준 중 진입만 차단)

```cpp
if (!LockOnComponent->IsLockedOn())
{
    const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
    if (ASC && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_Aiming))
        return;  // 조준 중이면 락온 진입 X
}
LockOnComponent->ToggleLockOn();  // 락온 중 조준 → 락온 해제는 허용
```

📌 **목표:** 조준 중 락온만 막기. 락온 후 조준한 경우 갇힘 방지.

## 4단계 ② — 가드 붕괴

### 감지 로직

```cpp
const float ShieldBefore = ASC->GetNumericAttribute(...Shield);
const float ToHealth = ApplyMitigation(ASC, LocalDamage, bBlockedHit);
const float ShieldAfter = ASC->GetNumericAttribute(...Shield);

const bool bShieldDepleted = ShieldBefore > 0.f && ShieldAfter <= 0.f;
const bool bGuardBroken    = bBlockedHit && bShieldDepleted;
```

⚠️ **`ShieldBefore > 0` 이 핵심.** 없으면 실드 0 상태로 맞을 때마다 붕괴가 재발동.

### 이벤트 체인

```
SendGuardBreak()
  └ Event.Combat.GuardBreak
     └ GA_GuardBreak (신규 BP)
        ├ ActivationOwnedTags = State.Combat.GuardBroken
        ├ CancelAbilitiesWithTag = Ability.Player.Parry
        ├ Wait Delay 0.6s
        ├ Play AM_SB_Block_Hit_Break (1.0s, bStopWhenAbilityEnds=false)
        └ EndAbility
```

**왜 별도 GA인가**

- `GA_Parry` 안에서 못 한다. 버튼 해제 시 GA 캔슬 + `bStopWhenAbilityEnds` 로 몽타주가 잘린다
- 구체 패링 GA를 확장해도 작동할 수 있지만, 붕괴만의 분리된 타이밍(0.6s 무방비)이 필요하다

### 5개 GA 차단 (붕괴 중)

```
ActivationBlockedTags += State.Combat.GuardBroken
```

GA_Parry, GA_LightCombo, GA_HeavyCombo, GA_Dodge, GA_HitReact

📌 **무방비 0.6초 / 몽타주 1.0초 분리 = B 방식(SB 근거)** —
애니 길이와 입력 차단 시간이 다르다. 0.6초 뒤 조작이 풀리고 몽타주는 관성처럼 남았다가
이동 입력에 밀려난다.

⚠️ **AM_SB_Parry_Counter_Attack_R 노티 0개 — 이 몽타주만 판정 없음.** 미해결.

## 4단계 ③ — 실드 회복 지연

### 신호 → 차단

```
SendShieldDepleted()
  └ Event.Combat.ShieldDepleted
     └ GA_ShieldDepleted (신규 BP)
        └ LooseGameplayTag State.Shield.RegenBlocked 4.0s
```

### 리젠 GE에 적용

```
GE_ShieldRegen           Ongoing Tag Requirements → Ignore Tags = State.Shield.RegenBlocked
GE_ShieldRegen_InCombat  Ongoing Tag Requirements → Ignore Tags = State.Shield.RegenBlocked
```

**선례:** `State.Stamina.RegenBlocked` · `State.Gun.RegenBlocked` 패턴과 동일.

## SB 조사 결과 (덤프 실측, 재조사 방지용)

덤프 루트 `D:/stellarblade_Fakuri/Exports/SB/Content/`

| 항목 | 실측값 | 출처 |
|------|--------|------|
| **적 실드 파괴 LifeTime** | 1.0 고정 | ShieldZero 그룹 9행 전부 |
| **적 실드 파괴 태그** | ActorState_BlockingBehavior + Action_SkillCancel | EffectTable.json |
| **경직 위계** | HitStun 0.3 < ShieldBreak 1.0 < Groggy 5.0 | Local/Data/ |
| **강공격 가드피격 입력차단** | 0.6s (일반 0.3s) | Result_Guard_Back_Strong |
| **강공격 가드피격 애니** | P_Eve_Guard_Hit_Strong_* 1.4667s (입력과 길이 불일치) | Local/Data/ |
| **이브 MaxShield** | 500 | CharacterTable.json |
| **이브 ShieldBlock** | 2.0(단계) = 17.5% 감쇄 | CharacterTable.json |
| **이브 가드 중 실드 데미지** | 1.5배(−50%) | GuardShieldDamage_Step |
| **이브 실드 0 → 회복** | 4초 차단 후 20% 즉시(500→100) | Local/Data/ |

### ★ 플레이어 가드 붕괴 정의 실패

- ReactionWhenShieldZero 9행 전부 = 플레이어 제외
- TargetChainEffectAliasWhenZeroShieldArray = 이브 → 몬스터만
- 고아 에셋 Result_Guard_Break.json 존재 (2.8s, 이브 애니, 참조 0)
- SkillActiveStepTable NextStepAliasWhenBreakGuard = 플레이스홀더만

**결론:** 로직이 C++/BP로 옮겨간 상태. 덤프로는 더 못 찾는다.

## 검증

```
빌드      통과 (승환)
PIE       ✓ 가드 홀드 → 이동 가능
         ✓ 정지 상태에서 자세 유지
         ✓ 붕괴 후 0.6초 무방비
         ✓ 실드 4초 회복 차단
태그      조준 중 락온 진입 차단 ✓
          락온 해제는 허용 ✓
```

## 남은 것

```
실드 값 조정      총량 50 · 흡수율 0.8/0.4 · 회복 3.0/0.4
                ⚠️ 전투 중 0.4/s는 125초가 필요해 실질 0
                   SB의 "차단 후 20% 즉시 회복" 도입 필요성 미정

3단계 연출       반격기에 몰아주기 · CustomTimeDilation 1.5 · 카메라 랙 동적 제어
                패링 연출(GCN_PerfectParry · LCS_PerfectParry · M_PP_ScreenEffect)
                과 겹치는 구조 실측이 먼저

AM_SB_Parry_Counter_Attack_R    노티 0개 = 판정 없음

패링/회피 반격  태그 분리 보류 — 반격 애니 2개뿐이라 지금 나누면 회피 손해

Content/Robot3/     135MB 커밋할지 삭제할지 미정
```

커밋 = 코드 `b28f858` · `8857635` · `becf5b4` / 에셋 (로컬만)
