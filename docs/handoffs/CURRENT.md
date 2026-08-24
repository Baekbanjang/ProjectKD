# 현재 상태 — 2026-08-24

> **세션 시작 시 여기부터.** 진행상황 · 다음 할 일 · 보류 목록 · 설계 미결.
>
> **이 파일에는 현행만 둔다.** 절이 쌓이면 월 단위로 `archive/` 로 덜어낸다.
> 지난 기록 = [`archive/2026-07~08-past-sessions.md`](archive/2026-07~08-past-sessions.md)
>
> ⛔ 옛 **A/B 레인(기획A / 코드B)은 2026-07-31 종료**. 그 규칙을 따르지 말 것.
> ✅ 2026-08-24부터 **다른 체제로 2세션을 돌린다** — `코드·에셋 = KD` / `문서·지식 = 볼트`.
> 규칙과 대기 항목은 `CLAUDE.md §0 1-1` + 세션 브릿지(`볼트 notes/_세션브릿지.md`)에.

---

## ★★ 2026-08-24 — 문서 정리 + 세션 브릿지 운영 개시. **다음 세션은 여기부터**

dev-log = `docs/dev-logs/2026-08-22-debug-console-player-init.md` (직전 = `2026-08-21-aim-knockback-camera.md`)

### ✅ `Project_KD` 는 깨끗하다 (2026-08-24 기준)

소스·docs·`CLAUDE.md` 전부 커밋·푸시 완료. 작업 트리 변경 0 / 미푸시 0.

```
9da756c  [doc] 두 세션 역할 구분 + 세션 브릿지 등록
09c40f7  [docs] 길동 시대 문서 정리                    ← 볼트 세션
e71cd87  [docs] 08-21 / 08-22 dev-log + 현재 상태     ← 볼트 세션
5c32389  [Input] 입력 버퍼 보관 기한 상한 0.8
a975cb2  [refactor] 개발용 온스크린 표시를 콘솔 변수로
```

### 🟡 남은 것 = `Content` 12개

**"전부 촬영용 원복분"이 아니다.** 3덩어리로 갈린다 — git 크기 이력 대조로 확정(MCP 불필요).

```
① 촬영 임시값 원복 (4파일)   커밋해도 안전
   DA_Sword_Bandit   촬영전과 바이트까지 동일 = Poise 1 -> 3
   GA_Parry          78433 -> 78492 -> 78433 = 퍼펙트 창 0.5 -> 0.2
   GA_Light/HeavyCombo  디버그 표시 원복

② 촬영과 무관한 오래된 미커밋 (6파일)   촬영 커밋 6개가 건드리지도 않았다
   BP_Dummy         -688   마지막 커밋 180ab96 (08-19)   실제 내용 변경
   BP_Bandit_Parry  -450   마지막 커밋 662390c (MCP 도입 전)  실제 내용 변경
   BP_Bandit -7 / BP_Axe_Elite +8 / Arrow ±0 / Arrow2 ±0   노이즈 수준

③ LV0_Test.umap  -4699   조명·노출 원복분으로 보이나 값 확인은 에디터 필요
④ Robot3/        미추적 유지
```

⚠️ **②의 `BP_Dummy` · `BP_Bandit_Parry` 는 내용이 실제로 빠졌고 무엇이 빠졌는지 모른다.** 커밋 전 에디터 확인 권장.
→ **권장 = ①만 먼저 커밋.** 한 커밋에 섞으면 나중에 되돌릴 때 못 가른다.

### 🔴 그 밖에 대기 중인 것 2건

```
KDPlayerState.h:33   주석 Dosul -> Ammo    가이드 준비됨, 승환 입력 대기(§0)
디버그 원복 누락 4건   GA_AirLightAttack · GA_CounterThrust · GA_ShotBlast · GA_SprintAttack
                     촬영용으로 끈 6개 중 2개만 돌아왔다. 판정 궤적이 안 보이는 상태
```

**단일 진실 = 세션 브릿지** `C:\Users\asdasd\Desktop\Obsidian_organize\ProjectKD\notes\_세션브릿지.md`

### 🔴 그다음 = 확인 1건

**`AM_SB_Parry_Counter_Attack_L` 만 워프 노티 `RotationType = DEFAULT`** (타겟명 `CounterTarget`). 워프 노티 19개 중 유일. 반격이 엉뚱한 방향으로 도는지 미확인.
→ 함정 전문 = 볼트 `트러블슈팅/모션워핑-Facing회전이_지나쳐달리는클립을_뒤로돌림`

### 이번에 한 것 4건

```
디버그 콘솔화   KD.ShowDamage / ShowDodge / ShowApproach / ShowKnock  (ECVF_Cheat)
                ⚠️ 무기 궤적·조준선은 여전히 BP 체크박스 (bDrawDebug / bDrawAimDebug)
플레이어 초기값  GE_InitPlayerStats 신설 -> BP_PlayerState.StartupEffects
                적은 DA 로 이미 데이터화, 플레이어만 C++ 생성자에 있었다
마무리 워프 회전 AM_SB_Combo_01~05_04 warp_rotation 해제 (적 등 뒤 착지 버그)
입력 버퍼        ClampMax 0.8 — 상한만 열었고 BP 값은 0.5 그대로
```

### 문서에서 낡아 있던 것 3건 (고침)

```
PROJECT_OVERVIEW  UAS_Player 가 Dosul 을 아직 있다고 적고 있었다 (08-18 에 Ammo 로 교체됨)
볼트 06_적_AI      넉백 코드가 StopMovement() 만 있는 옛 버전 / KnockbackStrength 400 (실제 1800)
볼트 04_어트리뷰트  "플레이어는 생성자 값이 곧 최종값" — GE 신설로 거짓이 됨
```

**인용 전에 검증할 것.** 셋 다 코드는 진작 바뀌었는데 문서만 남아 있던 것들이다.

### 포폴 촬영 — 완료

영상 편집본 = `D:/Capcut/0822(1).mp4` (6분, 1994x1080 60fps). 구성·자막 확정. **산출물이지 재개점이 아니다.**

---


---

> 🗄️ **지난 세션 기록은 [`archive/2026-07~08-past-sessions.md`](archive/2026-07~08-past-sessions.md) 로 분리했다** (2026-08-24, 826행).
> 완료된 절이라 현재 상태와 무관하다. 설계 근거를 되짚을 때만 연다.

---

## 3. 보류 목록 (우선순위 순)

| # | 항목 | 내용 |
|---|---|---|
1 | **`DA_ComboTree` 값 채우기** ★ | **두 값의 상태가 다르다 — 헷갈리지 말 것** (2026-07-31 A레인 지적으로 정정)<br>· **`InputWindow`** = **칸 있음 / 값 전부 0** → 아직 `ComboResetTime 1.5f` 공용값으로 돈다<br>· **`DamageMultiplier`** = **칸 자체가 없다.** `.h` 실측 확인 — DA를 열어도 그 칸은 안 보인다. `FComboNode`에 추가부터 해야 함(`InputWindow` 바로 아랫줄, 같은 형식)<br>SB 입력창 실측: 1~2타 0.7~0.8 / 3~4타 0.9~1.2 / 마무리 1.4~2.0 / 회피 0.8 / 저스트회피 1.5<br>⚠️ **DA는 2개다** — `DA_ComboTree` + `DA_AirComboTree`(같은 `FComboNode` 구조)<br>⚠️ `FComboNode`에 **`DamageEffectClass`(노드별 GE)가 이미 있다** — 계수를 float으로 넣을지 노드별 GE로 갈지 먼저 정할 것. 26노드 × 개별 GE = 에셋 26개라 **float 계수가 가볍다** |
2 | **캔슬 윈도우 늦은 몽타주 3개** | `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74). 버퍼 0.5초로도 못 덮는다. `ANS_CancelWindow`를 앞으로 당기는 게 유일한 해법 — 단 안무 자체가 후딜이 긴 동작일 수 있어 포즈 재확인 필요 |
4 | ~~발사체 리팩토링 3건~~ **2/3 이미 닫힘** (2026-08-13 실측) | ① 델리게이트 바인딩 → `KDProjectile.cpp:53~54`에서 **`BeginPlay`로 이미 이동됨**(08-10 `b36c1c2`) ✅<br>② `GA_Dodge`가 발사자를 안 봄 → `GA_Dodge.cpp:198`에 **`&& Proj->GetInstigator() != Avatar` 이미 있음** ✅<br>③ faction 게이트 비대칭 → `KDProjectile.cpp:72~77`이 "적→적 통과"만 검사. **살아 있으나 소환수·동료가 생겨야 터진다. 급하지 않음**<br>곁가지 = `InitProjectile`에 방향을 정하는 줄이 없다(스폰 회전을 그대로 씀). 버그가 아니라 현재 설계 |
5 | `EnterNode`가 `Context`를 안 받는다 | 트리를 지상→공중 순차 조회로 우회 중. 노드 ID가 안 겹쳐서 지금은 확실하지만, 겹치는 ID가 생기면 깨진다 |
6 | `OnInActionTagChanged` 재호출 | GA가 겹치면 `NewCount` 1→2로 재호출. 같은 소켓 재부착이라 결과 동일. **제약**: `AttachWeaponToHand()`에 1회성 작업(사운드·이펙트) 넣지 말 것 |
7 | **트레일 NS 변수 검증** | `SwordLength`/`TrailWidth`가 새 NS에 먹는지 PIE 확인. 안 먹으면 NS User Parameter 이름을 맞춰야 함 |

### 2026-08-12 신규 보류 (dev-log `2026-08-12-input-component-and-muzzle-socket.md` 참조)

| # | 항목 | 내용 |
|---|---|---|
8 | ~~`AM_SB_Combo_05_03` 첫 `Shot` 노티 각도~~ **닫힘** (2026-08-13 MCP 실측) | **5개 전부 `muzzleDir=True` / `halfAngle=180` / `ignoreHitStop=True`로 이미 통일돼 있다.** "1번만 10, 나머지 179"는 지나간 기록<br>**총격 노티 전수 = 13개 몽타주 20발.** 05_03(5발)만 위 설정이고 **나머지 12개는 전부 기본값**(`muzzleDir=False` / `halfAngle=0`→GA 값 20도 / `ignoreHitStop=False`)<br>→ 08-13에 붙인 총격 자동 조준은 **12개 몽타주 15발에 먹는다.** 05_03은 각도 180(=전방위)이라 방향이 판정에 영향 없음 |
9 | ~~360° 콘이면 한 적이 5번 맞는다~~ **의도로 확정** (2026-08-13 승환) | 발당 데미지 분배 **안 한다.** 5연타 전방위가 의도. `ignoreHitStop=True`도 그 의도와 맞물림(SB도 다단히트 스텝은 히트스톱을 끈다). `ShotRange` 500도 유지 |
10 | **디버그 구체 그리기 미적용** ★ | 각도 90° 이상이면 `DrawDebugSphere`로 대체. **05_03이 정확히 그 경우라 지금 그 판정 범위를 눈으로 볼 방법이 없다**(`DrawDebugCone`이 180°에서 뒤쪽 한 점으로 뭉쳐 바늘로 보임 — `LineBatchComponent.cpp:515~546`). 판정은 정상, 그리기만 문제<br>코드 = `GA_ShotBlast.cpp:107`의 `ConeRad` 선언을 `else` 안으로 옮기고 `if (HalfAngle >= 90.f) DrawDebugSphere(World, Origin, ShotRange, 24, ...)` 분기. include 불필요(`:163`에서 이미 사용) |
11 | **일반 공격 자동 조준(미착수)** | 락온 안 걸었을 때 가장 가까운 적 쪽으로 자동 회전. `ULockOnComponent::FindBestTarget()`이 public이라 재사용 가능 — `GA_PlayerMeleeAttackBase::OnActivated`의 락온 게이트만 바꾸면 됨. 미결 = 카메라 정면 기준(현재 동작, ±45°)이냐 스틱 입력 방향 기준이냐 |
12 | **`UKDPlayerAbilityInputComponent.cpp` 352줄** | §1 Component 300줄 선 초과. 분리 여부 미결 |
13 | **`DA_Sword_Bandit` 등 적 정의 4개 `PoiseDamageByAttack` 키 미확정 관측** | python 조회 결과 비어 보이나 조회 한계일 수 있음. 에디터에서 직접 확인 필요 |

### 닫힌 항목 (2026-08-12)

- ✅ **`ANS_EnemyAttackWindow`의 `AttackWindowTag`** — 생성자 이니셜라이저 한 줄 추가(`: AttackWindowTag(GameplayTags::State_Combat_EnemyAttackHitWindow)`). 이미 배치된 노티에도 소급 적용(CDO 기본값 직렬화 미포함). 코드 1줄로 끝났다 — 아래는 원인 기록
  - 증상은 맞았다 — 비어 있으면 퍼펙트 회피가 영원히 안 뜬다. 형제 클래스 `ANS_CancelWindow`가 이미 생성자에서 기본값을 넣는 패턴인데(`CancelTag(GameplayTags::State_Combat_CanCancel)`) 이쪽만 빠져 있었다

### 닫힌 항목 (2026-07-30~31)

- ✅ **`GA_Dodge` 레벨 전체 순회 → 반경 물리 조회** (`84b57af`). 채널 2개(Pawn + `ECC_GameTraceChannel1`) 필수 — 발사체는 Pawn이 아니다. `EnvQueryContext_AllyEnemies`는 **고칠 필요 없다**: 타입 지정 순회 + "아레나 3~12마리라 가볍다"고 파일 주석에 판단 근거가 있다
- ✅ **`ANS_WeaponTrail` 액터별 분리** (`84b57af`). 노티 객체는 몽타주 에셋 소속 1개라 모든 액터가 공유 → 멤버 대신 `TMap<MeshComp, Trail>`
- ✅ **`PerfectDodgeWindowSec` 삭제** (`84b57af`)
- ✅ **`FComboNode.InputWindow` 칸 + 배선** (`c0ccc9e`). `ProcessInput` 타이머를 노드 결정 후로 이동, `EnterNode` 우선순위 3단계
- ✅ **입력 버퍼 0.5 / 2** (Content `60698c5`)
- ✅ **`AirComboResetTime` 폐기** (2026-07-31) — `DA_AirComboTree`도 같은 `FComboNode`라 **`Air_1~4` 각각이 이미 `InputWindow` 칸을 가진다.** 컴포넌트 레벨 노브보다 정밀하고 코드 0줄. 항목 자체가 사라졌다
- ✅ **트레일 NS 27개 배정** (Content `4f1ac60`)

### 닫힌 항목

- ✅ **`PerfectDodgeWindowSec` 삭제** (2026-07-30) — 참조 0건인 죽은 값. 실제 퍼펙트 판정은 시간이 아니라 **상태**(적 ASC의 `State.Combat.EnemyAttackHitWindow` 태그). ⚠️ `PerfectDodgeCheckRadius`(500)는 살아 있다
- ✅ **`Block_End` = A안(연결 안 함)으로 확정** (2026-07-30 승환) — C++·BP 그래프 전수 확인 결과 재생 경로가 없고, 지금 자연스럽게 보이는 건 **ABP 블렌드 아웃(0.25초)**이다. 연결하면 가드를 놓고 0.6초간 묶여 "가드 풀었는데 못 움직인다"는 새 문제가 생긴다. SB도 블렌드로 처리
  - 뒤집을 때 방법 3개: ① 짧은 전용 GA ② `GA_Parry` 종료를 몽타주 끝까지 지연 ③ GA 없이 `AnimInstance`에 직접 재생(가장 단순, 순수 연출이라 판정 없음 — 단 §1-3 의존성 방향 확인)

---

## 4. 설계 미결 — 판단 대기 ★

코드가 아니라 **판단이 필요한** 것들. 착수 전에 승환이 정해야 한다.

### ① 스탠스 체계

> **★2026-07-31 — 실목록과 수치를 찾았다.** `Content/Local/Data/CharacterStanceTable.json`. 스탠스가 곧 이동 속도 세트다.

| 스탠스 | Walk | Jogging | **Run** | LockOn Run |
|---|---|---|---|---|
| **`P_Eve_Default`** (평시 = 검) | 150 | 300 | **500** | **280** |
| `P_Eve_BlockSword` / `Fusion` / `Tutorial` / `AirDead` / `Freeze` | 150 | 300 | 500 | 280 |
| **`P_Eve_Tachy`** (각성) / `Fusion2` | 150 | 450 | **600** | 400 |
| **`P_Eve_Gun`** / `Gun_Gorgon` | **100** | **100** | **100** | 100 |
| `P_Eve_GunNikke` / `GunBlockSword` / `GunTutorial` | 120 | 120 | 120 | 120 |
| `P_Eve_Fishing` | 0 | 0 | 0 | 0 |

**여기서 나오는 것 3가지**
- **총 스탠스 = 100. 걷기(150)보다 느리다.** "총은 딜링이 아니라 근접의 준비 도구"라는 §1-B 해석이 수치로 확증됐다. **사격 착수 시 이 값을 그대로 쓴다**
- **락온 전용 속도가 따로 있다** (500 → 280, 44% 감속). 우리는 이 개념이 없다
- **스프린트가 없다.** SB는 150/300/500 3단계. 우리는 250/500/700/800 4단계로 **전반적으로 빠르다** — 우리 Jog(500)가 SB 최고속과 같다

SB는 `Default(=Sword) / Tachy / Fusion / Gun계열 / Fishing / 특수(사망·동결·튜토리얼)`로 나뉜다. **평시 스탠스가 없다** — Eve는 항상 무장 상태다.
우리 초안(승환): `Default(평시 전투 = Sword&Gun) / Gun(진짜 사격만) / 사망`.
→ **사격 로직 착수 전까지 보류 결정됨**(YAGNI). 사격을 시작할 때 이 표를 확정해야 한다.

> **2026-07-31 갱신 — 이제 확정할 재료가 다 모였다.** §1-B의 사격 조사 참조.
> 팩 구성이 **두 설계를 다 지원한다**: (a) SB식 홀드 조준(임시 모드, 놓으면 검 복귀) / (b) 스탠스 전환식(검·총 대등, `Combo_Attack_Shoot` 4타 사용).
> **권장은 (a)** — 검 콤보 구조를 안 건드리고 MM Chooser 컬럼 하나로 조준 로코모션이 들어온다. (b)는 `DA_GunComboTree`가 하나 더 필요하고, 그건 아직 값도 안 채운 콤보 트리를 하나 더 이고 가는 것.
> **사격 착수 전에 확정할 것.**

### ② 공중 콤보 재설계
`Air_01`이 **총 클립**이라 1타에 검 판정이 없다(`MeleeTrace` 없음이 의도).
방향: **07 유지 + 14~16 `Attack_Air_to_Floor`를 마무리로 붙여 지상 콤보로 연결**(공중 공격 → 지상 찍기 → 자연스럽게 지상 콤보).
`02_Attack` 폴더는 **전부 루트모션** — InPlace 클립이 없다. "제자리 공중 공격"은 RM을 끄는 게 아니라 **이동량 0인 클립**으로 얻어야 한다.

### ③ 스태미나 폐기
점프·달리기 자원 폐기 결정(스킬 코스트는 유지). SB 644행 실측으로 검증됨(SB의 Stamina = 적 격파 게이지). **미착수.** 소모처 3곳 값 0 + BP Cost GE 함정 주의.

### ④ 락온 애니
락온 상태 전용 애니가 보류 상태. 검 콤보 → 총 순서로 미뤄뒀다.

### ⑤ 미착수 폴리싱
~~트레일 NS 27개 미배정~~ **✅ 완료 (2026-07-31, Content `4f1ac60`)** / **사운드 노티 없음** / 데미지 GE 26노드 비어 있음 / LoP식 방사형 회피 이펙트.

> **사운드가 이제 제일 큰 구멍이다.** 트레일 27개가 붙어서 **볼 건 생겼는데 들을 게 없다.**
> 노티 자리는 이미 잡혀 있다(`ANS_MeleeTrace` 위치 그대로) — 사운드 노티를 얹는 작업이다. SB도 `FootStepL/R` + `CheckPhyMat`(물리재질 연동)으로 발소리를 따로 관리한다.

> **★ 2026-08-13 MCP 전수 실측으로 그림이 바뀌었다 — 상세 = 메모리 `reference_project_sound_inventory`**
>
> | | 실제 |
> |---|---|
> **발소리** | ❌"전부 없다" → ✅ **`/Game/MotionMatchingAnimation/Audio` 에 286개 이미 있다.** 걷기30·달리기29·스트레이프29·착지20·점프17·구르기9 + `MSS_FoleySound_*` **동작별 래퍼 13개**. **없는 건 재료가 아니라 배선.** Sonniss 없이 지금 붙일 수 있다 |
> **검 소리** | 배선 완료(08-11). `SC_Sword_Hit`→`Metal_Hit_Flesh_1~20` / `SC_Sword_Swing`→`Metal_Light_Whoosh_1~12` / `_Heavy`→`Metal_Heavy_Whoosh_1~10`. **랜덤 풀은 넉넉하다** |
> **"빈 철봉" 정체** | `Metal_Light_Whoosh` = **공기 가르는 소리만 든 팩.** 금속 울림 성분이 없다. → `SlashTrailElemental/Resource/SW_Basic_Slash`·`SW_Distortion_Slash` 를 큐 안에서 Mixer 로 겹치면 붙는다(⚠️ 미시험) |
> **총성** | **0개.** Sonniss GDC 7.47GB 필요. `Gun_and_Sword` 팩엔 사운드가 하나도 없다 |
> **그 밖 없는 것** | 피격 보이스·신음 / UI / 발검·납검 금속음 = 전부 0. 앰비언트는 MM 샘플 딸림 4개뿐 |
>
> 우리가 만든 사운드 에셋은 `/Game/SB_Style_GameProject/Audio/Combat` **큐 3개가 전부**다. `/Game/Assets/Sfx` 2개는 길동 잔재.

### ⑥ 카메라 (2026-07-31 신규)
1단계 값 표는 §1-B에 **완성돼 있다**(BP 6 + 코드 1줄, 30분). 2단계 스플라인 돌리도 실현 가능 확인됨.
**판단 필요**: 카메라를 지금 하나, 사격 뒤로 미루나. 승환은 "시점이 중요하다"고 했고, 값 이식은 30분이라 사격 전에 끼워넣을 수 있다.

---

## 5. 세션 시작 시 읽을 것

**단일 진실 = `CLAUDE.md §0 세션 시작 프로토콜`.** 여기 복사본을 두면 또 갈리므로 포인터만 둔다.

⚠️ **에셋 값은 코드 기본값과 다를 수 있다.** MCP 로 실측할 것 — 2026-07-28 에 공격 GA 5개가 폐기된 길동 창 소켓(`Spear_Tip`)을 물고 있어 근접 판정이 죽어 있었다.

---

## 6. 오늘 배운 함정 3개 (재발 방지)

1. **`.h` 선언과 `.cpp` 정의는 짝이다.** `InAction`이 `.cpp`만 있어서 빌드가 깨졌다. 태그 추가 시 양쪽 확인
2. **구독과 해제는 같은 태그로.** `RegisterGameplayTagEvent(A).Add()` 하고 `RegisterGameplayTagEvent(B).Remove()` 하면 조용히 실패한다 — 핸들 리셋은 그대로 돌아서 코드가 깨끗해 보인다
3. **`search_assets`는 패턴을 무시하고 100개를 통째로 반환한다.** 목록·집계는 `execute_python`으로. 실측: 821개 중 100개가 돌아와 1만 토큰 낭비

### 2026-07-31 추가 4개

4. **`CurveFloat`의 `float_curve`는 Python에 노출 안 된다.** `get_editor_property('float_curve')` → `Failed to find property`. **커브 키 편집은 에디터에서만.** 읽기는 `get_float_value(x)` / `get_time_range()` / `get_value_range()`로 가능하니, 값 샘플링으로 키를 역추출한 뒤 사람이 편집하는 방식이 현실적
5. **UE의 `FieldOfView`는 가로 FOV다.** 16:9에서 75 → 세로는 46.7도(아래로 23.35도뿐). "왜 발이 안 보이지"의 계산 근거가 여기
6. **핸드오프의 "미완"을 믿지 말고 실측할 것.** BP 값 6개가 이미 다 들어가 있었다. `inspect_cdo`로 30초면 확인된다
7. **스플라인 점의 회전이 시선인지 판별하는 법** — 점들의 높이가 크게 다른데 회전값이 같거나 무관하면 **그건 시선이 아니다.** 머리 위 5m와 발치 아래에서 같은 각도로 같은 대상을 볼 수 없기 때문

