# 캐릭터 스킬 GA — 단발 3종 + 차지형 1종 (2026-09-02)

09-01에 만든 스킬 몽타주 위에 GA를 얹었다. 스킬 1·2·4는 단발, 스킬 3은 **누른 시간만큼 길이가 달라지는 차지형**이다.

---

## 1. 신규 클래스 2쌍

```
UKDGameplayAbility_Skill        : UKDGameplayAbility_PlayerMelee    스킬 1·2·4
UKDGameplayAbility_SkillCharge  : UKDGameplayAbility_Skill          스킬 3
```

부모(`PlayerMelee`)가 몽타주·데미지GE·소켓·트레이스·히트스톱·슬로모·자동조준·접근워프를 전부 `EditDefaultsOnly`로 노출하고 있어, **스킬별 값은 BP 에셋 4개가 그대로 데이터**가 된다. DataAsset을 끼우면 단계만 늘어난다(`CLAUDE.md §3` 승격 기준 미달).

신규 태그 6개 — `Ability.Player.Skill1~4` · `Cooldown.Player.Skill` · `Event.Skill.HoldRelease`.

⚠️ `Ability.Skill.*` 계층을 새로 파지 않고 기존 `Ability.Player.*` 아래에 붙였다. 적 포이즈·처형이 `Ability.Player.*`로 분기하기 때문이다(`DA_*.PoiseDamageByAttack` 맵 키). 다만 **그 맵에 Skill1~4 항목은 아직 없다** — 조회 실패 = 포이즈 0이라 스킬로는 경직이 안 걸린다. `CounterSlash`도 같은 상태다.

---

## 2. ★ `SetByCaller` GE 는 Cost 로 못 쓴다

09-01에 발견한 함정을 이번에 실제로 우회했다.

```cpp
// GameplayAbility.cpp:1094   CheckCost 가 CDO 를 그대로 CanApplyAttributeModifiers 에 넘김
// GameplayEffect.cpp:5181    거기서 새 Spec 생성 -> SetByCaller 주입 지점이 없다
```

`UKDGameplayEffect_StaminaCost`는 `FSetByCallerFloat`이라 **매그니튜드가 0으로 계산된다.** 그대로 `CostGameplayEffectClass`에 걸면 `CheckCost`가 항상 통과하고 `ApplyCost`가 0을 소모한다.

해법 = 두 함수를 오버라이드한다.

```
CheckCost   Super 를 안 부르고 스태미나를 직접 비교
            (Super 를 부르면 SetByCaller 경고 로그가 매번 뜬다)
ApplyCost   MakeOutgoingGameplayEffectSpec -> SetSetByCallerMagnitude -> ApplyToOwner
```

`CostGameplayEffectClass` 자체는 생성자에서 그대로 등록한다 — `ApplyCost`가 그 클래스로 Spec을 만든다.

📌 정상 동작하는 비교군 = `UKDGameplayEffect_AmmoCost`(`FScalableFloat(-1.0f)` 고정값).

---

## 3. 차지 구조 — 몽타주 1개 + 섹션 5개

### 왜 몽타주를 안 나눴나

`Speed_Attack`은 팩이 `Start`/`Loop`/`End` 3클립으로 잘라서 준다. 09-01에 몽타주도 3개로 만들었는데, 이번에 **하나로 합쳤다.**

```
AM_SB_Skill_03   Start f0 / Loop f10 / Charge1 f24.35 / Charge2 f35 / End f60   (총 2.25s)
링크              선형 그대로. Loop→Loop 자기 링크는 안 검
```

몽타주를 갈아타지 않는 이유는 둘이다.

```
① 부모가 몽타주 1개 전제      OnMontageCompleted 가 private + non-virtual -> 자식이 못 가로챈다
                             이어 돌리려면 부모 헤더를 고쳐야 하는데 콤보 GA 20개가 같은 부모
② 이음새에 블렌드가 낀다      초당 600cm 로 전진 중이라 갈아탈 때마다 튄다
```

섹션 방식은 **부모를 0줄 건드린다.**

### 차지 단계 = 어디서 End 로 빠질지

```
차지 1단계  SectionName=Loop      Start → Loop ─────────────────→ End
차지 2단계  SectionName=Charge1   Start → Loop → Charge1 ───────→ End
차지 3단계  SectionName=Charge2   Start → Loop → Charge1 → Charge2 → End
```

키를 떼면 **그 섹션의 출구 하나만** 갈아끼운다.

```cpp
Montage_SetNextSection( SectionName , EndSectionName )
```

⚠️ **`SectionName` 은 "다음 섹션"이 아니라 "마지막으로 재생할 섹션"이다.** 키를 떼는 순간 캐릭터는 아직 `Start`에서 멈춰 있다 — 미리 종착역을 정해두고 출발하는 구조다. 이 지점을 세 번 되물었다.

---

## 4. ★ 컷 지점은 포즈 스캔으로 정했다

루프를 아무 데나 자르면 End로 점프할 때 몸이 튄다. **인접 섹션 전환(Loop→Charge1)은 연속 재생이라 안 튀고, 건너뛰는 점프(→End)만 튄다.**

`AS_Speed_Attack_Loop_Seq` 50프레임 전수 스캔 (뼈 15개 회전차 평균, 기준 = `End f0`):

```
f0~f5     0~1.5도    End 시작 포즈와 사실상 동일
f6~f16    13~60도    골반이 137도까지 돌아간다
f17~f27   21~45도
f28~f34   51도
f35~f47   34~38도
f49       11.3도     ← 안무가가 의도한 연결점
```

**중간 컷은 최선이 21.6도(로컬 f25 = 몽타주 f35)로, 의도된 연결점의 약 2배다.** 노티 사이 빈 자리와 겹치는 구간이 `f31~f35` 하나뿐이라 사실상 선택지가 없었다.

⚠️ **섹션 점프는 블렌드가 아예 없어서**, 포즈가 안 맞으면 크로스페이드보다 오히려 더 튄다.

---

## 5. 루프 루트모션 = 계단식 5버스트

전 프레임 스캔 결과, 루트가 **정확히 100cm씩 5번 순간이동**한다.

```
f5 ~f10   0 → 100      5프레임(0.083초)에 100cm = 초당 1200cm
f15~f20   100 → 200
f25~f30   200 → 300
f35~f40   300 → 400
f45~f49   400 → 473.3   (마지막은 73cm)
```

판정 노티 5개가 각 버스트에 하나씩 붙어 있다 — **대시하며 베고, 멈췄다가, 또 대시하며 벤다.** 그래서 차지 단계가 저절로 1m 단위로 떨어진다.

```
차지 1단계   97 + 200 + 409 = 706cm      (Start 97 · End 409 는 항상 붙는다)
차지 2단계   97 + 300 + 409 = 806cm
차지 3단계   97 + 473 + 409 = 979cm
```

⚠️ **최소 이동이 6m다.** `Start`(97cm)와 `End`(409cm, 그것도 앞 0.25초에 다 쓴다)가 차지와 무관하게 항상 붙기 때문이다. 조일 레버 = `MontagePlayRate` 또는 접근 워프.

---

## 6. 타이머 3개 — 어빌리티에는 Tick 이 없다

`UGameplayAbility`는 매 프레임 도는 자리가 없다. "나중에 이걸 해라"는 타이머나 노티로 예약해야 한다.

| 타이머 | 하는 일 | 없으면 |
|---|---|---|
| `FreezeTimer` | Start 끝(0.1667초)에 `Montage_Pause` | 차지가 아예 성립 안 됨 |
| `MaxHoldTimer` | 3초 뒤 강제 키뗌 처리 | 신호 유실 시 영구 정지 |
| 안전 타이머 복구 | 키뗌 후 재장착 | 어빌리티가 안 끝날 수 있음 |

★ **부모가 건 안전망을 끄면 그 자리를 대신할 것이 필요하다.**

```
부모 UKDGameplayAbility   (몽타주 길이 ÷ 재생속도) × 1.5 = 3.375초 뒤 강제 종료
차지                      홀드가 그 길이를 넘긴다 -> ClearSafetyTimer()
그 결과                   "안 끝나는 상황"을 막을 게 없어진다 -> MaxHoldTimer 로 교체
```

**필요 없어서 끄는 게 아니라 길이가 안 맞아서 갈아끼우는 것이다.**

정지 시점을 타이머로 잡을 수 있는 이유는 **몽타주가 자기 섹션 길이를 알려주기 때문**이다(`GetSectionLength(0)`). 하드코딩이 아니라 에셋에서 읽으므로 섹션을 옮겨도 따라간다.

⚠️ `Montage_Pause` 는 **프로젝트에서 이 클래스가 유일 사용처**다. 히트스톱은 `CustomTimeDilation` 방식이라 별개 스위치다(`KDHitStopComponent.cpp:49`). 나중에 히트스톱을 몽타주 정지 방식으로 바꾸면 여기가 깨진다.

📌 `PROJECT_OVERVIEW §2-5` 의 *"히트스탑 = Montage_Pause"* 는 stale이라 이번에 정정했다. 08-04에 컴포넌트로 옮기면서 방식이 바뀐 걸 문서가 못 따라갔다.

---

## 7. 키 뗌 신호는 GameplayEvent 로 받는다

UE 표준 부품(`UAbilityTask_WaitInputRelease`)은 **이 프로젝트에서 안 돈다.**

```
소스 전수 grep   BindAbilityActivationToInputComponent · AbilityLocalInputPressed · InputID  = 0건
발동 경로        Enhanced Input -> TryActivateAbilitiesByTag
```

어빌리티가 입력 ID에 묶여 있지 않아 `Spec.InputPressed`가 영원히 false다. `GA_Parry`가 홀드인데도 별도 `CancelByTag` 경로를 쓰는 이유가 이것이다.

**그런데 스킬 3은 캔슬이면 안 된다** — `End` 섹션에 마무리 타격과 총 1발이 있어서 캔슬하면 통째로 사라진다. 그래서 이벤트를 쓴다.

```
키 뗌 → Handle_SkillHoldStop → Character.TrySkillHoldStop()
     → InputComponent 가 Event.Skill.HoldRelease 발신
     → GA 의 WaitGameplayEvent 수신 → 섹션 링크 교체 + Montage_Resume
```

§1-3 준수(직접 포인터 X). 노티 → GameplayEvent → GA 라는 기존 계약과 같은 모양이다.

---

## 8. 입력 배선 — 기존 구조로 통일

처음에 컨트롤러가 입력 컴포넌트를 직접 잡는 안을 냈다가 **되돌렸다.**

```
채택   Controller → Character → InputComponent      기존 8개와 동일
근거   어떤 컴포넌트를 들고 있느냐는 Pawn 의 내부 사정이다
       컨트롤러가 거기까지 알면 컴포넌트를 쪼갤 때 컨트롤러도 고쳐야 한다
```

⚠️ `Handle_Move`의 `TryMovementCancel`만 캐릭터를 안 거친다. **그게 예외지 본이 아니다.** 이동 관련 작업할 때 같이 정리 대상.

캐릭터의 `Try*`는 판단을 안 하는 한 줄 래퍼다(08-12 리팩토링으로 로직이 전부 `UKDPlayerAbilityInputComponent`로 이사했다). 그래도 5개 추가는 15줄이라 500줄 한도에 영향이 없다.

---

## 9. 조회법 2개 (재사용)

**① 몽타주 섹션 시각은 파이썬으로 못 읽는다**

```
파이썬   get_editor_property('composite_sections')   "Failed to find property"
MCP      inspect 툴의 get_property                    읽힌다
```

반환 문자열의 `LinkValue`가 섹션 시작 시각(초)이다. 같은 이유로 몽타주 `Notifies`도 파이썬에서 "protected"로 막히는데 `AnimationLibrary.get_animation_notify_events()`로는 읽힌다.

**② GE 태그 부여는 `GEComponents` 안에 있다**

UE5.3+ 부터 옛 `Granted Tags` 속성이 아니라 컴포넌트다.

```
UTargetTagsGameplayEffectComponent
  프로퍼티명 = InheritableGrantedTagsContainer   (표시명은 "Add Tags")
```

⚠️ 프로퍼티명이 표시명과 달라서 `InheritableGameplayEffectTags`로는 못 읽는다. 엔진 소스 `TargetTagsGameplayEffectComponent.h:53` 확인.

---

## 10. 값 (실측)

**GA_Skill_01 · 02 · 04** (부모 `KDGameplayAbility_Skill`)

```
StaminaCost 20 · CapsuleRadius 20 · DamageEffectClass KDGameplayEffect_Damage_Physical
HitConfirmProfile DA_HitLightAttack · CooldownGameplayEffectClass GE_SkillCooldown
ActivationOwnedTags   State.Combat.InAction · State.Combat.Attacking
ActivationBlockedTags State.Movement.InAir · State.Combat.Attacking
나머지는 전부 C++ 기본값 (Approach 200/700 · AutoAim 500/90/135 · HitStop 0.08 · Sweep · Sword 소켓)
```

🔴 `CapsuleRadius`는 헤더 기본값이 3이다. **BP에서 20으로 안 올리면 판정이 거의 안 닿는다** — 08-16 작업분이 BP 쪽이라 새 BP는 매번 다시 넣어야 한다.

**GA_Skill_03** (부모 `KDGameplayAbility_SkillCharge`) — 위에 더해

```
ChargeSteps[0]  Loop     0.0
ChargeSteps[1]  Charge1  0.4
ChargeSteps[2]  Charge2  0.8
EndSectionName  End      MaxHoldTime 3.0
```

**GE_SkillCooldown** — HasDuration 1.0 · `TargetTagsGameplayEffectComponent` → `Cooldown.Player.Skill`

스킬 4개가 **태그 하나를 공유**하므로 하나 쓰면 넷 다 1초 잠긴다(승환 결정).

---

## 검증

- [x] 빌드 통과 (승환)
- [x] PIE 전반 동작 확인 (승환) — 발동·자원·쿨다운·판정·차지 3단계
- [x] BP CDO 전수 실측 — 4개 GA + GE + BP_PlayerState 부여 목록
- [ ] `Charge1`(f24.35) 점프 지점 포즈 스냅 — 눈으로 판정 남음 (59도 구간)
- [ ] 첫 Shot(0.1731) 이 정지 직전에 발사되는지 — 어색하면 0.19 로 이동
- [ ] 이동 거리 체감 (풀차지 9.8m)

## 커밋

```
Content  d799c85  GA BP 4개 + GE_SkillCooldown + IA 4개 + GA 폴더 재편 15개 이동
```

## 남은 것

```
스킬2 땅 찍기(1.417초)     번지는 AoE 어빌리티 — GA_ShotBlast 와 같은 층위의 독립 GA 로
                          노티 자리는 의도적으로 비워둠
스킬 UI                   SB식 우하단 슬롯
연출                      스킬 전용 HitConfirmProfile · 슬로모
PoiseDamageByAttack       Skill1~4 항목 없음 = 스킬로 경직이 안 걸린다 (적 밸런싱 때)
```
