# 2026-09-03 — 스킬 연출: 차지 단계 나이아가라 + Skill_01 링

전날 배선한 스킬 몽타주 연출(`2026-09-02-area-blast-ga.md` §스킬 몽타주)의 후속. **Skill_03 차지형만 방식이 통째로 달라야 했다.**

---

## 1. 결론부터 — 홀드 중에는 AnimNotify 가 전부 죽는다

`UKDGameplayAbility_SkillCharge::OnActivated()` 가 진입 섹션 길이만큼 뒤에 몽타주를 **정지**시킨다.

```cpp
// KDGameplayAbility_SkillCharge.cpp:33-35
const float FreezeDelay = AttackMontage->GetSectionLength(0) / Rate;   // 0.167s
World->GetTimerManager().SetTimer(FreezeTimerHandle, this,
    &UKDGameplayAbility_SkillCharge::FreezeAtChargeStart, FreezeDelay, false);
// :86  Anim->Montage_Pause(AttackMontage);
```

홀드하는 동안 몽타주 시간이 **0.167 에서 안 움직인다.** AnimNotify 는 재생 시간이 그 지점을 지나야 발화하므로 **홀드 구간에 꽂은 노티는 하나도 안 터진다.** 에러도 로그도 없다.

→ **연출을 붙일 곳이 몽타주가 아니라 GA 다.** 홀드 경과를 아는 주체가 GA 뿐이다(`ChargeStartTime`).

`Skill_01`·`Skill_02`·`Skill_04` 는 해당 없음 — 정지 구간이 없다.

### AM_SB_Skill_03 실측

```
섹션 5개 / 2.25s
  Start    0.000~0.167   AS_Speed_Attack_Start_Seq
  Loop     0.167~0.406   AS_Speed_Attack_Loop_Seq
  Charge1  0.406~0.583   Loop_Seq
  Charge2  0.583~1.000   Loop_Seq
  End      1.000~2.250   AS_Speed_Attack_End_Seq

AM_SB_Skill_03_Start / _Loop / _End  ← 참조자 0 = 고아
```

⚠️ **섹션 시작 시각은 파이썬으로 못 읽는다** (`CompositeSections` 가 protected). MCP `inspect get_property CompositeSections` 로 뚫으면 `SegmentBeginTime` / `LinkValue` 가 나온다. (`skill-charge-ga` 에 같은 함정 기록됨 — 재확인)

---

## 2. 설계 — GA 타이머 → GameplayCue → 나이아가라

```
OnActivated ─┬─ 0.0s 즉시 ──► NotifyChargeStep(0) ─┐
             ├─ 3.0s 타이머 ─► NotifyChargeStep(1) ─┼─► GameplayCue.Skill.Charge
             └─ 5.0s 타이머 ─► NotifyChargeStep(2) ─┘    RawMagnitude = 단계
릴리즈/취소 ─► ClearStepTimers()                          ↓
                                                    GCN_SkillCharge
                                                    Spine_03 에 NS 부착 + 색 지정
```

**큐는 태그 1 개.** 단계 구분은 `FGameplayCueParameters::RawMagnitude` (`GameplayEffectTypes.h:863`).

기존 `BP_GCN_EnemyTelegraph` 는 자식 태그 3 개(`.Parryable/.Unblockable/.Lethal`)로 갈랐지만, 여기선 **단계 = `ChargeSteps` 배열 인덱스**라 매그니튜드가 맞다. 단계가 늘어도 태그를 새로 안 만든다.

**GC 경유는 룰 강제** — `CLAUDE.md §1-1` "GA 가 UI/Sound/Camera 직접 호출 금지".

### 단일 진실이 유지된다

`MinHoldTime` 하나를 고치면 **애니 분기(`ReleaseCharge :119`)와 이펙트 타이밍이 같이 바뀐다.** 화면에 3단계 색이 떴으면 실제로 `Charge2` 섹션이 나간다. 두 값을 따로 관리하지 않는다.

---

## 3. C++ — 4 파일 / +61 줄

승환이 "너가 수정해줘" 명시 → 이 세션이 직접 편집(§0 예외).

```
KDGameplayTags.h / .cpp             GameplayCue.Skill.Charge 선언·정의
KDGameplayAbility_SkillCharge.h     NotifyChargeStep · ClearStepTimers · TArray<FTimerHandle>
KDGameplayAbility_SkillCharge.cpp   타이머 예약 · Clear 3곳 · 새 함수 2개
```

### 타이머 예약 (`OnActivated` 끝)

```cpp
ClearStepTimers();
StepTimerHandles.SetNum(ChargeSteps.Num());
for (int32 i = 0; i < ChargeSteps.Num(); ++i)
{
    const float Delay = ChargeSteps[i].MinHoldTime;
    if (Delay <= 0.f)
    {
        NotifyChargeStep(i);
        continue;
    }

    FTimerDelegate StepDelegate = FTimerDelegate::CreateUObject(
        this, &UKDGameplayAbility_SkillCharge::NotifyChargeStep, i);
    World->GetTimerManager().SetTimer(StepTimerHandles[i], StepDelegate, Delay, false);
}
```

★ **`FTimerDelegate` 를 쓴 이유** — `SetTimer` 의 기본형은 **인자 없는 함수**만 건다(기존 `FreezeAtChargeStart` 가 그 형태). `NotifyChargeStep(int32)` 는 인자를 받아서 못 건다. `CreateUObject` 의 페이로드로 `i` 를 싸두면 타이머가 나중에 그 숫자와 함께 부른다. 안 쓰면 `NotifyStep0/1/2` 를 단계 수만큼 만들어야 한다.

📌 `FTimerDelegate` 는 **비동적 델리게이트라 `UFUNCTION` 이 필요 없다.** `CreateUObject` 는 대상이 파괴되면 자동 무효화.

📌 `Delay <= 0.f` 분기 — `SetTimer` 에 0 을 넣으면 UE 가 타이머를 안 걸고 조용히 무시한다. 0 단계는 즉시 호출.

### 발신

```cpp
void UKDGameplayAbility_SkillCharge::NotifyChargeStep(int32 StepIndex)
{
    // 기능 : 차지 단계 도달 - 연출은 GC 위임
    if (bChargeReleased) { return; }

    FGameplayCueParameters CueParams;
    CueParams.RawMagnitude = static_cast<float>(StepIndex);
    K2_ExecuteGameplayCueWithParams(GameplayTags::GameplayCue_Skill_Charge, CueParams);
}
```

`K2_ExecuteGameplayCueWithParams` = `GameplayAbility.h:691`. `K2_` 는 **Kismet 2(BP 옛 이름) 접두사일 뿐 평범한 C++ 함수**다. 내부(`GameplayAbility.cpp:1710`)는 `AbilityLevel` 을 채우고 `ASC->ExecuteGameplayCue()` 를 부른다 — ASC 를 직접 부르는 것보다 이쪽이 낫다.

### `ClearStepTimers()` 3 곳

| 위치 | 이유 |
| --- | --- |
| `OnActivated` 시작 | `InstancedPerActor` 라 이전 활성화의 핸들이 남는다 |
| `ReleaseCharge :98` | **0.5초에 떼면 0.8초 타이머가 살아 있다** — 안 지우면 검을 휘두르는 중에 3단계가 혼자 뜬다 |
| `OnCleanup :71` | 취소·사망 |

★ 타이머는 **처음에 전부 예약해두고 각자 독립적으로** 터진다. `bLoop=false` 라 터지면 자멸한다. **"단계마다 이전 것을 끄고 반복"이 아니다** — 반복문은 예약할 때 한 바퀴만 돈다. (승환이 이 지점에서 헷갈려 브릿지에 기록)

★ **include 추가 불필요** — `SkillCharge.h → Skill.h → PlayerMelee.h → MeleeTrace.h → KDGameplayAbility.h → Abilities/GameplayAbility.h` 로 `FGameplayCueParameters` 완전형이 도달한다.

---

## 4. 에셋 — NS 선정에서 두 번 갈아탔다

### 1차 `NS_AuraFX_Scifi` → 체급 미달

```
NS_AuraFX_Scifi     에미터 2개   ember_Black · ember_Black001
NS_SB_ChargingBow   에미터 10개  NE_Ring · NE_Shockwave_Air · NE_Chromatic
                                 NE_Sparks · NE_Light · NE_AnamorphicLens
                                 NE_Energy_Line_01 · _02 · NE_Ref · NE_Spot_Light
```

승환 관측 = **"Aura 가 잘 안 나온다"**. 잔불 두 줄기 vs 링·충격파·스파크·라이트 10 종. 애초에 체급이 달랐다.

### ★ AuraFX 계열은 파라미터로 형태를 구분할 수 있다

| NS | 에미터 | `User.Color` | 형태 단서 |
| --- | --- | --- | --- |
| `NS_AuraFX_Scifi` | 2 | Vector3f | ⚠️ **`SwordLength`** = 검 부피 |
| `NS_AuraFX_Mystic` | 3 (+Lightning) | Vector3f + **`Color2`** | **`Sphere Radius`** = 몸 |
| `NS_AuraFX_Dark` | 1 (Mesh) | Vector3f | `SM_HalfSphere_02` · `Mesh Scale` = 반구 |
| `NS_AuraFX_Lightning` | 2 | **LinearColor** | 파라미터 1개뿐 |

**`SwordLength` 가 있으면 검용, `Sphere Radius` 면 몸용.** 이름만 보면 다 같은 "Aura" 라 구분이 안 된다.

### 2차 `NS_SB_ChargingBow` 복제 → 색 파라미터가 없다

`userParameterCount = 0`. 런타임 틴트가 불가능하다. (같은 계열인 `NS_SB_Slash_PowerUp_01` 도 0 개)

→ 처음엔 **색깔 수만큼 에셋 분리**(`NS_SB_Charge_01/02/03` 복제)로 갔으나, 승환이 **"NS 에서 사용자 노출 값을 추가하면 안 되나"** 제안 → 그쪽이 정답이었다.

### 최종 — 유저 파라미터를 직접 뚫었다

```
NS_SB_Charge_01   ChargingBow 복제
  User.LinearColor  (LinearColor)   ← 새로 노출
  각 에미터 Initialize Particle > Color 를 Link Inputs 로 연결
  끈 에미터 = NE_Spot_Light · NE_Light
```

★ **참고 답안이 프로젝트 안에 있었다** — `NS_SB_Telegraph_Red` 가 **완전히 같은 10 에미터 계열인데 `User.Color`(LinearColor)가 이미 뚫려 있다.** 어느 모듈을 물렸는지 그대로 보고 따라 할 수 있다.

⚠️ **타입이 맞아야 `Link Inputs` 목록에 뜬다.** `Initialize Particle > Color` 는 `LinearColor` 라 유저 파라미터도 `LinearColor` 여야 한다. `Vector` 로 만들면 목록에 안 보인다.

⚠️ **에미터마다 내부 색이 달라서 전부 링크하면 한 색으로 뭉개진다.** 살릴 것만 골라서 링크했다.

🔴 **색 변경에 한계가 있다** (승환 판정, 현행 유지). 에미터 내부 색과 머티리얼(`MI_FX_Slash_Flare_*` 등)이 지배적이라 `User.Color` 로 밀 수 있는 폭이 좁다. 완전 통제가 필요해지면 복제 3 개 수동 색칠로 돌아간다.

---

## 5. BP — `GCN_SkillCharge`

```
/Game/SB_Style_GameProject/GAS/GC/GCN_SkillCharge
부모  GameplayCueNotify_Actor          (GCN_PerfectDodge · GCN_HitReact 와 동일)
태그  GameplayCue.Skill.Charge
      Auto Destroy On Remove ☑ / Delay 2.0
```

🔴 **경로가 스캔 목록 안이어야 한다.** 밖에 만들면 **에러 없이 그냥 안 잡힌다.**

```ini
; DefaultGame.ini:19-20
+GameplayCueNotifyPaths=/Game/SB_Style_GameProject/GAS/GC
+GameplayCueNotifyPaths=/Game/Blueprints/AbilitySystem/GameplayCueNotify
```

### `OnExecute` 그래프 (9 노드)

```
On Execute ─exec─► Spawn System Attached ─exec─► Set Niagara Variable (LinearColor) ─exec─► Return
                     SystemTemplate     NS_SB_Charge_01
                     AttachToComponent  ◄── Get Component by Class (MyTarget, SkeletalMesh)
                     AttachPointName    Spine_03
                     LocationType       SnapToTarget
                     bAutoDestroy ☑  bAutoActivate ☑
                     ReturnValue ─────────────────► self
                                                    InVariableName  LinearColor

Break GameplayCueParameters ─► RawMagnitude ─► Truncate ─► Select(Vector) ─► To LinearColor ─► InValue
                                                            Option0 (1, 1, 1)
                                                            Option1 (1, 0.8, 0.2)
                                                            Option2 (1, 0.2, 0.1)
```

### BP 함정 4 개

1. 🔴 **`Set Niagara Variable **By String** (...)` 은 5.3 deprecated.** 검색하면 같이 뜬다. `By String` **없는** 쪽(FName 버전 = `SetVariableVec3` / `SetVariableLinearColor`)을 골라야 한다.
2. 🔴 **`InVariableName` 이 비어 있으면 조용히 아무것도 안 한다.** 실제로 한 번 비어 있었다. `User.` 접두사는 **빼고** 쓴다.
3. ⚠️ **`Select` 는 출력을 먼저 연결해야 Wildcard 가 굳는다.** `Index` 부터 꽂으면 타입이 안 정해져서 옵션 핀에 값을 못 넣는다.
4. ⚠️ `RawMagnitude` 는 float 라 `Select.Index`(정수)에 바로 못 꽂는다 → **`Truncate`**.

### ★ `To LinearColor (Vector)` 는 알파를 1.0 으로 채운다

`Select` 를 Vector 로 두고 변환 노드를 끼워도 된다. 알파가 0 이 될까 걱정했으나 **`Color.cpp:45` 가 `A(1.0f)`** 로 채운다 (`Conv_VectorToLinearColor` → `FLinearColor(FVector3d)`).

### ★ `OnExecute` 반환값은 버려진다

`GameplayCueNotify_Actor.cpp:286` 이 `OnExecute(MyTarget, Parameters);` 로 호출하고 결과를 안 받는다. `Return` 이 `false` 여도 **문제 없다.** (엔진 기본 구현도 `return false`)

---

## 6. 최종 값

```
GA_Skill_03  (Class Defaults)
  ChargeSteps  [0] Loop     0.0     ← 🔴 반드시 0.0
               [1] Charge1  3.0
               [2] Charge2  5.0
  EndSectionName  End
  MaxHoldTime     6.0
```

🔴 **`[0].MinHoldTime` 을 0 보다 크게 하면 안 된다.** 그 시간 전에 떼면 `ReleaseCharge :119` 가 조건을 만족하는 단계를 못 찾아 `Step = nullptr` → `Montage_SetNextSection` 을 안 타고 **`Loop → Charge1 → Charge2 → End` 전부가 재생된다.**

⚠️ **5초 홀드면 몽타주가 0.167 지점에서 5초간 정지한다.** 승환 판정으로 현행 유지. 정지가 거슬리면 `Loop` 섹션의 `Next Section` 을 자기 자신으로 바꾸고 `Montage_Pause` 를 빼는 재설계가 필요하다(원본 애니 이름이 `Start/Loop/End_Seq` 인 걸 보면 **애초에 그 구조로 만들어진 에셋**이다).

---

## 7. Skill_01 — 방사형 링 (별건)

같은 날 처리. 승환 요청 = **"펼쳐지는 방사형"**.

### ★ 이미 링이 있었다

`NS_SB_Slash_PowerUp_01` 은 `M_FX_Shockwave_Air` + **`SM_FX_Circle_01`(원판 메시)** 를 물고 있다. 같은 조합이 `NS_PerfectDodge` — 퍼펙트 회피 때 보이는 그 링이다. **"방사형이 없다"가 아니라 작아서 안 보였던 것.**

⚠️ **`ENiagaraRingDiscMode` 는 방사형 신호가 아니다.** 거의 모든 NS 에 붙어 있는 Shape Location 모듈의 enum 이라 그걸로 판별하면 전부 걸린다. **진짜 단서는 머티리얼·메시**(`MI_FX_Slash_Shockwave_Air_00*`, `SM_FX_Circle_01`, `DecalRenderer`, `SM_GroundAttackRock_02`).

### 교체

```
AM_SB_Skill_01  VFX 트랙  t=0.100
  NS_SB_Slash_PowerUp_01  ->  NS_SB_Telegraph_Red     Z 오프셋 100 (승환 조정)
                t=0.600     NS_SB_Slash_PowerUp_Burst_01  (유지)
```

`NS_Telegraph_Red` 는 `BP_GCN_EnemyTelegraph` 가 쓰고 있어 **복제 후 사용**(원본을 고치면 적 예고가 같이 바뀐다).

📌 `soc_fx_root` = `root` 본이라 **오프셋 0 이 바닥 높이**다. 처음에 바닥으로 내렸으나 승환이 100(가슴 높이)으로 올렸다.

### Vefects Shockwave 84 개 삭제

`Effect/Shockwave/` 95 에셋 삭제(외부 참조 0 확인). 원본은 `/Game/Vefects/Easy_Shockwaves_VFX/` 에 그대로.

⚠️ Vefects 계열은 **`User.Color` 가 단색이 아니라 컬러커브**(`NiagaraDataInterfaceColorCurve`)라 노티·BP 에서 색을 못 바꾼다. 그래서 팩이 18 색을 미리 구워 놓았다.

---

## 8. Skill_02 — 땅 찍기 임팩트 3종

같은 날 마무리. **VFX · 슬로우 · 카메라 쉐이크를 같은 프레임에 겹쳤다.**

```
AM_SB_Skill_02   2.583s / 60fps (AS_Skill_04_Seq 155프레임)

  0.050~0.999   검 3연격        WeaponTrail · MeleeTrace · Sound_Swing
  0.999 (60f)   VFX             NS_SB_Free_Magic_Circle2 @ soc_fx_root
  0.999 (60f)   WindupSlow      SlowRate 0.2 -> 1.0 · 1.241 까지
  0.999 (60f)   CameraShake     AN_CameraShake + CameraShake_SB_XL
  1.008 (60.5f) GroundBlast     AN_SendGameplayEvent -> Event.Montage.AreaBlast
```

### NS — `NS_SB_Free_Magic_Circle2`

`/Game/Free_Magic/VFX_Niagara/NS_Free_Magic_Circle2` 복제 (원본은 `Free_Magic_Map` 데모 레벨이 참조).

★ **유저 파라미터 19개** — 에미터 5개 각각에 `Color_*` / `Scale_*` / `Velocity_*` / `Shape_*` + 전체 `Scale_All`. 지금까지 본 NS 중 가장 잘 열려 있다. `NS_SB_Charge_01` 에서 겪은 "에미터 내부 색이 지배해서 색이 안 바뀐다" 문제가 여기선 구조적으로 없다.

승환이 **`Ray` · `Mesh1` 만 켜고 나머지 3개(`Sparks1` `Circle` `Sparks2`)는 껐다.**

⚠️ **수명 파라미터는 노출돼 있지 않다.** `Loop Duration`(Emitter State) · `Lifetime`(Initialize Particle)을 에셋 안에서 고쳐야 한다. **MCP·파이썬으로 못 읽는 값**이라 검증은 PIE 눈으로만 가능하다.

### ★ 슬로우와 나이아가라 길이는 묶여 있다

`UKDAnimNotifyState_WindupSlow` 는 **`Montage_SetPlayRate` 로 몽타주만** 늦춘다(`CustomTimeDilation` 아님). **나이아가라는 실제 시간으로 그냥 흐른다.**

```
필요 NS 길이(초) = 구간 애니 길이 ÷ SlowRate
                 = 0.242 ÷ 0.2  =  1.21초   (60fps 73프레임)

SlowRate 0.15 -> 1.61s (97f) / 0.20 -> 1.21s (73f) / 0.30 -> 0.81s (48f) / 0.40 -> 0.61s (36f)
```

🔴 **`SlowRate` 를 먼저 확정하고 NS 를 자른다.** 반대로 하면 슬로우 세기를 바꿀 때마다 NS 를 다시 손봐야 한다.

📌 **사라지는 시각 = `Loop Duration` + `Lifetime`.** 스폰이 멈춰도 마지막 입자가 `Lifetime` 만큼 더 산다.

### `UKDAnimNotifyState_WindupSlow` 재사용

원래 **적 공격 예고용**으로 만든 클래스인데(주석: 비대칭 전투 — 적만 예고) 플레이어 스킬에 그대로 썼다.

```cpp
// NotifyBegin :30   Montage_SetPlayRate(Montage, GetEffectiveSlowRate(...))
// NotifyEnd   :47   현재 재생률이 "내가 건 값"일 때만 RestoreRate 로 복구
```

★ **`NotifyEnd` 의 재생률 비교(`:47`)가 안전장치다.** 윈드업 중 다른 몽타주로 갈아타면(경직 등) 무조건 복구하는 게 아니라 **남의 재생률을 안 덮는다.**

⚠️ **`SlowRate` 하한이 0.05** — `PlayRate = 0` 이면 몽타주 시간이 안 흘러 `NotifyEnd` 가 영영 안 온다(**자기가 건 정지를 자기가 못 푼다**). 완전 정지는 이 도구로 불가능하고 `UKDHitStopComponent`(`CustomTimeDilation = 0` + `ResumeTimer`)가 담당한다.

⚠️ **플레이어에겐 배수가 안 먹는다.** `GetEffectiveSlowRate :11` 이 `Cast<AKDEnemyBaseCharacter>` 로만 `TelegraphSlowMultiplier`(적 DA)를 곱한다. 플레이어는 캐스팅 실패 → `Multiplier = 1.0` → `SlowRate` 원값 사용. **동작에는 문제 없다.**

⚠️ 헤더 주석의 경고 — **루트모션 전진 구간에 걸치면 이동 거리가 줄어든다.** 마지막 `MeleeTrace` 가 0.916 에 끝나 겹치지 않는다.

### 카메라 쉐이크 — 자산이 이미 있었다

```
Effect/CameraShake/
    AN_CameraShake                          애님노티파이 (GhostSamurai 복제본)
    CameraShake/CameraShake_SB_{SS,S,M,L,XL,SP}
```

노티 프로퍼티는 `CameraShakeAsset` · `User Play Space Rot` 둘뿐. **노티 꽂고 에셋만 지정하면 끝** — 코드·GC 불필요.

★ **GC 경로도 있었지만 안 썼다.** `UKDAnimNotify_PlayerCue`(`CueTag` 하나로 소유자 ASC 에 큐 발신) + `GAS/GC/CameraShake/LCS_*` 5종이 기존 전투 쉐이크 경로다. 다만 땅 찍기용 태그·GCN 이 없어 **태그 추가 + C++ + BP 생성**이 필요하다. **어떤 세기가 맞는지도 모르는 채 배선부터 늘리지 않으려고** `AN_CameraShake` 로 먼저 갔다. 세기 확정 후 GC 로 옮기는 건 언제든 가능하다.

⚠️ **쉐이크는 실제 시간으로 흔들린다.** 몽타주만 느려지므로 **흔들림이 먼저 끝나고 느린 화면이 이어진다.** 승환 판정 = 이대로 OK.

## 9. 검증

PIE 통과 (승환). 홀드 단계별 색 전환 확인.

```
Skill_03
  ✅ 0 / 3 / 5초 단계 전환
  ✅ 조기 릴리즈 시 남은 단계 미발화     ClearStepTimers
  ✅ 반복 사용 시 컴포넌트 미누적        bAutoDestroy
  🟡 색 변경 폭 제한                     에미터 내부 색 지배 - 현행 유지

Skill_02
  ✅ VFX · 슬로우 · 쉐이크 동시 발동     승환 판정 "딱 좋다"
  ✅ CameraShake_SB_XL 세기 적정
  ✅ 슬로우 해제와 NS 종료 정렬          73프레임 기준으로 승환이 직접 조정
```

## 10. 남은 것

- `NS_SB_Charge_02` / `_03` — 유저 파라미터 방식으로 전환하면서 **미사용.** 정리 대상
- `AM_SB_Skill_03_Start` / `_Loop` / `_End` — 참조자 0 인 고아 3 개
- `NS_SB_Charge_01` 의 `NE_Chromatic` — 색수차라 `User.LinearColor` 를 안 따를 가능성
- 카메라 연출 — Skill_02 는 `AN_CameraShake` 로 끝냈다. **Skill_01 · 03 · 04 는 아직 없다**
- 스킬 전용 `HitConfirmProfile` — 현재 `DA_HitLightAttack`(CameraShakeClass = None) 공용
- 적 DA `PoiseDamageByAttack` — `Skill1~4` · `CounterSlash` · `AreaBlast` 키 누락 (조용히 Poise 0)

## 커밋

```
코드    7567920  [GAS] 차지 단계 도달 GameplayCue 발신 - 홀드 중 노티 불가 대응
Content b1bdb60  [BP] Skill_01 첫 VFX = NS_SB_Telegraph_Red 로 교체
        bf0ad07  [chore] Effect/Shockwave 삭제 - Vefects 복사본 95개
        0bf77fe  [BP] Skill_01 링 높이 조정 - Telegraph_Red Z 0 -> 100
        00abf0e  [BP] GCN_SkillCharge 생성
        014152f  [BP] 차지 단계 NS 3개 복제 - ChargingBow 기반
        f5aa536  [BP] Skill_03 차지 단계 연출 완성
        26e7cf4  [BP] NS_SB_Free_Magic_Circle2 복제 - Skill_02 땅 찍기 후보
        6538955  [BP] Skill_02 땅 찍기 VFX
        fc3ec11  [Anim] Skill_02 임팩트 슬로우 - WindupSlow
        6c73630  [Anim] Skill_02 카메라 쉐이크 - CameraShake_SB_XL
```
