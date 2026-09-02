# PROJECT_KD — 프로젝트 전체 개관 (AI 세션 부트 문서)

> **이 문서의 목적**: 새 세션 시작 시 AI가 이 문서 하나만 읽고 프로젝트 전체(현재 방향·코드 구조·에셋 위치)를 빠르게 파악하게 하는 단일 진입점. 매번 소스를 재스캔하지 않기 위한 것.
>
> **최종 갱신: 2026-09-02** (소스 전수 실측 — 클래스 98개·줄수·태그·에셋 폴더)
>
> ### 🔴 갱신 규칙 — 이 문서는 두 번 낡았다
>
> | 갱신 대상 | 언제 |
> |---|---|
> | **§2 코드 아키텍처** | **새 `.h` 파일을 만들거나 클래스를 지우거나 개명할 때.** 기능 추가만으로는 안 건다 |
> | **§4 에셋 맵** | 폴더를 새로 파거나 재편할 때 |
> | **§5 현재 지점** | 건드리지 않는다 — `handoffs/CURRENT.md` 로 위임했다 |
>
> ⚠️ **"바뀌면 갱신"은 작동하지 않았다.** 07-22~09-02 사이 dev-log 52개가 쌓이는 동안 이 문서는 실질 갱신 3회였고, 08-26 클래스 68개 개명이 통째로 누락돼 **클래스명 40개가 6주간 거짓**이었다. 그래서 위 표는 "판단"이 아니라 **파일 조작(생성·삭제·개명)** 을 방아쇠로 삼는다.
>
> 세부 진행상황은 `handoffs/CURRENT.md`와 `docs/dev-logs/`가, 룰은 `CLAUDE.md`가 담당한다. 이 문서는 **"지도"** 다.

---

## 0. 한 줄 정체성

**Project_KD** = 총과 칼을 동시에 쓰는 **지상 중심 스텔라 블레이드(SB) 스타일 3인칭 액션**. UE5.6 + GAS, 1인 개발, 마네킹(Manny)으로 선(先)개발 후 스킨 교체. 코어 = **패링 긴장감 + 콤보 타격감**.

- **엔진 소스 참조**: `D:\epicStore\UE_5.6\Engine\Source`
- **VCS**: Git 로컬 이중구조 — 코드 repo(Source/Config/docs만 추적, GitHub `Baekbanjang/ProjectBT` 비공개) + Content repo(`Content\.git`, 로컬 세이브포인트, 원격 푸시 금지, 13GB)
- **룰 단일 진실**: `CLAUDE.md`(프로젝트) + `~/.claude/CLAUDE.md`(전역, UE5+Karpathy 4원칙)
- **세션 2개 체제** (2026-08-24~): 코드·에셋 = 이 세션(KD) / 문서·지식 = 클라우디안(볼트) 세션. 통신은 볼트 `notes/_세션브릿지.md` 게시판

---

## 1. 피벗 이력 & stale 주의 ★먼저 읽을 것

세계관이 3번 바뀌었다. **최신(3번)만 유효**, 앞의 둘은 재료·참고용으로만 남음.

| # | 시기 | 코드네임 | 내용 | 상태 |
|---|---|---|---|---|
| 1 | ~2026-06 | **흑길동(killdong)** | 홍길동 한국무협, 도술·창(Spear)·산골도적 | ❌ stale (세계관·캐릭터·수치 무효) |
| 2 | 2026-06~07 | **버터(Butter)** | 트릭컬 팬게임 다크액션, 맨손 복싱 | ❌ stale (동결 보관) |
| 3 | **2026-07-21~** | **Gun & Sword** | 9CG 팩 기반 SB스타일 지상 액션 | ✅ **현재 유효** |

**현재 유효 문서 세트** = 이게 방향의 단일 진실:
- `docs/handoffs/CURRENT.md` ★★ (현재 상태·다음 할 일)
- `docs/handoffs/2026-07-21-pivot-gunsword.md`
- 볼트 `notes/Project_New/{_세션상태, 신규기획_GunSword_액션_v0, GunSword_콤보구성_v0, 피벗_SB스타일_액션_결정, 로드맵_GunSword_v1}.md`
- 볼트 `notes/Reference/StellarBlade_*` (밸런스/콤보 근거)

**stale 주의 (참조 시 폐기 컨텍스트 인지)**:
- ~~`docs/design/기획/` 8폴더~~ → 🗑️ **2026-08-24 삭제.** 볼트 `ProjectKD/notes/ProjectKD/기획/`에 최신 판본. 세계관·수치는 **무효**(길동 시절), 시스템 *설계 문법*만 참고 가치
- ~~`docs/design/butter-*.md`~~ → 🗑️ 버터 폐기 시 삭제. 락온 스트레이프 구현물 + 3D/셀셰이딩 파이프라인 지식노트만 보존
- ~~`docs/specs/deep-interview-abp-weapon.md`~~ → 🗄️ `docs/archive/kildong/`으로 이동. 단 `WeaponComponent`·`weapon_r` 소켓 결정은 현행 코드에 살아 있음
- ~~`docs/design/player-locomotion-system.md`~~ → 🗑️ 삭제. **현행은 `ABP_SB`** (`Content/SB_Style_GameProject/Animation/ABP_SB.uasset`)
- 볼트 `notes/ProjectKD/` → 🗄️ 동결. 길동 시대 원본
- **핵심**: 세계관은 3번 바뀌어도 **전투 코드(GAS/콤보/트레이스/락온/카메라)는 계속 이어져 재사용** 중

**Gun & Sword 팩 상태**: 애니팩(9CG, Fab, ~993클립) — `/Game/Gun_and_Sword`에 1069 uasset 임포트 완료. 설계 원칙 = "기획에 에셋 맞추기"가 아니라 **"보유 에셋에 기획 맞추기"**.

**1차 스코프**: 스테이지 1개(잡몹 5~10분) + 보스 1체. 마네킹으로 처음~끝 플레이 가능 + 지상콤보 2체인/공중미니콤보/사격/패링/저스트회피/처형 작동 + 보스 패턴 3개↑ 패링공략 + "패링 손맛" 체감.

---

## 2. 코드 아키텍처 (`Source/Project_KD/`)

**규모 (2026-09-02 실측)**: 파일 252개 = `.h` 103 + `.cpp` 99. `PROJECT_KD_API` 클래스 **98개**.

> ⚠️ **클래스 이름은 2026-08-26에 68개가 개명됐다.** `UGA_*` `UAS_*` `UWeaponComponent` 같은 이름은 **더 이상 없다.** 옛 이름을 쓴 문서를 만나면 그 문서 전체를 의심할 것.
> ```
> UGA_LightAttack  →  UKDGameplayAbility_LightAttack
> UAS_Combat       →  UKDCombatAttributeSet
> UWeaponComponent →  UKDWeaponComponent
> UAT_MeleeTrace   →  UKDAbilityTask_MeleeTrace
> UANS_MeleeTrace  →  UKDAnimNotifyState_MeleeTrace
> ```
> 근거 = `dev-logs/2026-08-26-class-naming-convention.md`. 규칙 = `GA_`/`GE_`/`AS_`는 **에셋 이름 규칙**이지 C++ 클래스 규칙이 아니다(엔진도 `UGameplayAbility`처럼 타입 접두를 안 쓴다).

### 2-1. 시스템 맵 (폴더별)

**Character/**
- `AKDBaseCharacter` (Abstract) — `ACharacter`+`IAbilitySystemInterface`. Player/Enemy 공통 조상. ASC 포인터만 보유(생성은 하위가)

**Player/**
- `AKDPlayerCharacter` (454줄) — 플레이어 Pawn. 컴포넌트 13개 소유:
  ```
  CameraBoom(UKDSpringArmComponent) · FollowCamera · CameraDollySpline · AimDollySpline
  InputBuffer · AbilityInputComp · ComboComp · LockOnComponent · CombatStateComp
  WeaponComp + GunWeaponComp(같은 클래스 2인스턴스) · HitStopComp · SprintComp · MotionWarping
  ```
  `Try*` 계열은 전부 `AbilityInputComp`로 위임하는 한 줄 래퍼(BP 호환용)
- `UKDPlayerAbilityInputComponent` (490줄, ★300선 초과) — `Try*` 13개 본문 + 입력버퍼 소비 + 제자리 턴. 태그 기반 GA activate/cancel. `BeginPlay`에서 형제 컴포넌트를 `FindComponentByClass` 1회 조회
- `AKDPlayerController` (392줄) — Enhanced Input 바인딩, `Handle_*`가 캐릭터로 얇게 위임. `Handle_Move`만 `State.Combat.MovementCanCancel`을 직접 읽어 이동 캔슬(Controller가 GAS 만지는 유일 지점 — **예외이지 본이 아니다**)
- `AKDPlayerState` — **플레이어 ASC 실소유자**. `UKDPlayerAttributeSet`+`UKDCombatAttributeSet` 생성. `GrantStartupAbilities()`(1회 가드) + `StartupEffects` 루프(`:40`)
- `UKDPlayerAnimInstance` — ABP 로코모션/상태변수(GroundSpeed, VelocityX/Y, TurnYawOffset, bIsLockedOn, bIsInBattleStance). ThreadSafe 업데이트
- `AKDPlayerCameraManager` — SB식 커브 카메라(FovByDistance/SlopeControl/FovPunch 3커브), `UpdateViewTarget` 오버라이드, `TriggerFovPunch`
- `UKDSpringArmComponent` (343줄) — `USpringArmComponent` 상속. **카메라 랙 버스트**(`RequestLagBurst(Scale/MaxDist/Duration/BlendSpeed)`) 소유. 요청은 주문서만 놓고 Tick의 `UpdateLagBurst`가 민다. 복구는 요청이 아니라 자동 — `Remaining`이 0을 지나면 `DefaultLagSpeed`로 스스로 갈아탄다

**Combat/**
- `UKDWeaponComponent` (281줄) — 무기 1개 표현. 플레이어가 **2인스턴스**(검/총) 보유해 다중무기. `bUseSheathing`으로 발검여부, `State.Combat.InCombat` 리스너로 자동 발검/납검(`Event.Combat.WeaponToggle` 발신). 스태틱/스켈레탈 겸용
- `UKDLockOnComponent` (552줄, ★★300선 대폭 초과 — **최우선 분리 후보**) — Sphere Overlap+`FKDTargetFilter`로 타겟검색(`IKDTargetableInterface` 경유, 캐스팅 X). Tick에서 카메라 보간. `State.Character.LockOn` 부여
- `UKDExecutionComponent` (303줄) — 처형 판정 사이클(경직+처형트리거 히트→invuln+GE+안전망타이머→데스블로/생존). `Event.Combat.Hit` 구독
- `UKDCombatStateComponent` (195줄) — `Team.Enemy` 주기스캔으로 `State.Combat.InCombat` 자동토글
- `UKDStaggerComponent` (211줄) — Poise 0→Stagger GE 적용/복귀 타이머
- `UKDKnockbackComponent` (131줄, 08-25 신설) — 넉백. `LaunchCharacter`(속도) → **RootMotionSource(거리 cm)** 로 전환. DA 필드 = `KnockbackDistance`
- `UKDHitStopComponent` (121줄) — **`CustomTimeDilation = 0`** 방식(`:49`). 액터 소유
- `UKDSlowMotionSubsystem` (118줄, 08-30 신설, `UWorldSubsystem`) — `RequestSlowMo(Scale, Duration, Priority)`. 우선순위로 중복 요청 조정
- `UKDHitFeedbackComponent` — BoneShake 타이밍만(순수 데이터), ABP가 읽어 뼈 오프셋
- `AKDProjectile` (Abstract) — 적/총 발사체, GE Spec 운반+`Event.Combat.Hit`(근접과 동일 계약)
- `Combat/Data/` — `UKDWeaponDataAsset`(메시+소켓+발검/납검몽타주) · `UKDLockOnConfig`(안에 `FKDTargetFilter`를 담는다) · `UKDHitConfirmProfile` · `FKDTargetFilter`(struct, `KDTargetFilter.h`) — Arc|Cylinder · 반각 · 높이 · 기준벡터 · 정렬 · bDrawDebug
- `UKDExecutionProfile` — 적별 처형 프로필(생존형 엘리트 vs 즉사 잡몹)

**AbilitySystem/Attributes/** (3세트 · 어트리뷰트 15개)
```
UKDCharacterAttributeSet   Health · MaxHealth · Poise · MaxPoise · Shield · MaxShield
 └ UKDPlayerAttributeSet   Stamina · MaxStamina · Ammo · MaxAmmo          (플레이어 전용)
UKDCombatAttributeSet      AttackPower · DamageReductionRate · IncomingDamage
                           ShieldAbsorbRate · BlockShieldAbsorbRate
```
- `UKDCombatAttributeSet` (278줄) — **메타어트리뷰트 `IncomingDamage`가 모든 데미지의 단일 게이트.** `PostGameplayEffectExecute`가 전방판정(±90°)→Perfect Parry/일반 Parry/적방어패링/일반피격 분기. **ExecCalc 0개** (SetByCaller 방식 — 아래 §2-5)
- ★ **`Defense`(뺄셈)는 2026-08-28에 삭제되고 `DamageReductionRate`(비율 0~1)로 바뀌었다.** 근거 = SB 덤프에 뺄셈식 방어력이 없다 + 뺄셈은 약한 공격일수록 손해가 커서 데미지 10짜리가 `Defense 10` 엘리트에겐 통째로 0이 된다
- **초기값이 어디서 오나** — 적은 `DA_EnemyDef_*` → `PossessedBy`. 플레이어는 **`GE_InitPlayerStats`** → `BP_PlayerState.StartupEffects` → `KDPlayerState.cpp:40` 루프. 생성자 값은 그 GE가 안 건드리는 어트리뷰트의 폴백
- `KDAttributeAccessors.h` — `ATTRIBUTE_ACCESSORS` 매크로 단일 정의처(엔진 미제공)
- ⚠️ **스태미나 소모처는 `UKDGameplayAbility_Skill` 하나뿐**(소스 전수 grep + `GA_Dodge`/`GA_HeavyCombo` uasset 스캔 0건). 회피·강공은 소모하지 않는다 — 스태미나 폐기 결정(2026-07-28) 이후 **스킬 코스트 역할로만** 살아 있다

**AbilitySystem/Abilities/ — GA 상속 트리** (폴더가 `Player/` `Enemy/`로 갈렸다)
```
UKDGameplayAbility (Abstract) — SafetyTimer + EndAbility→OnCleanup 공통화
├ UKDGameplayAbility_MeleeTrace (Abstract, 365줄) — 몽타주+TraceBegin/End→AbilityTask→DamageEffect
│  ├ UKDGameplayAbility_PlayerMelee (Abstract, 254줄)
│  │   히트스톱 · 슬로모 3칸(Scale/Duration/Priority) · 자동조준 · 접근워프 · HitConfirm
│  │  ├ UKDGameplayAbility_PlayerCombo (Abstract) — 콤보(ComboComponent)
│  │  │  ├ UKDGameplayAbility_LightAttack
│  │  │  └ UKDGameplayAbility_HeavyAttack
│  │  ├ UKDGameplayAbility_PlayerAirCombo → UKDGameplayAbility_AirLightAttack  (공중, 중력억제)
│  │  ├ UKDGameplayAbility_Skill        (09-02 신설, 단발 — 스킬 1·2·4)
│  │  │  └ UKDGameplayAbility_SkillCharge (09-02 신설, 차지 — 스킬 3)
│  │  ├ UKDGameplayAbility_CounterThrust  (퍼펙트 회피 반격)
│  │  │  └ UKDGameplayAbility_CounterSlash (08-28 신설, 퍼펙트 패링 반격)
│  │  └ UKDGameplayAbility_SprintAttack
│  └ UKDGameplayAbility_EnemyMeleeTrace (Abstract) → UKDGameplayAbility_EnemyRushAttack
├ UKDGameplayAbility_Dodge (320줄)   4방향 회피, Perfect판정, i-frame GE 분기
├ UKDGameplayAbility_Parry (175줄)   홀드가드, BlockStart→Loop→Hit→Loop 상태머신
├ UKDGameplayAbility_AimMode / _Shoot / _ShotBlast (324줄)   총 계열
├ UKDGameplayAbility_PlayerTurn / _PlayerExecution
└ UKDGameplayAbility_EnemyParry / _EnemyRangedAttack(Abstract) / _EnemyHitReact
```
자식 GA가 대체로 30줄 이하인 이유 = base가 복잡도 흡수(base는 의도적 예외).
※ 2026-08-26 `UGA_PlayerOneShotAttack` 층 삭제(멤버 0) — 자식 셋이 `PlayerMelee`로 올라왔다.

**AbilitySystem/Tasks/** — `UKDAbilityTask_MeleeTrace` (267줄): 소켓축 캡슐스윕(Sweep) 또는 팁 라인(TipLine) 2모드, SubStep 보간(터널링 방지), 액터당 1히트, LoS 차단. `ETraceMeshSource`(Weapon/OwnerBody)로 무기·맨몸 동일 태스크 재사용

**AbilitySystem/AnimNotifies/** (11개 · 히트판정=노티가 단일 진실)
```
State 7개   MeleeTrace(판정구간+per-window 오버라이드) · CancelWindow(CanCancel)
            EnemyAttackWindow(Perfect Dodge 판정) · TelegraphWindow(적 전조)
            MovementCancel · WindupSlow(적 윈드업 슬로우) · WeaponTrail(Niagara)
단발 4개    WeaponAttach(발검/납검, TargetWeaponTag) · PlayerCue · Shoot · ShotBlast
```

**AbilitySystem/GameplayCues/** (3개) — `AKDGameplayCueNotify_EnemyTelegraph`(부모태그 1개로 Parryable/Unblockable/Lethal 라우팅, 맵 기반 무증설) · `AKDGameplayCueNotify_ExecutionCamera` · `UKDGameplayCueNotify_HitImpact_Light`

**AbilitySystem/Effects/** (8개) — `UKDGameplayEffect_Damage_Physical`(Instant, IncomingDamage에 SetByCaller 가산 + Invulnerable/Dead 리젝컴포넌트) → `_Damage_Unblockable`(자식, 패링스킵). `_ParryWindow` / `_PerfectParryWindow` / `_DodgeInvincible` / `_Stagger` / `_StaminaCost`(SetByCaller) / `_AmmoCost`(ScalableFloat 고정 -1)

**AbilitySystem/Context/** — `FKDGameplayEffectContext` (08-26 신설): 커스텀 EffectContext. `PoiseMultiplier`·`DamageMultiplier`·`KnockbackMultiplier` 운반
**AbilitySystem/Globals/** — `UKDAbilitySystemGlobals`: ASC가 우리 Context를 만들게 한다. 배선 = `DefaultGame.ini:12` `AbilitySystemGlobalsClassName` (⚠️ **문자열 경로라 ClassRedirects가 안 먹는다**)
**AbilitySystem/Combo/** — `UKDComboComponent`(226줄, 입력시퀀스 누적→정확/prefix 매칭, 템포커브 배속) · `UKDComboTreeDataAsset`(`FComboBranch` 배열, `EComboContext` Ground/Air 분리, 노드에 `DamageMultiplier`·`PoiseMultiplier`)
**AbilitySystem/Library/** — `UKDAbilityStatics` (187줄): `GetMuzzleTransform` / `SpawnDamageProjectile` / `IsFriendlyFire` / **`ApplyDamageEffect`**(데미지 적용 공통화) / `SendHitEvent`

**Enemy/ + Enemy/AI/**
- `AKDEnemyBaseCharacter` (Abstract, **888줄 ★최대 초과**) — ASC를 **Pawn-direct 소유**(Player의 PlayerState 방식과 대비). HitFeedback/Stagger/Execution/Knockback 컴포넌트 델리게이트 연결. 브레인정지/랙돌/넉백/경직·처형 액추에이션 밀집
- `AKDEnemyAIController` (246줄) — Perception 타겟인지+시야기억, CrowdFollowing
- `UKDEnemyAnimInstance` · `UKDEnemyDefinitionDataAsset`(적1종 스탯/AI/공격셋 data-driven) · `UKDEncounterSubsystem`(WorldSubsystem, 동시공격 토큰)
- `AI/` — EQC 2개(포위대형/타겟) · Service 3개(FindPlayer/RequestAttackToken/TryParry) · Task 4개(ActivateAbilityByTag/FindKitingLocation/**FindPatrolPoint**/SelectAttack 거리밴드+가중치)

**UI/** `UKDEnemyStateBarWidget`(Abstract, 08-18 — 우리 첫 월드 스페이스 UI)
**Game/** `AKDGameModeBase`(얇은 배선) · **Input/** `UKDInputBufferComponent`(태그+타임스탬프 버퍼) · **Interface/** `IKDTargetableInterface`(CanBeTargeted/OnTargeted/GetLockOnPoint) · **Library/** `UKDCinematicLibrary` · **Movement/** `UKDSprintComponent`(Walk/Jog/Sprint/FullSprint 4단)

### 2-2. 핵심 전투 흐름

- **콤보**: `Try*Attack`→태그 GA activate 실패시 InputBuffer.Push. `Tick`이 Attacking/CanCancel 조합 보고 소비시점 결정. GA가 `ComboComponent.ProcessInput`→정확매칭시 브랜치 몽타주/GE 교체, 진행중이면 `DefaultAttackMontages[len-1]`. `ANS_CancelWindow`가 회복구간 CanCancel 부여. 템포 = 입력간격 커브→PlayRate
- **락온**: `FindBestTarget`(Overlap→인터페이스자격→`FKDTargetFilter`→LoS→정렬)→`EngageLockOn`(LockOn 태그+위젯+인터페이스 알림)→Tick 카메라 보간추적. 필터는 `DA_LockOnConfig_Default` 안에 들어 있다(**DA가 필터를 담는 그릇**)
- **패링**: 홀드. 진입 시 Block GE + `GE_PerfectParry` 동시. `UKDCombatAttributeSet::PostGEExec`가 전방판정으로 Perfect(0뎀+이벤트)/일반 분기. 일반 막기는 **Shield를 소모**하고(`BlockShieldAbsorbRate`), Shield 0이면 **가드 붕괴**(08-30)
- **닷지**: 4방향. Perfect판정 = 반경내 적 ASC `EnemyAttackHitWindow` 태그 / 발사체 `IsPerfectDodgeable()` 조회. Perfect = 긴 i-frame + `CounterReady`
- **반격 2갈래** (08-28): `State.Combat.CounterReady`를 **퍼펙트 회피**(`GA_Dodge`)와 **퍼펙트 패링**(`GE_PerfectParry_Counter`)이 공유한다. 파생기는 둘 — `CounterThrust`(약) / `CounterSlash`(강, `TryHeavyAttack`의 CounterReady 분기). ⚠️ **태그를 공유하므로 회피도 패링 반격을 쓸 수 있다.** 나눌지는 보류(`CURRENT.md` 참조)
- **처형**: ExecutionComponent가 `Event.Combat.Hit` 구독→Staggered+트리거태그 매칭시 invuln+콜리전무시+데스블로판정+`Event.Combat.ExecutionStarted`를 플레이어에 전송(→처형 GA 자동발동). 안전망 타이머. 데스블로는 `SetNumericAttributeBase(Health,0)` 코드강제사망
- **무기 발검/납검**: `WeaponComponent.OnInCombatTagChanged`→`Event.Combat.WeaponToggle` 발신(다중무기 시 1개만 브로드캐스트)→BP GA/애님이 Draw/SheathMontage→몽타주의 `AnimNotify_WeaponAttach`가 실제 소켓 부착
- **트레이스/데미지**: `GA_MeleeTrace` 몽타주+`Event.Montage.TraceBegin/End`→`AbilityTask_MeleeTrace`. 히트시 팀태그 아군방지→`UKDAbilityStatics::ApplyDamageEffect`(SetByCaller `AttackPower` + Context에 배수 3종)→`PostGEExec`(유일 데미지게이트)→`Event.Combat.Hit` 브로드캐스트로 피격측 각자 반응

### 2-3. GameplayTags (`KDGameplayTags.h`, 총 **108개**)

| 네임스페이스 | 개수 | 비고 |
|---|---|---|
| `Ability` | 30 | `Player.{Light,Heavy,Dodge,Parry,SprintAttack,CounterThrust,CounterSlash,AirCombo,Execution,Aim,Shoot,ShotBlast,Skill1~4}` · `Movement.Turn` · `Enemy.{Grunt,Elite,Boss}.Attack.*` · `Combat.Unblockable` |
| `State` | 27 | `Combat.{InCombat,InAction,Attacking,Dodging,Invulnerable,CanCancel,MovementCanCancel,Parrying,PerfectParryReady,EnemyAttackHitWindow,CounterReady,AirComboLock,Aiming,Shooting,Staggered,SuperArmor,...}` · `Character.LockOn` · `Movement.{Turning,InAir}` · `Dead` · `Camera.Cinematic` |
| `GameplayCue` | 15 | `Combat.HitImpact.Light` · `Enemy.Telegraph.{Parryable,Unblockable,Lethal}` · `Camera.*` |
| `Event` | 15 | `Montage.{TraceBegin,TraceEnd}` · `Rush.Warp` · `Skill.HoldRelease` · `Combat.{Hit,HitReact,WeaponToggle,ExecutionStarted,PerfectParryTriggered,ParrySuccess}` |
| `Cooldown` | 12 | `Player.Skill`(스킬 4종 **공유**) · `Enemy.*` |
| `Input` | 6 | `Action.{Light,Heavy,Dodge,Parry}` · `Combo.{Light,Heavy}` |
| `SetByCaller` | 2 | AttackPower · Stamina |
| `Team` | 1 | Enemy |

- ★ 2026-08-17 **`Ability.Mugong.*` → `Ability.Player.*` 개명**(길동 컨셉 폐기). `Config/DefaultGameplayTags.ini`의 `+GameplayTagRedirects` 11줄이 다리였으나 **에셋은 08-26 전수 재저장으로 이미 새 이름이 구워져 있다**(08-28 바이너리 실측 Mugong 0건). 그 줄들은 지워도 되나 급하지 않다
- ⚠️ **태그 문자열을 키로 들고 있는 에셋** = `KDEnemyDefinitionDataAsset.PoiseDamageByAttack` (`TMap<어빌리티 태그, float>`). **`Skill1~4`와 `CounterSlash` 항목이 없다** = 그 공격들은 경직을 못 건다(조회 실패 = 값 없음 = Poise 0, 에러 없음). 적 밸런싱 때 채울 것

### 2-4. 아키텍처 규칙 준수 현황 (2026-09-02 실측)

- **ASC 초기화**: Player=`PossessedBy`에서 `InitAbilityActorInfo(PS, this)`, Enemy=`(this, this)`. `OnRep_PlayerState` 미구현(싱글 전제, 의도적)
- **의존성 방향**: 대체로 준수. Component→구체 Pawn 캐스팅 없음(ASC 태그/인터페이스 경유). GA→Component는 `UKDGameplayAbility`의 접근자로 통일(`AbilitySystem/` 폴더에서 `KDPlayerCharacter.h` include 0개). 예외 = `Handle_Move`가 GAS 태그 직접 읽음
- **GC 개수**: C++ 3개 + BP GCN 다수 — `GAS/GC/` 폴더 16 uasset. 5~10개 룰의 상단
- **GE 자식 CDO**: `CreateDefaultSubobject`+`GEComponents.Add` 패턴 준수(§2-7 fatal 회피)

**줄수 한도 대비 (`.h`+`.cpp` 합산)**

| 클래스 | 줄수 | 한도 | 판정 |
|---|---|---|---|
| `AKDEnemyBaseCharacter` | **888** | Pawn 500 | 🔴 최대 초과 — 분리 최우선 |
| `UKDLockOnComponent` | **552** | Component 300 | 🔴 초과폭이 08-27 대비 더 커졌다 |
| `UKDPlayerAbilityInputComponent` | **490** | Component 300 | 🔴 초과 |
| `AKDPlayerCharacter` | 454 | Pawn 500 | 🟢 한도 내 |
| `AKDPlayerController` | 392 | — | 🟡 |
| `UKDGameplayAbility_MeleeTrace` | 365 | GA 200 | ⚪ base라 의도적 예외 |
| `UKDSpringArmComponent` | 343 | Component 300 | 🟡 소폭 |
| `UKDGameplayAbility_ShotBlast` | 324 | GA 200 | 🔴 초과 |
| `UKDGameplayAbility_Dodge` | 320 | GA 200 | 🔴 초과 |
| `UKDExecutionComponent` | 303 | Component 300 | 🟢 **해소됨** (08-25 분해로 306→303, 사실상 선상) |

✅ **해소된 것** — `AKDPlayerCharacter`(입력 분리, 08-12) · `UKDExecutionComponent` · `UKDGameplayAbility_Parry`(175줄).
⚠️ **새로 넘은 것** — `UKDSpringArmComponent`(08-31 랙 버스트) · `UKDGameplayAbility_ShotBlast`.

### 2-5. ★코드 주의/함정 (문서화된 설계 결정)

- **데미지는 ExecCalc가 아님** — SetByCaller + 메타어트리뷰트(`IncomingDamage`) 게이트웨이 방식. **새 데미지 로직은 `UKDCombatAttributeSet::PostGameplayEffectExecute`에 붙는다**
- **`PostGameplayEffectExecute` 사후로직 금지** — Health 0시 `HandleDeath`가 동기완료되므로 그 뒤 코드 추가 금지(주석 경고)
- **InstancedPerActor GA 멤버 잔류** — 핸들·`DamageEffectClass` 등은 매 `ActivateAbility` 시작부에서 명시 리셋 필수
- **히트스톱 = `CustomTimeDilation = 0`** (`KDHitStopComponent.cpp:49`). ⚠️ **`Montage_Pause`는 `UKDGameplayAbility_SkillCharge`가 프로젝트 유일 사용처**(차지 중 정지) — 히트스톱을 몽타주 정지 방식으로 되돌리면 차지가 깨진다
- ★ **슬로모·히트스톱은 시계가 다르다** (08-31) — 만료 판정에 실시간(`GetRealTimeSeconds`, "IS NOT dilated")을 쓰고 알람에 게임 시계 타이머를 쓰면 **요청 길이의 `1/Scale` 배**로 지속된다. 양쪽 다 `실시간 초 × 현재 배율 = 게임 시계 초` 환산이 들어가 있다. **새로 타이머를 걸 때 같은 함정을 밟지 말 것**
- ★ **`SetByCaller` GE는 `CostGameplayEffectClass`로 못 쓴다** (09-02) — `CheckCost`가 CDO를 그대로 넘겨 매그니튜드가 0으로 계산된다(`GameplayEffect.cpp:5181`). `CheckCost`/`ApplyCost` 두 함수를 오버라이드해 우회한다(`UKDGameplayAbility_Skill`). 정상 비교군 = `_AmmoCost`(`FScalableFloat` 고정값)
- ★ **어빌리티에는 Tick이 없다** — "나중에 이걸 해라"는 타이머나 노티로 예약. 부모의 안전망 타이머를 끄면 **대신할 것을 반드시 걸 것**(필요 없어서 끄는 게 아니라 길이가 안 맞아서 갈아끼우는 것)
- ★ **모션 워핑 타겟은 조기 return에 살아남는다** — 발동 시작에 `RemoveWarpTarget`을 안 하면 **옛 좌표로 워프**한다(08-20 접근워프 / 08-31 반격 두 번 다 같은 뿌리)
- **워프 회전 규칙** — 타겟을 `FromComponent`/`FromActor`로 넘기면 `RotationType = Facing`, `FromLocationAndRotation`으로 넘기면 `Default`. 현행 19개 전부 부합
- **처형 데미지 = 코드강제** — `FinishExecution`이 데스블로시 `SetNumericAttributeBase(Health,0)`. GE 밸런스 무관
- **팀 이분법** — `Team.Enemy` 유무로만 아군방지. 팀 3개↑(소환수 등) 확장시 깨짐
- **`AnimNotify_WeaponAttach` TargetWeaponTag=None = 전무기 동시발검** — 다중무기 몽타주에서 특정무기만 부착하려면 태그 필수
- **새 근접 GA BP는 `CapsuleRadius`를 20으로** — 헤더 기본값이 3이라 안 올리면 판정이 거의 안 닿는다
- 🔴 **`KDGameplayAbility_MeleeTrace.h`가 `TipLine`인데 생성자가 `Sweep`으로 덮는다** — 헤더만 읽으면 오해한다. 3개월째 미정리

---

## 3. 기획 요약

### 3-1. 현재 방향 (Gun & Sword, ✅유효)
위 §1 참조. 콤보 설계 문법(`GunSword_콤보구성_v0.md`): 스타터 분리+후반타 풀 공유, 입력창 우상향, 저스트회피=메인풀 재합류/저스트패링=전용 카운터 격리, 건=무기전환 아닌 액션세트(콤보 중 Shoot 캔슬), 공중=독립 미니그래프, 스프린트 전용파생 없음.

**밸런스 기준 (SB 이브 실측 이식)**: 패링창 **0.15s** / 라이트어택 계수 0.7·1.0·1.0·1.8 / 히트판정 0.1s / 피격경직 0.3s / 입력버퍼 0.8s(`MaxBufferSize` 2) / 히트스톱 0.08s(SB 0.07, **공격자만**).

### 3-2. 시스템별 (현재 유효 vs stale)

| 시스템 | 상태 | 요약 |
|---|---|---|
| **락온** | ✅구현완료 | `UKDLockOnComponent` + `FKDTargetFilter`. DA가 필터를 담는 그릇. Radius 1000 |
| **카메라** | ✅구현완료 | SB식 FOV/Pitch/TurnSpeed 커브 + 랙 버스트(08-31) |
| **콜리전 채널** | ✅구현완료 | 무기중립 |
| **데미지** | ✅구현완료 | SetByCaller + `IncomingDamage` 게이트 + 비율 감소 + Shield 층 |
| **콤보** | ✅구현완료 | 지상 20타 + 공중. 노드에 `DamageMultiplier`/`PoiseMultiplier` |
| **패링·반격** | ✅1·2·4① 완료 | 창 0.15 / 반격 2갈래 / 막기가 Shield 소모 + 가드 붕괴. 잔여 = 3단계 연출·5단계 적 전조 |
| **처형** | ✅구현완료 | 적별 프로필(생존형/즉사) |
| **총** | ✅구현완료 | 조준 스탠스·히트스캔·발사체·머즐·크로스헤어 |
| **스킬** | ✅1차 완료 | **09-02.** 단발 3 + 차지 1. 잔여 = 스킬2 땅찍기 AoE · 연출 · UI |
| **HUD** | 🟡부분 | 좌하단 HP/ST/SH + 탄약 + 적 상태 바(월드) 완료. 잔여 = **우하단 스킬 슬롯** |
| **로코모션** | ✅유효 | MM(PSD) + `ABP_SB`. 가드 로코모션 08-30 추가 |
| **적·보스** | 🟡부분 | 잡몹 5종 + 더미 2종 DA. **보스 미착수.** 길동 적 기획은 ❌stale |
| **도술 / 공중QTE** | ❌stale | 길동 전용. 스킬 1~4가 도술 역할을 대체했다 |
| **분위기** | ❌stale | 길동 사극톤. GunSword 톤 문서 없음(SB 참조가 유일) |

### 3-3. 문서/볼트 지도
- **docs/design/**: 🟠 **참고용 보존 (2026-05 정지).** 5문서 전부 갱신이 없고 **지금은 거짓인 문장이 섞여 있다.** 읽을 것은 "왜 그렇게 골랐나"뿐. 거짓 목록 = `docs/INDEX.md §design`
- ✅ **설계 문서의 단일 진실 = 볼트 `notes/코드구조/`** (10문서). 락온·카메라·콜리전·데미지·로코모션 5주제 담당. **이 세션은 dev-log를 남기고, 볼트 세션이 그걸 읽어 반영한다**
- **docs/handoffs/**: `CURRENT.md`★★ · `2026-07-21-pivot-gunsword`✅ · `archive/`(지난 세션 기록)
- **옵시디언 볼트** (`C:\Users\asdasd\Desktop\Obsidian_organize\ProjectKD\`):
  - `notes/_세션브릿지.md` ★★ — **두 세션의 유일한 통신 수단**
  - `notes/Project_New/`✅ — GunSword 기획 + 로드맵 + 전투수치
  - `notes/Reference/StellarBlade_*`✅ — SB 이브 실측(JSON덤프/데이터시트/콤보트리 복원)
  - `notes/코드구조/`✅ · `notes/언리얼/`(학습·트러블슈팅) · `notes/ProjectKD/`🗄️동결

---

## 4. 콘텐츠 에셋 맵

### 4-1. ★현재 메인: `Content/SB_Style_GameProject/` (약 224 uasset)

| 폴더 | uasset | 내용 |
|---|---|---|
| **Animation** | 80 | `ABP_SB`(메인) · `Equip_AM/` · `TurnAbility_AM/` · `MotionMatching/{Schema,DataBase,Normalization}` · 몽타주 전체 |
| **GAS** | 59 | 아래 세분 |
| **Audio** | 37 | 발소리·총성·검 |
| **Input** | 17 | `IA_*` · `IMC_Default` (스킬 IA 4개 09-02 추가) |
| **UI** | 9 | `WBP_MainHUD` · `WBP_Crosshair` · `WBP_EnemyStateBar` · `WBP_LockOnReticle` · `DA_LockOnConfig_Default` |
| **Camera** | 6 | `CF_FovByCamDist` · `FovPunchCurve` · `TurnSpeedByVelocity` |
| **Player** | 5 | `BP_SBPlayer`(메인) · `BP_PlayerController/State` · `BP_KDPlayerCameraManager` |
| **WeaponData** | 4 | `DA_Weapon_{Sword,Gun,Spear,Bong}` |
| PostProcess 2 · Core 1 · Combat 1 · Effect 1 · ExecutionData 1 · Retargeter 1 | | |

**`GAS/Abilities/` (22개)** — 2026-09-02에 **폴더로 재편**(15개 이동):
```
Attack/            GA_LightCombo · GA_HeavyCombo · GA_AirLightAttack · GA_SprintAttack
Guard_and_Parry/   GA_Parry · GA_PerfectParryReaction · GA_CounterThrust · GA_CounterSlash
                   GA_GuardBreak · GA_HitReact
Move/              GA_Dodge · GA_Turn
Shot/              GA_AimMode · GA_Shoot · GA_ShotBlast
Skill/             GA_Skill_01 ~ 04
(루트)             GA_PlayerExecution · GA_ShieldDepleted · GA_WeaponToggle
```

**`GAS/GE/` (18개)**
```
Ammo/          GE_AmmoRegen · GE_GunRegenBlock
BlockAndParry/ GE_Block · GE_PerfectParry · GE_PerfectParry_Counter
Dodge/         GE_DodgeInvincible_Normal · _Perfect · GE_PerfectDodge_CounterThrust
Shield/        GE_ShieldRegen · GE_ShieldRegen_InCombat · GE_ShieldRecover
Stamina/       GE_StaminaRegen · GE_StaminaRegenBlock · GE_StaminaCost_Dodge · GE_FullSprintStaminaCost
Skill/         GE_SkillCooldown
(루트)         GE_AirComboLock · GE_InitPlayerStats
```
⚠️ `GE_StaminaCost_Dodge`·`GE_FullSprintStaminaCost`는 **참조가 확인되지 않았다**(GA_Dodge uasset 스캔 0건). 스태미나 폐기의 잔재일 가능성 — 정리 판단 대기

**`GAS/GC/` (16개)**
```
CameraShake/          LCS_Hit · LCS_CounterAttack · LCS_ParryClash · LCS_PerfectParry · LCS_ShootRecoil
ExcuteGCN/            GCN_ExcutionCamera · GCN_ShootRecoil
HitConfirmDataAsset/  DA_HitLightAttack · DA_HitCounterAttack
NiagaraEffect/        NS_PerfectDodge · NS_Temp_PefectDodge
(루트)                GCN_PerfectParry · GCN_PerfectDodge · GCN_CounterThrust
                      GCN_HitReact · GCN_PlayerHitConfirm · GCN_CameraLagBurst
```
**`GAS/ComboData/`** — `DA_ComboTree` · `DA_AirComboTree`

### 4-2. 애니 소스 팩: `Content/Gun_and_Sword/` (1069 uasset)
`Animations/Sequence/` 9개 번호 카테고리:

| 카테고리 | 개수 | 내용 |
|---|---|---|
| 01_Idle | 31 | Idle, Idle_Combat, Offset_Idle(Combat), Offset_Aim |
| 02_Attack | 96 | Combo_Attack_01~05(_Shoot), Air, Normal/Skill/Speed_Attack, Attack_Air_to_Floor, Run_Attack, Parry_Counter_Attack, Dash_Air_Attack, Execution |
| 03_Walk | 292 | Walk/Combat/Block/Aim/Equip, 방향세분(F_0/L_45/R_90/B_180…) |
| 04_Run | 288 | Run/Combat/Aim/Equip, 방향세분 |
| 05_Jump | 42 | Jump/Combat, Double_Jump |
| 06_Dodge | 100 | Dodge/Combat/Air, Dodge_to_Run |
| 07_Roll | 68 | Roll/Combat, Roll_to_Run |
| 08_Hit | 59 | Hit/Combat/Air/Large, Get_Up, Knock_Down, Block |
| 09_Turn | 16 | Turn/Combat/Block/Aim |

- **RM vs IP 분리**: 방향성 이동류(03/04/06)의 각 세부폴더에 `01_RM`(루트모션)/`02_IP`(인플레이스). 콤보류·Idle·Jump·Hit·Turn은 리프에 `AS_*_Seq` 직접
- **스켈레톤/메시** (`Demo/Characters/Mannequins/Meshes/`): `SKM_Manny_GunSword`/`SK_Mannequin_GunSword`, 무기 `Gun.uasset`/`Sword.uasset`(스태틱메시)
- **미사용 스킬 애니 재고**: `Skill_01`(Start40/Loop40/Shoot90) · `Skill_02` 105f · `Skill_04` 155f · `Skill_05`(28x3 반복) · `Aim_the_Target`(이동사격). ※`Skill_03`·`Speed_Attack`은 09-02에 사용

### 4-3. 기타 팩 (참고)
버터잔재 `TrickalFanGame/`(stale) · `Map/`(버터맵) · 적애니 `BossAnimationsPack`/`BossyEnemyAnimPack`(자체스켈레톤) · `RamsterZ_FreeAnims_Volume1` · `WM_Female_Bundle`(Lucy) · `MotionMatchingAnimation`(Epic MM샘플) · VFX `SlashTrailElemental`/`Vefects` · SFX `MetalandWoodMeleeWeaponsSFXPack` · `killdong_ExecutionTest` · `Robot3/`(미추적) · `_ThirdParty/`

### 4-4. 자주 쓰는 경로 빠른참조
| 찾는 것 | 경로 |
|---|---|
| 메인 플레이어 BP | `SB_Style_GameProject/Player/BP_SBPlayer` |
| 무기 데이터 | `SB_Style_GameProject/WeaponData/DA_Weapon_*` |
| 콤보 트리 | `SB_Style_GameProject/GAS/ComboData/DA_ComboTree`·`DA_AirComboTree` |
| GA 전체 | `SB_Style_GameProject/GAS/Abilities/{Attack,Guard_and_Parry,Move,Shot,Skill}/` |
| ABP | `SB_Style_GameProject/Animation/ABP_SB` |
| MM 스키마/DB/정규화 | `SB_Style_GameProject/Animation/MotionMatching/{Schema,DataBase,Normalization}/` |
| 공격 몽타주 소스 | `Gun_and_Sword/Animations/Sequence/02_Attack/` |
| 이동 소스(RM/IP) | `Gun_and_Sword/Animations/Sequence/{03_Walk,04_Run,06_Dodge}/.../01_RM\|02_IP` |
| GS 스켈레탈메시 | `Gun_and_Sword/Demo/Characters/Mannequins/Meshes/SK_Mannequin_GunSword` |

---

## 5. 현재 지점

> 🔴 **여기에 진행상황을 쓰지 않는다.** 07-22~09-02 사이 이 절이 6주간 거짓이었던 자리다.
>
> ### 👉 **[`docs/handoffs/CURRENT.md`](handoffs/CURRENT.md)** 를 읽을 것 — 진행상황·다음 할 일·보류·설계 미결 전부 거기 있다.

한 줄 요약만 둔다 (2026-09-02 기준): **캐릭터 스킬 4종 GA 완료, PIE 통과. 다음 = 스킬2 땅찍기 AoE → 스킬 연출 → 우하단 스킬 UI.**

---

## 6. 세션 시작 프로토콜 (AI)

`CLAUDE.md §0` 이 원문이고 여기는 요약이다.

1. `CLAUDE.md`(프로젝트) + `~/.claude/CLAUDE.md`(전역)
2. **이 문서** — 전체 그림(지도)
3. `docs/INDEX.md` — 이번 작업에 뭘 더 읽을지 고른다
4. **`docs/handoffs/CURRENT.md`** ★★ — 현재 지점
5. **볼트 `notes/_세션브릿지.md`** ★★ — 상대 세션과의 역할 구분·대기 항목
6. 이번 작업 영역의 최신 dev-log
7. `git status` + `git log`

**작업 규칙 리마인더**
- **`.h`/`.cpp` = 작성자가 직접 입력.** AI는 설계·가이드까지 (서브에이전트도 동일)
- 에디터/MCP 에셋 작업은 **승환이 명시 요청할 때만**(조회는 OK). MCP 에셋 작업 전 **Content 커밋 의무**
- **판정 권한**: 코드·에셋·빌드·PIE = 이 세션 / 노트·문서 교차대조 = 볼트 세션. **못 보는 쪽은 단정하지 않는다**
- 기능 완료시 `docs/dev-logs/{날짜}-{기능}.md` + `docs/INDEX.md` 등록. **새 클래스를 만들거나 지웠으면 이 문서 §2도 같이**
