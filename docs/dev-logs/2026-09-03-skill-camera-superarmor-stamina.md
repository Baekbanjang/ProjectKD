# 2026-09-03 (2) — 스킬 카메라 · 슈퍼아머 · 스태미나 리젠 폐지

같은 날 [나이아가라 연출](2026-09-03-skill-charge-vfx.md) 이후 이어진 3건. 성격이 달라 로그를 나눴다.

---

## 1. 스킬 카메라 — 시점을 안 뺏는 방식(B) 채택

### ★ SB 는 스킬에서 시점을 안 뺏는다 (실측 근거)

Eve 액션 511파일 전수에서 카메라를 건드리는 키는 셋뿐이고, **셋 다 뺏는 게 아니라 얹는다.**

```
SBShowCamShakeKey            216건   쉐이크 · FOV 펀치
SBShowControlCamLagSpeedKey   55건   액션별 카메라 랙 속도
SBShowCamAnimKey              62건   시네마틱 카메라 애니

시점 고정 · 입력 차단 키 = 목록에 없음
```

반격 1타(`P_Eve_Sword_JustParry_LightAttack1`) 실측:

```
0.000  CamShake            FOV -70 / Freq 1.5 / 0.4s      렌즈 펀치 한 번. 위치·회전 X
0.000  ControlCamLagSpeed  TargetSpeed 1.0 · MaxDistance 300 · Duration 0.35
                           평시 랙 속도 19 를 1.0 으로 떨궈 카메라를 일부러 뒤처지게
```

🟡 같은 파일 0.000 에 `Effect` 3종(`BlockMove_Step` · `BlockRotation_Skill` · `UseSkill_Step`)이 붙는다. `SBShowEffectKey` 는 VFX 가 아니라 **게임플레이 상태 부여(GE)** 다. ⚠️ `BlockRotation_Skill` 이 캐릭터 회전을 막는지 카메라를 막는지는 이 자료로 못 가린다 — `BlockMove` 와 나란히 있고 둘 다 `ActorState` 라 **캐릭터 쪽**으로 읽는 게 자연스럽다(해석).

→ **승환 판단 = 시점 고정 안 함.** SB 와 같은 결론.

### 세 가지 길 비교

| | A · 레벨 시퀀스 | **B · 카메라 애니메이션** | C · 랙 버스트 |
| --- | --- | --- | --- |
| 방식 | 뷰 타겟을 뺏는다 | **게임플레이 카메라에 오프셋 가산** | 랙 속도를 낮춰 뒤처지게 |
| 플레이어 시점 | ❌ 뺏김 | ✅ 유지 | ✅ 유지 |
| 캐릭터 연출 | ✅ 가능 | ❌ 카메라만 | ❌ |
| 선불 | 태그 + C++ + GCN + 시퀀스 | 플러그인 1개 | **0 — 이미 다 있다** |
| 프로젝트 기존 사용처 | 처형(`KDGameplayCueNotify_ExecutionCamera`) | 없음 | `GCN_CameraLagBurst` |

**B 채택.** A 는 처형급 연출용, C 는 Skill_02 후보로 남겼다가 **불필요 판정**(아래).

### ★ 카메라 애니메이션의 동작 — 오프셋 가산

```cpp
// CameraAnimationCameraModifier.cpp:445-452, 458
FCameraAnimationHelper::ApplyOffset(InOutPOV, CameraOffset, AnimatedLocation, AnimatedRotation);
InOutPOV.Location = AnimatedLocation;
InOutPOV.Rotation = AnimatedRotation;
InOutPOV.FOV = OriginalFieldOfView + DeltaFieldOfView * Scale;
```

`ModifyCamera` 는 **스프링암·락온·조작 계산이 끝난 `InOutPOV` 위에서** 돈다. 그래서 시점을 안 뺏고 락온도 살아 있다.

`Scale`(`:434`)이 **위치·회전·FOV 에 모두 곱해진다.** 시퀀스 하나로 세기를 조절한다.

### 🔴 플러그인 — `TemplateSequence` 를 켜야 한다

```
EngineCameras     EnabledByDefault true    이미 켜짐 (PlayCameraAnimation 제공)
TemplateSequence  EnabledByDefault false   🔴 수동 활성 (Camera Animation Sequence 에셋 타입)
```

⚠️ UE5.6 에서 모디파이어는 `GameplayCameras` 가 아니라 **`EngineCameras`** 플러그인에 있다.
⚠️ 에셋 생성 메뉴는 `Animation` 이 아니라 **`Cinematics`** 아래다 (`AssetDefinition_TemplateSequence.h:24` = `EAssetCategoryPaths::Cinematics`).

### 🔴 FOV 는 절대값이 아니라 "차이"로 먹는다

```cpp
// CameraAnimationSequencePlayer.cpp  ResetDefaultValues()
FieldOfView = ViewInfo.FOV;                    // 매 프레임 현재 게임 FOV 로 초기화
CurrentFocalLength = (Filmback.SensorWidth / 2.f)
                   / FMath::Tan(FMath::DegreesToRadians(FieldOfView / 2.f));
```

**기준 초점거리가 고정이 아니라 현재 게임 FOV 에서 매 프레임 역산된다.** 우리 게임 FOV 는 거리 커브로 변한다(`KDPlayerCameraManager.cpp:69`).

```
게임 FOV 75 → 기준 15.48mm → 18mm 키면 -8.1도   조여짐 ✅
게임 FOV 65 → 기준 18.65mm → 18mm 키면 +1.8도   넓어짐 🔴 반대로 작동
```

📌 **락온 중엔 `DefaultFOV`(75) 로 고정된다**(`:57`) → 락온 걸고 테스트하면 기준선이 확정돼 판정이 정확하다.

### ★★ 시네카메라 센서폭은 36mm 가 아니라 **23.76mm**

`UCineCameraComponent` 기본 Filmback = 16:9 Digital Film. 이걸 36mm 로 잘못 알려줘서 환산표가 통째로 빗나갔었다.

```
초점거리 → FOV  (센서폭 23.76mm)
  15.5mm  75.0도   ← 게임 기본. 여기가 ±0 기준선
  17mm    69.7도   -5.3
  18mm    66.9도   -8.1   ← 채택
  20mm    61.4도   -13.6
  28mm    46.0도   -29    과함
```

초점거리를 애니메이션하면 `RecalcDerivedData()`(`CameraAnimationCameraModifier.cpp:429`)가 FOV 로 변환한다 — **별도 FOV 트랙이 필요 없다.**

### Skill_02 — 한 프레임에 4종

```
AM_SB_Skill_02   2.583s / 60fps

0.999 (60f)   VFX          NS_SB_Free_Magic_Circle2
0.999 (60f)   WindupSlow   SlowRate 0.2 -> 1.0 (1.241 까지)
0.999 (60f)   CameraShake  AN_CameraShake + CameraShake_SB_XL
0.999 (60f)   CameraAnim   AN_PlayCameraAnim + CAS_Skill_02_Impact
1.008 (60.5f) GroundBlast  Event.Montage.AreaBlast
```

`AN_PlayCameraAnim` — 변수 4개(`CameraAnim` `Scale` `EaseIn` `EaseOut`) · `Get Player Controller` → `Get EngineCamerasSubsystem` → `Play Camera Animation`.

⚠️ **랙 버스트는 안 넣었다** (승환 판단). VFX·슬로우·쉐이크·카메라애님으로 충분.

### Skill_03 차지 줌 — 루프 + 명시 정지

차지 중엔 **캐릭터가 정지**(`Montage_Pause`)라 랙·쉐이크가 안 통한다. 카메라를 직접 움직이는 수밖에 없다.

```
CAS_Charge   Location X 125 · 초점거리 18mm
             키 2개(0.00 / 0.10) 값 동일 → bLoop 로 그 오프셋을 붙들어둔다

GCN_SkillCharge (15노드)
  [기존] Spawn System Attached → Set Niagara Variable (LinearColor)
  [추가] Stop All Camera Animations Of (CAS_Charge, bImmediate=false)
       → Play Camera Animation (CAS_Charge, bLoop=true, EaseIn 0.25 / EaseOut 0.30)
              Scale ◄── Select(float) 0.35 / 0.65 / 1.00 ◄── 기존 Truncate 재사용

AM_SB_Skill_03   CameraAnim 트랙 · 1.033(62f) AN_StopCameraAnim
```

★ **`StopAllCameraAnimationsOf`(`EngineCamerasSubsystem.h:62`) 가 핵심.** 시퀀스 에셋만으로 모든 인스턴스를 멈춘다 — **핸들 저장이 불필요**하다. AnimNotify·GCN 은 상태를 못 들고 있어서 이 함수가 유일한 길이다.

★ **Skill_02 와 키 구성이 정반대다.**
```
CAS_Skill_02_Impact   0 → -40 → 0   한 번 튀었다 복귀    bLoop = false
CAS_Charge            125 → 125     계속 유지            bLoop = true
```

⚠️ **`bImmediate = true` 로 하면 안 된다.** 단계 전환 때 오프셋이 0 으로 튕겼다가 다시 들어와 화면이 한 번 뜬다. `false` 로 크로스 블렌드.

⚠️ **`End` 섹션 경계(1.000)에 정확히 두지 않는다.** 섹션 점프로 진입할 때 경계 프레임 노티는 놓칠 수 있어 **2프레임 안쪽(1.033)** 에 뒀다.

🔴 **안전망 없음** — 취소·사망으로 몽타주가 끊기면 `End` 를 안 지나 줌이 남는다. 다음 Skill_03 사용 시 `Stop` 이 먼저 돌아 자동 정리되므로 영구 고장은 아니다. **재현 확인 후 판단**(보류).

### 📌 시퀀서 함정

- **액터를 넣지 않는다.** 팩토리가 `ACineCameraActor` 를 바인딩으로 고정한다(`CameraAnimationSequenceFactoryNew.cpp:20`). 레벨 시퀀스처럼 액터를 끌어다 넣을 자리가 없다.
- 런타임엔 실물 카메라가 아니라 **대역**(`UCameraAnimationSequenceCameraStandIn`)에 평가한다.
- **시퀀서 프리뷰로는 판단이 안 된다.** `(0,0,0)` 이 "현재 게임 카메라 자리"인데 시퀀서엔 게임 카메라가 없다 → **PIE 가 유일한 검증**이다.
- 🔴 **첫 키와 끝 키가 0 이어야 한다**(일회성 연출 한정). 오프셋이라 끝에 값이 남으면 카메라가 어긋난 채 남는다.

---

## 2. 슈퍼아머 — 태그는 있었고 배선이 반쪽이었다

### 발견

```
State.Combat.SuperArmor          KDGameplayTags.h:88-89 / .cpp:70
  주석: "HitReact GA만 차단, 데미지/포이즈/넉백은 통과. 공격 GA Owned Tags로 부여"

차단하는 쪽   KDGameplayAbility_EnemyHitReact.cpp:21   ✅ 배선됨
부여하는 쪽   GA BP 17개 전수 조회 → 0건               ❌ 아무도 안 준다
```

**적 히트리액트가 태그를 기다리는데 붙여주는 놈이 없었다.** 의도된 비대칭이 아니라 미완이다.

### 스킬이 끊기던 실제 경로

`GA_HitReact` 의 `CancelAbilitiesWithTag` 는 비어 있다. **GAS 가 취소하는 게 아니다.**

```
피격 → UKDCombatAttributeSet::SendHitReact (:180-191) → Event.Combat.HitReact
     → GA_HitReact 트리거 → 리액션 몽타주 재생
     → 같은 슬롯이라 스킬 몽타주가 밀려남 → OnMontageCompleted(bInterrupted) → 스킬 종료
```

**몽타주가 덮여서 끝난다.** 그래서 `GA_HitReact` 를 안 켜는 게 정답이다.

### 배선 — BP 5개 · 코드 0줄

```
GA_HitReact           ActivationBlockedTags  + State.Combat.SuperArmor
GA_Skill_01~04        ActivationOwnedTags    + State.Combat.SuperArmor
```

`ActivationOwnedTags` 는 GA 가 활성인 동안만 붙고 끝나면 자동으로 떨어진다 — 직접 지울 필요가 없다.

```
데미지 · 실드경감 · 넉백 · 히트 이펙트   그대로 통과
리액션 몽타주 · 스킬 중단                 차단
```

⚠️ 차지 홀드 중에도 GA 가 살아 있어 **Skill_03 은 최대 6초 슈퍼아머**다. 길다고 느끼면 홀드 구간만 빼는 방법이 있다 — 현행 유지.
⚠️ **평타 콤보엔 주지 않았다.** 20개 전부 슈퍼아머면 얻어맞으며 계속 때릴 수 있어 게임이 물러진다.
📌 적 보스에게 슈퍼아머를 주고 싶으면 그쪽 GA 에 Owned Tags 만 추가하면 된다 — 차단은 이미 걸려 있다.

---

## 3. 스태미나 — 시간이 주는 자원 → 때려서 버는 자원

### 개조 전 현황

```
어트리뷰트   UKDPlayerAttributeSet  Stamina / MaxStamina = 100 / 100
             KDPlayerAttributeSet.cpp:11-12 · 클램프 2중(:26-29, :41-44)
리젠         GE_StaminaRegen (BP) · INFINITE · Period 0.1초
             Ongoing Ignore: State.Stamina.RegenBlocked
             부여처 = BP_PlayerState StartupEffects (유일 참조)
소모         스킬 10 (KDGameplayAbility_Skill.cpp:50)
```

⚠️ `KDGameplayTags.h:95` 주석이 "StaminaComponent가 루즈 태그로 부여"라고 하는데 **`UKDStaminaComponent` 는 실재하지 않는다.** 주석이 낡았다.

### 개조

**끄기** — `BP_PlayerState` StartupEffects 에서 `GE_StaminaRegen` 제거. 코드 0줄.
GE 에셋과 `State.Stamina.RegenBlocked` 태그는 되돌릴 수 있게 남겨뒀다. 실드·탄약 리젠은 유지.

**채우기** — `UKDCombatAttributeSet::GainAttackerStamina` 신설.

```cpp
// KDCombatAttributeSet.cpp:99  — ToHealth 확정 후 · Health 차감 전
GainAttackerStamina(Data, ToHealth);
```

```
:61   패링에 삼켜지면 return        → 회복 X
:96   ToHealth <= 0 이면 return     → 실드가 다 먹으면 회복 X
:99   회복
:104  Health 차감 (HandleDeath 동기 완료. 이 아래 코드 금지)
```

★ **데미지 게이트웨이에 붙였다** (CLAUDE.md §1-2). 한 곳만 고쳐도 평타 20 · 스킬 4 · 총 · 범위가 전부 커버된다. GA 마다 넣으면 `OnTargetHit` 패턴이 3곳에 복제돼 있어 중복이 된다.

★ **평타 화이트리스트** — `Ability.Player.Light` / `Heavy` 만 통과.
```
화이트리스트   스킬이 5·6번으로 늘어도 자동 제외
블랙리스트     추가할 때마다 고쳐야 하고, 까먹으면 에러 없이 회복된다
```
적은 `Ability.Player.*` 를 안 써서 **소유자 검사까지 겸한다.**

★ **회복량 = 피해 × `KD.StaminaGainRate`** (기본 0.2). CVar 로 PIE 중 튜닝. 이 파일이 이미 `CVarShowDamage` 를 쓰고 있어 패턴이 같다.
⚠️ `#if !UE_BUILD_SHIPPING` **바깥**에 둔다. 안에 넣으면 시핑에서 이름이 사라져 호출부가 깨진다.

★ **`SetNumericAttributeBase`** 사용 — 같은 함수 `:104` 가 Health 에 이미 그 방식을 쓴다. `PostGameplayEffectExecute` 안에서 GE 를 새로 적용하면 재진입 위험이 있다.

### 🔴 처음엔 안 됐다 — `GetAbility()` 가 항상 nullptr

**컨텍스트를 만드는 함수가 둘인데 하는 일이 다르다.**

```cpp
UGameplayAbility::MakeEffectContext()          → Context.SetAbility(this)   있음 (GameplayAbility.cpp:1890)
UAbilitySystemComponent::MakeEffectContext()   → 없음
```

`KDAbilityStatics.cpp:103` 이 **ASC 쪽**을 쓴다 → `GetAbility()` 가 항상 `nullptr` → `GainAttackerStamina` 가 첫 관문에서 매번 `return`.

⚠️ **"엔진이 자동으로 채운다"는 내 설명이 틀렸다.** GA 가 직접 만들 때만 그렇다.
⚠️ **리젠이 가려주고 있었을 뿐 처음부터 회복이 안 되고 있었다.** 리젠을 끄고 나서야 드러났다.

**수정** — `ApplyDamageEffect` 에 `SourceAbility` 인자 추가(기본값 `nullptr`) + `Context.SetAbility(SourceAbility)`. 호출 3곳(`MeleeTrace:216` · `AreaBlast:160` · `ShotBlast:223`)에서 `this` 전달.

📌 `SetAbility(nullptr)` 은 `if (InGameplayAbility)` 로 감싸져 있어 무동작 — 기본값이 안전하다.
📌 **컨텍스트를 채우는 건 스태미나 전용이 아니다.** "이 데미지 어느 공격에서 왔나"는 무기별 반응·통계에서 또 필요해진다.
⚠️ **`GetAbility()` 는 복제를 안 탄다**(`AbilityInstanceNotReplicated`). 멀티 전환 시 GE AssetTag 방식으로 교체 필요.

### 결과

```
평타 명중   피해 × 0.2 회복
스킬        10 소모 · 회복 0
회피·질주   소모 0 — GE_StaminaCost_Dodge · GE_FullSprintStaminaCost 는 참조자 0 (고아)
```

**스킬을 쓰려면 먼저 평타를 맞혀야 한다.** 승환이 의도한 공방 리듬.

---

## 4. 검증

PIE 통과 (승환).

```
✅ 스킬 중 피격해도 안 끊긴다 · 데미지는 들어간다
✅ Skill_02 VFX·슬로우·쉐이크·카메라 4종 동시
✅ Skill_03 차지 단계별 색 + 줌 + FOV · 릴리즈 시 복귀
✅ 평타 명중 시 스태미나 회복 · 스킬은 회복 X
✅ 리젠 없이도 스태미나 순환
```

## 5. 남은 것

- `KD.StaminaGainRate` **0.2 로 확정** (승환) — 코드 기본값과 동일해 수정 불필요
- 취소 시 카메라 줌 잔류 — 재현 확인 후 판단
- 정리 대상 — `NS_SB_Charge_02/_03` · `AM_SB_Skill_03_Start/_Loop/_End` 고아 3개 · `GE_StaminaCost_Dodge` · `GE_FullSprintStaminaCost` 고아 2개
- `KDGameplayTags.h:95` 주석 정정 — `StaminaComponent` 는 실재하지 않는다
- Vertex Shake (머티리얼 WPO) · 무기 IK 스냅 — SB 조사 후보 1·2순위
- Skill_01 · 04 카메라 연출 — 현행 유지 (승환 판단)

## 커밋

```
코드     7567920  차지 단계 GameplayCue 발신
         a316cae  스태미나 리젠 제거 - 평타 명중 회복
         a2cc24c  데미지 Context 에 출처 어빌리티 기록
Content  ea7e294  Skill_03 차지 카메라 줌
         298cf68  Skill_03 차지 카메라 FOV
         4073a16  스킬 슈퍼아머
         b873162  BP_PlayerState 에서 GE_StaminaRegen 제거
```
