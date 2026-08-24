# PROJECT_KD — 프로젝트 전체 개관 (AI 세션 부트 문서)

> **이 문서의 목적**: 새 세션 시작 시 AI가 이 문서 하나만 읽고 프로젝트 전체(현재 방향·코드 구조·에셋 위치)를 빠르게 파악하게 하는 단일 진입점. 매번 소스를 재스캔하지 않기 위한 것.
> **갱신 규칙**: 아키텍처·방향·주요 에셋 위치가 바뀌면 이 문서를 갱신한다(정확성이 생명 — 낡으면 오히려 독). 세부 진행상황은 `docs/dev-logs/`와 메모리가, 룰은 `CLAUDE.md`가 담당. 이 문서는 "지도".
> **최종 작성: 2026-07-22** (코드/기획/에셋 3레인 전수 파악 기반)

---

## 0. 한 줄 정체성

**Project_KD** = 총과 칼을 동시에 쓰는 **지상 중심 스텔라 블레이드(SB) 스타일 3인칭 액션**. UE5.6 + GAS, 1인 개발, 마네킹(Manny)으로 선(先)개발 후 스킨 교체. 코어 = **패링 긴장감 + 콤보 타격감**.

- **엔진 소스 참조**: `D:\epicStore\UE_5.6\Engine\Source`
- **VCS**: Git 로컬 이중구조 — 코드 repo(Source/Config/docs만 추적, GitHub `Baekbanjang/ProjectBT` 비공개) + Content repo(`Content\.git`, 로컬 세이브포인트, 원격 푸시 금지, 13GB)
- **룰 단일 진실**: `CLAUDE.md`(프로젝트) + `~/.claude/CLAUDE.md`(전역, UE5+Karpathy 4원칙)

---

## 1. 피벗 이력 & stale 주의 ★먼저 읽을 것

세계관이 3번 바뀌었다. **최신(3번)만 유효**, 앞의 둘은 재료·참고용으로만 남음.

| # | 시기 | 코드네임 | 내용 | 상태 |
|---|---|---|---|---|
| 1 | ~2026-06 | **흑길동(killdong)** | 홍길동 한국무협, 도술·창(Spear)·산골도적 | ❌ stale (세계관·캐릭터·수치 무효) |
| 2 | 2026-06~07 | **버터(Butter)** | 트릭컬 팬게임 다크액션, 맨손 복싱 | ❌ stale (동결 보관) |
| 3 | **2026-07-21~** | **Gun & Sword** | 9CG 팩 기반 SB스타일 지상 액션 | ✅ **현재 유효** |

**현재 유효 문서 세트(6개)** = 이게 방향의 단일 진실:
- `docs/handoffs/2026-07-21-pivot-gunsword.md`
- 볼트 `notes/Project_New/{_세션상태, 신규기획_GunSword_액션_v0, GunSword_콤보구성_v0, 피벗_SB스타일_액션_결정}.md`
- 볼트 `notes/Reference/StellarBlade_*` 3파일 (밸런스/콤보 근거)

**stale 주의 (참조 시 폐기 컨텍스트 인지)**:
- ~~`docs/design/기획/` 8폴더~~ → 🗑️ **2026-08-24 삭제.** 볼트 `ProjectKD/notes/ProjectKD/기획/`에 더 최신 판본이 있어 이쪽을 정리했다. 세계관·수치는 **무효**(길동 시절), 시스템 *설계 문법*(SB 정합 콤보트리, 색상 신호 규약, GAS 모듈 공유)만 참고 가치 — **볼트에서 읽을 것.**
- `docs/design/butter-*.md` (roadmap/skills/anim-pipeline) → 버터 폐기. 단 락온 스트레이프 구현물 + 3D/셀셰이딩 파이프라인 지식노트는 보존.
- ~~`docs/specs/deep-interview-abp-weapon.md`~~ → 🗄️ **2026-08-24 `docs/archive/kildong/`으로 이동.** 창(Spear)+길동 스켈레톤 전제. 단 `WeaponComponent`·`weapon_r` 소켓 결정은 현행 코드에 살아 있음.
- ~~`docs/design/player-locomotion-system.md`~~ → 🗑️ **2026-08-24 삭제.** 문서 전체가 길동 `ABP_Player` 기준이었다. **현행은 `ABP_SB`** (`Content/SB_Style_GameProject/Animation/ABP_SB.uasset` — `Gun_and_Sword` 애니팩 참조, killdong 0건). 문서에 있던 SB BlendSpace 실측(`IdleRun_BS_Peaceful2D`·`LockOn_IdleRun_BS`·`SBVelocityX`)은 볼트 `notes/Reference/StellarBlade_로코모션_BlendSpace_실측.md`로 건져 보존.
- 볼트 `notes/ProjectKD/SESSION_STATE.md` → 파일 상단에 "2026-07-12부터 미갱신, 현행은 Project_New/_세션상태.md" 경고 있음.
- **핵심**: 세계관은 3번 바뀌었어도 **전투 코드(GAS/콤보/트레이스/락온/카메라)는 계속 이어져 재사용** 중. "전투/GAS는 재사용 자산" 원칙.

**Gun & Sword 팩 상태**: 애니팩(9CG, Fab, ~993클립) — 메모리엔 "미구매"로 기록됐으나 **실제 `/Game/Gun_and_Sword`에 1069 uasset 존재**(이미 임포트됨). 설계 원칙 = "기획에 에셋 맞추기"가 아니라 **"보유 에셋에 기획 맞추기"**.

**1차 스코프**: 스테이지 1개(잡몹 5~10분) + 보스 1체. 마네킹으로 처음~끝 플레이 가능 + 지상콤보 2체인/공중미니콤보/사격/패링/저스트회피/처형 작동 + 보스 패턴 3개↑ 패링공략 + "패링 손맛" 체감.

**밸런스 초깃값 (SB 이브 실측 이식)**: 패링창 0.1s(스토리 0.13) / 라이트어택 계수 0.7·1.0·1.0·1.8 / 히트판정 0.1s / 피격경직 0.3s / 입력창 1~2타 0.75s→후반 1.2s+.

---

## 2. 코드 아키텍처 (`Source/Project_KD/`, 169 파일)

### 2-1. 시스템 맵 (폴더별)

**Character/**
- `ABaseCharacter` (Abstract) — `ACharacter`+`IAbilitySystemInterface`. Player/Enemy 공통 조상. ASC 포인터만 보유(생성은 하위가).

**Player/**
- `AKDPlayerCharacter` (426줄, 2026-08-12 리팩토링으로 500줄 한도 해소) — 플레이어 Pawn. 컴포넌트 다수 소유(CameraBoom/FollowCamera/InputBuffer/**WeaponComp+GunWeaponComp**/ComboComp/LockOnComponent/CombatStateComp/SprintComp/MotionWarping/**AbilityInputComponent**). 남은 `Try{Light,Heavy}Attack/TryDodge/TryParry/TryExecute`는 `AbilityInputComponent`로 위임하는 한 줄 래퍼(BP 호환용).
- `UKDPlayerAbilityInputComponent` (`.cpp` 352줄) — `Try*` 8개 본문 + 입력버퍼 소비 + 제자리 턴 실제 구현. 태그 기반 GA activate/cancel. `TickComponent`에서 InputBuffer 소비(공중/지상/캔슬윈도우 분기). `BeginPlay`에서 형제 컴포넌트 4개(`InputBuffer`/`ComboComp`/`LockOnComp`/`SprintComp`) `FindComponentByClass` 1회 조회.
- `AKDPlayerController` — Enhanced Input 바인딩, `Handle_*`가 캐릭터로 얇게 위임. `Handle_Move`만 `State.Combat.MovementCanCancel` 읽어 이동 캔슬(Controller가 GAS 만지는 유일 지점).
- `AKDPlayerState` — **플레이어 ASC 실소유자**. `UAS_Player`+`UAS_Combat` 생성. `GrantStartupAbilities()`(1회 가드).
- `UKDPlayerAnimInstance` — ABP 로코모션/상태변수(GroundSpeed, VelocityX/Y, TurnYawOffset, bIsLockedOn, bIsInBattleStance). ThreadSafe 업데이트.
- `AKDPlayerCameraManager` — SB식 커브 카메라(FovByDistance/SlopeControl/FovPunch 3커브), `UpdateViewTarget` 오버라이드, `TriggerFovPunch` 히트연출.

**Combat/**
- `UWeaponComponent` (219줄) — 무기 1개 표현. 플레이어가 **2인스턴스**(검=WeaponComp / 총=GunWeaponComp) 보유해 다중무기. `bUseSheathing`으로 발검여부, `State.Combat.InCombat` 리스너로 자동 발검/납검(`Event.Combat.WeaponToggle` 발신). 스태틱/스켈레탈 겸용, GripPoint 자동 역보정.
- `UComboComponent` (211줄) — 입력시퀀스 누적→ComboTree `FComboBranch` 정확/prefix 매칭. 템포커브로 배속.
- `ULockOnComponent` (330줄, ★소폭초과) — Sphere Overlap+시야콘+LoS로 타겟검색(`IKDTargetableInterface` 경유, 캐스팅 X). Tick에서 카메라 보간. `State.Character.LockOn` 부여.
- `UCombatStateComponent` (195줄) — `Team.Enemy` 주기스캔으로 `State.Combat.InCombat` 자동토글.
- `UExecutionComponent` (306줄, ★소폭초과) — 처형 판정 사이클(경직+처형트리거 히트→invuln+GE+안전망타이머→데스블로/생존). `Event.Combat.Hit` 구독. Pawn엔 델리게이트 위임.
- `UStaggerComponent` (211줄) — Poise 0→Stagger GE 적용/복귀 타이머. 델리게이트 분리.
- `UHitFeedbackComponent` — BoneShake 타이밍만(순수 데이터), ABP가 읽어 뼈 오프셋.
- `AKDProjectile` (Abstract) — 적 원거리 발사체, GE Spec 운반+`Event.Combat.Hit`(근접과 동일 계약).
- `Combat/Data/` — `UWeaponDataAsset`(메시+소켓+발검/납검몽타주), `ULockOnConfig`, `UHitConfirmProfile`, `ExecutionProfile`(적별 처형: 생존형 엘리트 vs 즉사 잡몹).

**AbilitySystem/Attributes/**
- `UAS_CharacterBase` — Health/MaxHealth/Poise/MaxPoise **+ Shield/MaxShield**(2026-08-18 추가), `PreAttributeChange` 클램프.
- `UAS_Combat` — AttackPower/Defense + **메타어트리뷰트 `IncomingDamage`(모든 데미지의 단일 게이트)**. `PostGameplayEffectExecute`가 전방판정(±90°)→Perfect Parry/일반 Parry/적방어패링/일반피격 전부 분기. ★**ExecCalc 미사용** — SetByCaller 방식(아래 2-5 주의).
- `UAS_Player` — Stamina/MaxStamina **+ Ammo/MaxAmmo** (플레이어 전용). `Dosul`(길동 잔재)은 2026-08-18 에 삭제하고 그 자리에 `Ammo` 를 넣었다. ⚠️Stamina 는 여전히 소모 GE 를 적용하는 GA 가 0곳 — 스태미나 폐기 결정(2026-07-28) = 이동 자원 폐기 / **스킬 코스트 역할로만 유지**.
- **초기값이 어디서 오나** — 적은 `DA_EnemyDef_*` → `PossessedBy`. 플레이어는 **`GE_InitPlayerStats`**(2026-08-22 신설) → `BP_PlayerState.StartupEffects` → `KDPlayerState.cpp:40` 루프. 생성자 값은 그 GE 가 안 건드리는 어트리뷰트의 폴백.
- `KDAttributeAccessors.h` — `ATTRIBUTE_ACCESSORS` 매크로 단일 정의처.

**AbilitySystem/Abilities/ — GA 상속 트리**
```
UGA_ActionBase (Abstract) — SafetyTimer + EndAbility→OnCleanup 공통화
 └ UGA_MeleeTraceBase (Abstract, 349줄) — 몽타주+TraceBegin/End→AT_MeleeTrace→DamageEffect 적용
    ├ UGA_PlayerMeleeAttackBase — 히트스탑(Montage_Pause 방식)
    │  ├ UGA_PlayerAttackBase — 콤보(ComboComponent) + HitConfirm + 락온 자동조준
    │  │  ├ UGA_LightAttack / UGA_HeavyAttack (Heavy는 스태미나 소모 추가)
    │  ├ UGA_PlayerOneShotAttack — 단발
    │  │  ├ UGA_SprintAttack / UGA_CounterThrust (모션워핑 대시)
    │  └ UGA_PlayerAirAttackBase → UGA_AirLightAttack (공중콤보, 중력억제)
    └ UGA_EnemyWeaponTraceBase → UGA_EnemyRushAttack (gap-closer)
UGA_Dodge (338줄, ★초과) — 4방향 회피, Perfect판정, Normal/Perfect i-frame GE 분기
UGA_Parry (232줄, ★소폭초과) — 홀드가드, BlockStart→Loop→Hit→Loop 상태머신
UGA_PlayerTurn / UGA_PlayerExecution / UGA_EnemyHitReact / UGA_EnemyParry / UGA_EnemyRangedAttack
```
자식 GA가 200줄 이하인 이유 = base가 복잡도 흡수(base는 의도적 예외).

**AbilitySystem/Tasks/** — `UAT_MeleeTrace`: 소켓축 캡슐스윕(Sweep) 또는 팁 라인(TipLine) 2모드, SubStep 보간(터널링 방지), 액터당 1히트, LoS 차단. **`ETraceMeshSource`(Weapon/OwnerBody)로 무기·맨몸 동일 태스크 재사용**(무기중립화, 2026-07-17 완료).

**AbilitySystem/AnimNotifies/** (히트판정=노티가 단일 진실)
`AN_WeaponAttach`(발검/납검, TargetWeaponTag) · `ANS_MeleeTrace`(판정구간+per-window 오버라이드) · `ANS_CancelWindow`(CanCancel) · `ANS_EnemyAttackWindow`(Perfect Dodge 판정) · `ANS_TelegraphWindow`(적 전조) · `ANS_MovementCancel` · `ANS_WindupSlow`(적 윈드업 슬로우) · `ANS_WeaponTrail`(Niagara) · `AN_PlayerCue`.

**AbilitySystem/GameplayCues/** — `AGCN_EnemyTelegraph`(부모태그 1개로 Parryable/Unblockable/Lethal 라우팅, 맵 기반 무증설) · `AGCN_ExecutionCamera` · `UGCN_HitImpact_Light`.

**AbilitySystem/Combo/** — `UComboTreeDataAsset`(`FComboBranch` 배열, `EComboContext` Ground/Air 분리).
**AbilitySystem/Library/** — `UKDAbilityStatics`: `GetMuzzleTransform`(무기 메시 소켓 → 캐릭터 메시 → 액터 순) / `SpawnDamageProjectile`. ※`TryConsumeStamina`는 **없다**(2026-08-17 실측 — 이 줄이 오래 stale이었음).
**AbilitySystem/Effects/** — `GE_Damage_Physical`(Instant, IncomingDamage에 SetByCaller 가산+Invulnerable/Dead 리젝컴포넌트)→`GE_Damage_Unblockable`(자식, 패링스킵). `GE_ParryWindow`/`GE_PerfectParryWindow`/`GE_DodgeInvincible`/`GE_Stagger`/`GE_StaminaCost`.

**Enemy/ + Enemy/AI/**
- `AKDEnemyBaseCharacter` (Abstract, 723줄, ★최대초과) — ASC를 **Pawn-direct 소유**(Player의 PlayerState 방식과 대비). HitFeedback/Stagger/Execution 3컴포넌트 델리게이트 연결. 브레인정지/랙돌/넉백/경직·처형 액추에이션 밀집.
- `AKDEnemyAIController` — Perception 타겟인지+시야기억, CrowdFollowing.
- `UEnemyAnimInstance` · `UEnemyDefinitionDataAsset`(적1종 스탯/AI/공격셋 data-driven) · `UEncounterSubsystem`(WorldSubsystem, 동시공격 토큰).
- AI/ — EQC(포위대형/타겟), Service(FindPlayer/RequestAttackToken/TryParry), Task(ActivateAbilityByTag/FindKitingLocation/SelectAttack 거리밴드+가중치).

**Game/** `AKDGameModeBase`(얇은 배선) · **Input/** `UInputBufferComponent`(태그+타임스탬프 버퍼 0.2s/4개) · **Interface/** `IKDTargetableInterface`(CanBeTargeted/OnTargeted/GetLockOnPoint) · **Library/** `UKDCinematicLibrary` · **Movement/** `USprintComponent`(Walk/Jog/Sprint/FullSprint 4단, `OnMaxWalkSpeedChanged` 델리게이트).

### 2-2. 핵심 전투 흐름

- **콤보**: `Try*Attack`→태그 GA activate 실패시 InputBuffer.Push. `Tick`이 Attacking/CanCancel 조합 보고 소비시점 결정. GA가 `ComboComponent.ProcessInput`→정확매칭시 브랜치 몽타주/GE 교체, 진행중이면 `DefaultAttackMontages[len-1]`. `ANS_CancelWindow`가 회복구간 CanCancel 부여. 템포 = 입력간격 커브→PlayRate.
- **락온**: `FindBestTarget`(Overlap→인터페이스자격→시야콘Dot→LoS→최단거리)→`EngageLockOn`(LockOn 태그+위젯+인터페이스 알림)→Tick 카메라 보간추적.
- **패링(GA_Parry)**: 홀드, 진입시 BlockGE(Infinite 50%경감)+PerfectParryWindowGE(0.15s) 동시. `AS_Combat::PostGEExec`가 전방판정으로 Perfect(0뎀+이벤트)/일반(50%) 분기.
- **닷지(GA_Dodge)**: 4방향, Perfect판정 = 반경내 적 ASC `EnemyAttackHitWindow` 태그/발사체 `IsPerfectDodgeable()` 조회. Perfect=스태미나면제+긴i-frame+CounterWindow(→GA_CounterThrust), Normal=스태미나소모.
- **처형**: ExecutionComponent가 `Event.Combat.Hit` 구독→Staggered+트리거태그 매칭시 invuln+콜리전무시+데스블로판정+`Event.Combat.ExecutionStarted`를 플레이어에 전송(→GA_PlayerExecution 자동발동). 안전망 타이머. 데스블로는 `SetNumericAttributeBase(Health,0)` 코드강제사망.
- **무기 발검/납검**: `WeaponComponent.OnInCombatTagChanged`→`Event.Combat.WeaponToggle` 발신(다중무기 시 1개만 브로드캐스트, 총쪽 `bBroadcastsToggleEvent=false`)→BP GA/애님이 Draw/SheathMontage→몽타주의 `AN_WeaponAttach` 노티가 실제 소켓 부착. 공격중이면 즉시 손 부착.
- **트레이스/데미지**: `GA_MeleeTraceBase` 몽타주+`Event.Montage.TraceBegin/End`→`AT_MeleeTrace`. `ANS_MeleeTrace` per-window 오버라이드로 무기/맨손 동일태스크. 히트시 팀태그 아군방지→`DamageEffectClass`에 `SetByCaller(AttackPower)`→`AS_Combat::PostGEExec`(유일 데미지게이트)→`Event.Combat.Hit` 브로드캐스트로 피격측 각자 반응.

### 2-3. GameplayTags 요약 (`KDGameplayTags.h/.cpp`)
- **Ability**: `Player.{Light,Heavy,Dodge,Parry,SprintAttack,CounterThrust,AirCombo,Execution,Aim,Shoot,ShotBlast}`, `Movement.Turn`, `Enemy.{Grunt,Elite,Boss}.Attack.*`, `Combat.Unblockable`
  - ★ 2026-08-17 **`Ability.Mugong.*` → `Ability.Player.*` 개명**(길동 컨셉 폐기). 에셋 15곳은 `Config/DefaultGameplayTags.ini`의 `+GameplayTagRedirects` 11줄이 로드 시 이어준다. **그 줄들을 지우면 재저장 안 된 에셋의 태그가 에러 없이 빈다.**
- **State.Combat**: InCombat/InAction/Attacking/Dodging/Invulnerable/CanCancel/MovementCanCancel/Parrying/PerfectParryReady/EnemyAttackHitWindow/CounterReady/AirComboLock/Aiming/Shooting/Staggered/SuperArmor/PoiseRegenPaused(미구현). 그 외 `State.{Stamina.RegenBlocked, Character.LockOn, Movement.Turning/InAir, Dead, Camera.Cinematic}`
- **Event**: `Montage.{TraceBegin,TraceEnd}`, `Rush.Warp`, `Combat.{Hit,HitReact,WeaponToggle,ExecutionStarted,PerfectParryTriggered,ParrySuccess}`
- **SetByCaller**: AttackPower/Stamina · **Team**: Enemy · **Input**: `Action.{Light,Heavy,Dodge,Parry}`/`Combo.{Light,Heavy}` · **GameplayCue**: `Combat.HitImpact.Light`, `Enemy.Telegraph.{Parryable,Unblockable,Lethal}`, `Combat.*`, `Camera.*`
- **태그 총 95개** (2026-08-17 감사). 삭제분 = `SetByCaller.Dosul`(Stamina가 역할 대체) / `Input.Action.Aim`(홀드라 선입력 개념 X). 미사용이나 존치 = `State.Combat.PoiseRegenPaused`(GE_PoiseRegen 미구현) · `Input.Action.Parry`(패링 선입력 대비) · `Ability.Enemy.Boss.Attack.Basic`(보스 예약) · `Cooldown.Enemy.Grunt.Heavy`(형제 6개는 에셋에서 쓰이는데 이것만 0 — 적 작업 때 확인)

### 2-4. 아키텍처 규칙 준수 현황
- **ASC 초기화**(§2-8): Player=`PossessedBy`에서 `InitAbilityActorInfo(PS, this)`, Enemy=`InitAbilityActorInfo(this, this)`. `OnRep_PlayerState` 미구현(싱글 전제, 의도적).
- **의존성 방향**: 대체로 준수. Component→Pawn 캐스팅 없음(ASC 태그/인터페이스 경유). GC→Component는 델리게이트. GA→Component도 2026-08-12부로 `Cast<AKDPlayerCharacter>` 4곳 제거 — `UGA_ActionBase`의 `GetLockOnComponentFromActorInfo()`/`GetComboComponentFromActorInfo()` 접근자로 통일(`AbilitySystem/` 폴더에서 `KDPlayerCharacter.h` include 0개). 예외 = `Handle_Move`가 GAS 태그 직접 읽음.
- **GC 개수**: Telegraph 부모라우팅 + Combat 8종 + Camera 2종 ≈ 5~10개 룰 부합.
- **GE 자식 CDO**: `CreateDefaultSubobject`+`GEComponents.Add` 패턴 준수(§2-7 fatal 회피).
- **줄수 한도 위반** (리팩토링 후보): `AKDEnemyBaseCharacter`(723), `ULockOnComponent`(330), `UExecutionComponent`(306), `GA_Dodge`(338), `GA_Parry`(232), `UKDPlayerAbilityInputComponent.cpp`(352, 신규). ※`GA_MeleeTraceBase`(349)는 base라 의도적. `AKDPlayerCharacter`는 2026-08-12 리팩토링으로 426줄까지 내려가 해소됨(입력 판단이 `UKDPlayerAbilityInputComponent`로 이동).

### 2-5. ★코드 주의/함정 (문서화된 설계 결정)
- **데미지는 ExecCalc가 아님** — SetByCaller + 메타어트리뷰트(`IncomingDamage`) 게이트웨이 방식. CLAUDE.md §1-2 원문만 보면 ExecCalc가 기본으로 읽히나, 실제는 "단순케이스 SetByCaller 예외"를 데미지 전체로 확장한 구조. **새 데미지 로직은 `AS_Combat::PostGameplayEffectExecute`에 붙는다**.
- **InstancedPerActor GA 멤버 잔류** — `GA_Dodge`/`GA_Parry`의 핸들, `GA_PlayerAttackBase`의 `DamageEffectClass` 등은 매 `ActivateAbility` 시작부에서 명시 리셋 필수. 새 GA 작성 시 놓치기 쉬움.
- **히트스탑 = `Montage_Pause`** (SetPlayRate(0) 아님) — 겹친 2타가 rate=0 물어 영구정지하는 문제 회피.
- **처형 데미지 = 코드강제** — `FinishExecution`이 데스블로시 `SetNumericAttributeBase(Health,0)`. GE 밸런스 무관하게 항상 죽는 특수경로.
- **팀 이분법** — `Team.Enemy` 유무로만 아군방지. 팀 3개↑(소환수 등) 확장시 깨짐.
- **`PostGameplayEffectExecute` 사후로직 금지** — Health 0시 `HandleDeath`가 동기완료되므로 그 뒤 코드 추가 금지(주석 경고).
- **`AN_WeaponAttach` TargetWeaponTag=None = 전무기 동시발검** — 다중무기 몽타주에서 특정무기만 부착하려면 태그 반드시 지정.

---

## 3. 기획 요약

### 3-1. 현재 방향 (Gun & Sword, ✅유효)
위 §1 참조. 콤보 설계 문법(`GunSword_콤보구성_v0.md`): 스타터 분리+후반타 풀 공유, 입력창 우상향, 저스트회피=메인풀 재합류/저스트패링=전용 카운터 격리, 건=무기전환 아닌 액션세트(콤보 중 Shoot 캔슬), 공중=독립 미니그래프, 스프린트 전용파생 없음(Run Attack=진입 스타터만).

### 3-2. 시스템별 (현재 유효 vs stale)
| 시스템 | 상태 | 요약 |
|---|---|---|
| **락온** | ✅유효·구현완료 | `docs/design/lockon-system.md`. 인터페이스 기반 5게이트. 단 락온 스트레이프 애니는 버터 스켈레톤 기준→**마네킹 전환시 재작업 필요** |
| **카메라 커브** | ✅유효·구현완료 | `camera-curves-system.md`. SB식 FOV/Pitch/TurnSpeed 커브 |
| **콜리전 채널** | ✅유효·구현완료 | `collision-channels.md`. 무기중립 |
| **데미지** | ✅유효(설계) | `damage-system.md`. GAS SetByCaller+IncomingDamage(위 2-5) |
| **콤보** | ✅유효 | `GunSword_콤보구성_v0.md`가 진실. 길동 2_콤보 수치는 SB실측으로 대체 |
| **밸런싱** | ✅유효 | SB 이브 실측 데이터시트가 초깃값 단일진실 |
| **로코모션** | ⚠️부분 | 코드(`UKDPlayerAnimInstance`) 유효, BS 자산은 길동 스켈레톤→Manny 전환시 재작업 |
| **도술** | ❌stale | 길동 전용, Doul 자원. Gun&Sword엔 Skill1~5/Ultimate 슬롯이 유사역할 후보 |
| **공중QTE** | ❌stale | 길동 "동에번쩍". Gun&Sword는 공중전 축소 재설계중, 팩 Air Combo로 대체예정 |
| **UI/HUD** | 🟡설계완료·미구현 | 길동 3바(HP/Stamina/Doul) 폐기. **SB HUD 구조 전수 조사 완료**(2026-08-17) = 볼트 `notes/Reference/StellarBlade_UI_HUD_구조.md`. 방침 = `WBP_MainHUD` 1개만 뷰포트 + 캔버스 2장, 값은 GAS 델리게이트 push(`WaitForAttributeChanged`). 현재 위젯 2개뿐(`WBP_Crosshair` 화면고정 / `WBP_LockOnReticle` 월드). **1차 = 적 상태 바(Poise+HP)** — Poise만 실동작이라 |
| **적·보스** | ❌stale | 길동 도적/각시탈. 색상신호·GAS모듈공유 문법만 참고. GunSword 적 기획 미작성(스코프만 결정) |
| **분위기** | ❌stale | 길동 사극톤. GunSword 톤 문서 없음(SB 참조가 유일) |

### 3-3. 문서/볼트 지도
- **docs/design/**: `README`, `camera-curves-system`⚠️(05-28 stale), `collision-channels`⚠️(05-22 stale), `damage-system`⚠️(05-22 stale), `lockon-system`⚠️(05-28 stale) — 🗑️`player-locomotion-system`·`butter-*`·`기획/`(8폴더) 삭제됨
- **docs/specs/**: `README` — 🗄️`deep-interview-abp-weapon` → `archive/kildong/`
- ⚠️ **design/ 4문서는 전부 2026-05에서 멈췄다.** 틀린 내용은 없으나 이후 dev-log 30편 이상의 변경이 미반영. **현행 코드 설명은 볼트 `notes/코드구조/` 10문서**(08-08~08-23)가 담당하며, 락온·카메라·콜리전·데미지 4주제 전부 볼트가 더 최신·상세임을 2026-08-24 실측 확인.
- **docs/handoffs/**: `2026-07-21-pivot-gunsword`✅(현재방향 진실), 그외 길동/버터 핸드오프
- **옵시디언 볼트** (`C:\Users\asdasd\Desktop\Obsidian_organize\ProjectKD\`, GitHub private `BlackGildong`):
  - `notes/Project_New/`✅ — 신규 GunSword 기획(핵심 4문서 유효, 버터 자료 혼재)
  - `notes/Reference/StellarBlade_*`✅ — SB 이브 실측(JSON덤프/데이터시트/콤보트리 복원) = 밸런스·콤보 직행근거
  - `notes/언리얼/`(애니/QTE/회피/카메라/트러블슈팅 학습노트) — 기법수준이라 대체로 참고가능
  - `notes/ProjectKD/`❌ — 길동 기획 원본(SESSION_STATE 포함 stale)
  - `notes/wiki/`(43p) — 피벗 미반영, 갱신대기

---

## 4. 콘텐츠 에셋 맵

### 4-1. ★현재 메인: `Content/SB_Style_GameProject/`
- **Player/** — `BP_SBPlayer`(메인, 메시=SKM_Manny_GunSword, 부모=BP_PlayerCharacter), `BP_PlayerController/State`, `BP_KDPlayerCameraManager`
- **Core/** — `BP_GamemodeBase`
- **GAS/**
  - `Abilities/` — GA_LightCombo, HeavyCombo, AirLightAttack, Dodge, Parry, PerfectParryReaction, CounterThrust, HitReact, PlayerExecution, SprintAttack, Turn, WeaponToggle
  - `ComboData/` — `DA_ComboTree`, `DA_AirComboTree`
  - `GE/` — Block/AirComboLock/DodgeInvincible_Normal·Perfect/PerfectDodge_CounterThrust/PerfectParry, `GE/Stamina/`(Regen/RegenBlock/Cost_Dodge/FullSprint)
  - `GC/` — GCN_HitReact/CounterThrust/CounterTrail/PerfectDodge/PerfectParry/PlayerHitConfirm, `CameraShake/`(LCS_*), `ExcuteGCN/`, `HitConfirmDataAsset/DA_HitLightAttack`, `NiagaraEffect/`
- **WeaponData/** — `DA_Weapon_Sword`, `DA_Weapon_Gun`, `DA_Weapon_Spear`, `DA_Weapon_Bong`
- **Animation/** — `ABP_SB`(메인 ABP)
  - `Equip_AM/` — (발검/납검 몽타주 예정: AM_Equip_Draw/Sheath. 소스=AS_Idle_to_Idle_Combat / AS_Idle_Combat_to_Idle)
  - `TurnAbility_AM/` — `AM_Turn_180_L/R`
  - `MotionMatching/` — `Schema/PSS_SB_Default`, `DataBase/`(PSD_SB_Idles·Loco_Unarmed·Stops + PSS_SB_TurnInPlace), `Normalization/PSN_SB`
- **ExecutionData/** `DA_PlayerExecution` · **Camera/Curves/** CF_FovByCamDist·FovPunchCurve·TurnSpeedByVelocity · **Input/** IA_*·IMC_Default · **UI/** DA_LockOnConfig_Default·WBP_LockOnReticle · **PostProcess/** MPC_ScreenEffects·M_PP_ScreenEffect

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

- **RM vs IP 분리**: 방향성 이동류(03_Walk/04_Run/06_Dodge)의 각 세부폴더에 `01_RM`(루트모션)/`02_IP`(인플레이스) 하위폴더. 콤보류(02_Attack)·Idle·Jump·Hit·Turn은 분리없이 리프에 `AS_*_Seq` 직접.
- **스켈레톤/메시** (`Demo/Characters/Mannequins/Meshes/`): `SKM_Manny_GunSword`/`SK_Mannequin_GunSword`, 무기 `Gun.uasset`/`Sword.uasset`(스태틱메시). `Rigs/`(RTG/IK/CR/PA_Mannequin).

### 4-3. 기타 팩 (참고)
버터잔재 `TrickalFanGame/`(stale) · `Map/`(TrickalMapAsset, 버터맵) · 적애니 `BossAnimationsPack`/`BossyEnemyAnimPack`(자체스켈레톤) · `RamsterZ_FreeAnims_Volume1`(45개, 전투몽타주 후보) · `WM_Female_Bundle`(Lucy) · `MotionMatchingAnimation`(Epic MM샘플) · VFX `SlashTrailElemental`(검격트레일)/`Vefects`(충격파) · SFX `MetalandWoodMeleeWeaponsSFXPack` · `killdong_ExecutionTest`(처형 초기테스트) · `_ThirdParty/`(GhostSamurai_Bundle 보류, Windmill 버터맵 등)

### 4-4. 자주 쓰는 경로 빠른참조
| 찾는 것 | 경로 |
|---|---|
| 메인 플레이어 BP | `SB_Style_GameProject/Player/BP_SBPlayer` |
| 무기 데이터 | `SB_Style_GameProject/WeaponData/DA_Weapon_*` |
| 콤보 트리 | `SB_Style_GameProject/GAS/ComboData/DA_ComboTree`·`DA_AirComboTree` |
| GA 전체 | `SB_Style_GameProject/GAS/Abilities/` |
| ABP | `SB_Style_GameProject/Animation/ABP_SB` |
| MM 스키마/DB/정규화 | `SB_Style_GameProject/Animation/MotionMatching/{Schema,DataBase,Normalization}/` |
| 공격 몽타주 소스 | `Gun_and_Sword/Animations/Sequence/02_Attack/` |
| 이동 소스(RM/IP) | `Gun_and_Sword/Animations/Sequence/{03_Walk,04_Run,06_Dodge}/.../01_RM\|02_IP` |
| GS 스켈레탈메시 | `Gun_and_Sword/Demo/Characters/Mannequins/Meshes/SK_Mannequin_GunSword` |
| GS 무기메시 | `.../Meshes/{Gun,Sword}` |

---

## 5. 현재 진행 상황 & 다음 할 일

**현행 작업 = SB_Style 마이그레이션 + 무기(검+총) 발검/납검 시스템** (dev-log: `2026-07-21`, `2026-07-22-gunsword-holster-socket.md`)

✅ **완료**: 프레임워크 SB_Style로 이동, BP_SBPlayer + 컴포넌트 배선, MM 로코모션(idle/walk/run/8방향/루트모션/스프린트/턴), 무기 C++ 코드(2컴포넌트+AN_WeaponAttach+StaticMesh 지원), 검+총 홀스터 소켓 셋업, DA 교차오염 정상화.

⏳ **다음 (순서)**:
1. **발검/납검 몽타주** ← 현재 지점. `AM_Equip_Draw`만 생성됨. 남은 것: 납검 몽타주 + `AN_WeaponAttach` 노티 배치 + `DA_Weapon_Sword` DrawMontage/SheathMontage 배선 + PIE 검증(비전투 홀스터/전투 발검손/납검복귀)
2. Armed 로코모션 (PSD_SB_Loco_Combat + ABP InCombat 분기)
3. 전투 콤보 핵심경로 (팩 02_Attack Combo → 몽타주화(AM_) → 노티(MeleeTrace+CancelWindow) → GA_LightCombo repoint → PIE)
4. 락온 재장착(마네킹 스켈레톤 기준) / 보스·스테이지 / 고급 로코

⚠️ **커밋 대기 코드**: WeaponComponent, AN_WeaponAttach, KDPlayerCharacter, WeaponDataAsset.h, DefaultGame.ini — 발검/납검 PIE 검증 후 커밋 예정.

---

## 6. 세션 시작 프로토콜 (AI)

새 세션 시작 시 순서:
1. `CLAUDE.md`(프로젝트 룰) + `~/.claude/CLAUDE.md`(전역) 읽기
2. **이 문서(`docs/PROJECT_OVERVIEW.md`)** 읽고 전체 그림 복원
3. 메모리(`MEMORY.md`) + 최신 `docs/dev-logs/` 1~2개로 세부 진행상황 복원
4. `git status` + `git log` 확인

**작업 규칙 리마인더**:
- 에디터/MCP 에셋 작업(몽타주·노티·DA·BP)은 **승환이 명시 요청할 때만**(조회는 OK)
- `.h` = 사람 승인영역, AI draft→review→cpp (§0 프로토콜)
- 코드 = Fable5 설계/승인, 구현 = opus/sonnet 하청, 검증·단순작업 = sonnet
- 기능 완료시 `docs/dev-logs/{날짜}-{기능}.md` + `docs/INDEX.md` 등록
