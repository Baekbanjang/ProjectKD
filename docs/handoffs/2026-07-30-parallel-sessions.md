# 핸드오프 — 2026-07-30 ~ 31 · 2세션 병행 체제

> **최종 갱신 2026-07-31 (B레인).** 오늘 분은 §1-B에 있다.

> **이 문서를 읽는 세션에게**: 아래 §0에서 **네가 어느 레인인지** 먼저 확인하라.
> 레인을 어기면 두 세션이 같은 파일을 다르게 고쳐 커밋이 충돌한다.

---

## 0. 레인 구분 (충돌 방지 규칙) ★

| | **A레인 — 기획·설계** | **B레인 — 코드·에셋** |
|---|---|---|
| 하는 일 | 기획 검토, 설계안, 스펙 patch, 문서 정리, 자료조사 | `.h`/`.cpp` 수정, BP·에셋 작업, 빌드, PIE 검증 |
| 쓰기 허용 | `docs/**` · 옵시디언 볼트 | `Source/**` · `Config/**` · `Content/**` |
| **금지** | `Source/` `Config/` `Content/` 수정 | 기획 문서 재작성(스펙 patch는 A가) |
| Git 커밋 | `docs/` 만 | `Source/` `Config/` + Content repo |
| **UE 에디터 / MCP** | ❌ **쓰지 않는다** | ✅ B만 사용 |
| 메모리 쓰기 | ❌ 읽기만 | ✅ B만 기록 |

**왜 이렇게 나누나**
- **UE 에디터는 하나뿐이다.** 두 세션이 MCP로 동시에 에셋을 쓰면 어느 쪽 변경이 남는지 알 수 없다
- **메모리 `MEMORY.md`는 두 세션이 공유한다.** 동시에 쓰면 나중 쓰기가 앞 줄을 덮어 사라진다
- 빌드도 하나뿐이다. A가 코드를 고치면 B의 빌드가 이유 없이 깨진다

**A가 코드를 고쳐야 할 결론에 도달했을 때**: 고치지 말고 `docs/` 안에 "제안" 형태로 남기고 승환에게 알린다. B가 받아서 구현한다.

---

## 1. 지금 어디까지 왔나 (2026-07-30 종료 시점)

### 프로젝트 상태
- **Gun & Sword 피벗 라인** (2026-07-21). SB(스텔라 블레이드) 스타일 지상 액션
- 캐릭터 = 마네퀸(`SK_Mannequin_GunSword`), 이동 = 모션매칭 + Chooser, 전투 = 몽타주
- 코드 규모: `.h` 86개 ≈ 4,000줄 / `.cpp` 83개 ≈ 6,900줄
- **`docs/archive/` 는 읽지 않는다** — 버터/길동 폐기 라인

### 오늘 완료된 것

| 항목 | 결과 |
|---|---|
**`State.Combat.InAction` 우산 태그** | 코드 + BP 7개 + 빌드 + PIE 이상 없음 ✅ |
**회피 캔슬 목록 확대** | Light만 → 공격 4개. 세 곳의 서로 다른 목록을 헬퍼 하나로 통합 ✅ |
**ABP_SB 슬롯 재배치** | `Slot 'DefaultSlot'`을 Locomotion SM 뒤로 + 중복 노드 제거 → 공중 몽타주 출력됨 ✅ |
**`AM_SB_Combo_Air_01` 노티 복원** | `WeaponAttach@0f` + `CancelWindow 28~55f`. PIE 로그로 4타 연결 확인 ✅ |
**퍼펙트 패링 큐 발동 노드** | `GA_PerfectParryReaction`에 `Execute GameplayCue On Owner` 추가 ✅ |
**코드 설명서 8문서** | 옵시디언 볼트 `notes/코드구조/` 2,776줄 신설 ✅ |

### 커밋

```
소스 repo (GitHub 푸시됨)
  6cc9340 [doc]    INDEX에 코드 설명서 포인터
  474bd52 [Combat] 회피 캔슬 목록을 공격 4개로 확대
  4a44cfd [GAS]    State.Combat.InAction 우산 태그

Content repo (로컬 전용, 푸시 금지)
  72017a2 [Anim] AM_SB_Combo_Air_01 노티 + 스프린트/반격 GA에 Attacking
  33b9957 [BP]   InAction 태그 7개 + 퍼펙트패링 큐 + ABP DefaultSlot 이동
```

---

## 1-B. 2026-07-31 (B레인) — 조사 중심 세션

코드 변경은 거의 없고 **실측·조사·문서 정정**이 대부분이다. A레인이 볼 값이 여기 많다.

### ✅ 완료

| 항목 | 결과 |
|---|---|
**무기 트레일 NS 배정** | `NS_SlashTrail_Distortion_Only_Loop` → 몽타주 25개 / 슬롯 27개. Content `4f1ac60` ✅ |
**SB 카메라 실측 재검증** | 원본 JSON 재파싱. **볼트 문서 정정 3건 + FOV 문제 해결** (아래) |
**SB 조작 키맵 확정** | 승환 캡처로 마우스 버튼 세부까지 확정. 메모리 `reference_control_scheme` 전면 갱신 |
**사격 로직 사전조사** | 발사체 채택 결정 + 팩 애니 인벤토리 + 배선안 (아래) |

트레일은 SB식 **무색 왜곡** 궤적으로 골랐다(색 없음, 배경을 일그러뜨리는 방식).
⚠️ 미확인 하나 — 우리 `ANS_WeaponTrail`이 미는 `SwordLength`/`TrailWidth` User Parameter를 이 NS가 받는지는 **PIE로 봐야 안다.**

---

### ★ SB 카메라 — 볼트 문서에 틀린 게 3개 있었다

전문은 옵시디언 `notes/Reference/StellarBlade_Player_Analysis.md §4-6` (2026-07-31 정정 반영 완료).

| # | 이전 기술 | **실측** |
|---|---|---|
1 | 스플라인 회전 `+81.6 / -33.7 / -36.6` | **`-81.6 / +33.8 / +36.6`** — 부호 전부 반대. BalanceBeam·SlidingSlip도 |
2 | "SocketOffset 덤프 미출력 = 비교 불가" | **스플라인 t=1에 `Y: 40`이 있다** — 어깨 오프셋. 우리도 40이라 이미 일치 |
3 | SlidingSlip만 2점 | **`SlopeMoving2`도 2점** |

**새로 밝혀진 것 2개**

- **FOV 3개 공존 문제가 거의 풀렸다.** `Eve_CameraFov`가 `0→75, 1→75`로 **평평하다 = 무효 슬롯**. 남는 건 컴포넌트 `55`(초기값 성격)와 `FovCurveByCamDist` 40~75 → **거리연동이 이길 가능성 높음**(C++ 없어 추론)
- **`Eve_CameraPitch`(-89~+45)는 FOV 커브가 아니라 돌리 위치 입력이었다.** 시선 피치가 카메라를 곡선 위로 끌고 다닌다. 이게 `ViewPitchMin`/`Max`의 근거

**★가장 중요한 정정 — "SB 암 길이 400"은 성립 안 하는 말이다.**
`TargetArmLength`·`SocketOffset`·`TargetOffset` 셋 다 JSON에 **없다**(엔진 기본값). SB는 스프링암 길이를 안 쓰고 돌리가 위치를 통째로 정한다. 402는 스플라인 t=1의 **총 거리**다.

### 카메라 1단계 — 값 표 완성 (실행 대기)

고정 스프링암으로 SB **수평 시점**을 근사한다. BP 6개 + 코드 1줄.

| 항목 | 현재 | SB 환산 | 위치 |
|---|---|---|---|
`TargetArmLength` | 500 | **382** | BP CameraBoom ※코드 350은 BP가 덮음 |
`SocketOffset.Z` | 70 | **111~118** | BP |
`SocketOffset.Y` | 40 | 40 ✅ 일치 | — |
`bEnableCameraLag` | **false** | **true** | BP |
`CameraLagSpeed` | 10 | **19** | BP |
`CameraLagMaxDistance` | 0 | **57** | BP |
회전 랙 | false | false ✅ SB도 안 씀 | — |
`ProbeSize` | 12 | **10** | BP |
`ViewPitchMin` | -80 | **-89** | `KDPlayerCameraManager.cpp:12` |
`ViewPitchMax` / FOV | 45 / 75 | 45 / 75 ✅ | — |

> **`SocketOffset.Z` 48cm 차이가 핵심.** 거리만 줄이고 높이를 안 올리면 시점이 안 맞는다.
> 캡슐 보정 — SB 95/40, 우리 88/34라 Z는 **111** 근처가 실제로 맞다.

### 2단계 스플라인 돌리 — 실현 가능 확인됨

엔진 소스 확인: `SpringArmComponent.h:171 UpdateDesiredArmLocation()`, `:177 BlendLocations()` **둘 다 `protected virtual`**. 시프트업이 `SBSpringArmComponent`로 한 게 정확히 이 override다.
분량 = 클래스 1개(150~200줄) + 스플라인 1개. **우리는 레일 13개가 필요 없다** — SB는 수영·비행·외줄타기가 있어서 그렇고, 지상 액션은 기본 1개면 된다.
⚠️ `AKDPlayerCameraManager::UpdateViewTarget`이 이미 POV를 손댄다 — 어느 쪽이 최종인지 정하고 시작할 것.

**순서**: 1단계 값(30분) → 기준선 확보 → 돌리. 돌리를 먼저 만들면 비교 기준이 없다.

---

### ★ 사격 로직 — 사전조사 완료 (착수 대기)

**① 우리한테 이미 원거리 뼈대가 있다**
`Combat/KDProjectile.h` + `Abilities/Enemy/GA_EnemyRangedAttack.h`. 적 투척용으로 만든 것이고 **플레이어 총이 그대로 재사용 가능**하다(`AKDProjectile`이 `Abstract` = BP child로 비주얼·속도 지정).

**② 히트스캔이 아니라 발사체로 간다** (결정)
- `AKDProjectile` 재사용 = 0줄
- 퍼펙트 회피가 `bPerfectDodgeable`로 **발사체**를 인식한다 — 히트스캔이면 이 경로가 죽는다
- SB 드론건도 탄이 보인다

**③ 3인칭 함정** — 크로스헤어는 화면 중앙, 총구는 어깨 옆. `카메라에서 트레이스 → 조준점 확정 → 총구에서 재조준`이 필요하다. 근거리에서만 티가 나서 나중에 발견하게 되는 종류.

**④ GAS 타게팅은 대부분 불필요** — `WaitTargetData`/`TargetActor`/예측 스폰/Ability Batching은 전부 **멀티 예측 때문에** 있는 복잡도다. 싱글이라 GA 안에서 트레이스하고 스폰하면 된다.

**⑤ SB 방식 (키맵 캡처로 확정)**

```
Ranged Mode  = Ctrl        (패드 L2 홀드. PC판 토글/홀드 여부는 미확정)
Ranged Fire  = 좌클릭
Select Ammo  = 우클릭
```
설계 의도 = **총은 딜링이 아니라 근접의 준비 도구**. 실드 깎기 / Repulse 후 약점 노출 / 거리 벌리기 / 락온 해제.

**⑥ 팩 애니 인벤토리 — 예상보다 훨씬 많다** (이전 "6클립"은 틀린 기억)

```
01_Idle/05_Offset_Aim/        AimOffset 9포즈 그리드 (중앙+상하좌우+대각4)  ← 교과서 그대로
02_Attack/09_Normal_Attack/   Aim_the_Target_{Start, Shoot, Walk_Shoot, Run_Shoot, End}   5
02_Attack/06_Combo_Attack_Shoot/      01~04 + _All      5   ← 총 콤보 4타
02_Attack/08_Combo_Attack_Air_Shoot/  01~04 + _All      5
02_Attack/11_Dodge_Shoot/     Dodge_Shoot ×4 + Dodge_Air_Shoot ×4          8
03_Walk/04_Walk_Aim/          8방향 × (Start/Stop_L/Stop_R) × RM·IP
04_Run/03_Run_Aim/            8방향 × 동일
09_Turn/04_Turn_Aim/          90_L, 90_R, 180_L, 180_R                     4
```

**⑦ 배선안 — 홀드 조준(SB식) 권장**

```
IA_Aim 홀드 시작 → GA_AimMode → State.Combat.Aiming 부여
                   Aim_the_Target_Start (상체) / 이동속도 감소
                   MM Chooser에 Aiming 컬럼 추가 → Walk_Aim / Run_Aim DB로 전환
IA_Aim 해제      → 태그 제거 + Aim_the_Target_End → 검 스탠스 복귀
IA_Fire (Aiming 중) → GA_Shoot → Shoot / Walk_Shoot / Run_Shoot → 노티에서 발사체 스폰
```

**이 방식을 권하는 이유**: MM DB 선택이 이미 Chooser 테이블이라 조건 컬럼 하나로 조준 로코모션이 **C++ 0줄로** 들어온다. 검 콤보 트리도 안 건드린다.
`Combo_Attack_Shoot` 4타는 **스탠스 전환식**(총 스탠스 독립)으로 갈 때 쓴다 → `DA_GunComboTree`가 하나 더 필요해지므로 **나중**.

**⑧ 애니 함정** — 몽타주와 AimOffset이 싸운다. 몽타주 Additive를 mesh space로 두면 몽타주는 나오는데 AimOffset이 깨지고, 끄면 반대가 된다. 발사 몽타주를 AimOffset 위에 얹을 때 여기서 한 번 막힌다.

---

### 발사체 리팩토링 3건 (발견만, 착수는 총 작업과 함께 — 승환 지시)

| # | 문제 | 왜 위험한가 |
|---|---|---|
1 | **델리게이트 바인딩이 `InitProjectile` 안에 있다** | 안 부르면 발사체가 벽도 적도 통과하고 5초 뒤 조용히 소멸. **에러도 로그도 없다.** 두 번 부르면 GE 2중 적용. 생성자/`BeginPlay`로 옮길 것 |
2 | **`GA_Dodge`가 발사체 발사자를 안 본다** | `IsPerfectDodgeable()`만 검사 → **플레이어 총 도입 시 내가 쏜 총알로 퍼펙트 회피가 뜬다.** 지금은 발사체가 전부 적 것이라 안 터졌다 |
3 | faction 게이트가 Enemy 한쪽만 | "적→적 통과"만 있고 플레이어 대칭 없음. 소환수·동료 생기면 터진다. 급하지 않음 |

곁가지 — `InitProjectile`에 **방향을 정하는 줄이 없다**. `ProjectileMovement`가 스폰 회전을 그대로 쓰기 때문에 스폰 시 회전을 맞춰야 한다. 적은 몽타주가 적을 향해서 우연히 맞았다. **플레이어 총은 여기가 확장 지점.**

---

### SB에 있고 우리에 없는 것 (Eve BP 컴포넌트 전수 대조)

> ⚠️ **비교의 함정** — Eve BP 함수 43개에 **전투 로직이 하나도 없다**(발소리·머리카락·물·유체·얼굴뿐). SB 전투는 통째로 C++이고 덤프에 코드가 없다. 그러니 컴포넌트 목록 비교로 "우리가 뒤졌다"고 읽으면 안 된다 — 우리가 컴포넌트로 노출한 것(`ComboComponent` `LockOnComponent` `InputBuffer` 등)을 SB는 C++에 감춘 것뿐이다. **전투 시스템의 공백은 이 덤프로 알 수 없다.**

| 항목 | 내용 | 판단 |
|---|---|---|
**카메라 스플라인 돌리 13종** | 위 참조 | **1순위** |
**콜리전 4겹 분리** | `CollisionCylinder`(이동, `ECC_GameTraceChannel11` + Pawn **Ignore**) / `BodyCollision`(피격) / `WeaponPhysicsCollision`(Box) / `CameraOverlap`(Box) / `SBCollisionGroupComponent`. 우리는 캡슐 1개 | 3순위 |
**발소리** | `FootStepL/R` + `_Reset` + `CheckPhyMat`(물리재질). **우리는 사운드가 전무하다** | **2순위** |
메시 7분할 | Face/Hair/Ponytail×2/AccSlot×2/Body. 의상·헤어 교체 | 스코프 밖 |
햅틱 2채널 | `RainHaptic` / `TachyModeVBComp`. 비 맞는 진동이 따로 있다 | 스코프 밖 |
환경 상호작용 | 물 깊이 / FluidFlux ×3 / 바람 / `AddBlood` | 스코프 밖 |
기타 | 씬캡처(포토모드) / `AssistanceSlot`(드론 슬롯) / JALI 페이셜 / `getThighAngles` | 스코프 밖 |

**콜리전 분리는 우리 구조상 쉽다** — `AT_MeleeTrace.cpp:86`이 `ECC_Pawn` **ObjectType** 기반이라, 피격 캡슐을 Pawn 타입으로 새로 달고 이동 캡슐을 커스텀 채널로 옮기면 **트레이스 코드는 그대로**다. 단 `GA_Dodge` 반경 오버랩·`LockOnComponent`·적 캐릭터를 같이 확인해야 한다.

### 2026-07-31 커밋

```
Content repo (로컬)
  4f1ac60 [VFX] 무기 트레일 NS 배정 - Distortion_Only_Loop  (25 파일)
```

문서·메모리 갱신(커밋 대상 아님): 옵시디언 `StellarBlade_Player_Analysis.md §4-6` 정정 5군데 / 메모리 `reference_sb_camera_data_2026-05-27` · `reference_control_scheme` 갱신

---

## 2. 즉시 다음 (B레인) ★ 최우선

### ✅ ① GameplayCue 스캔 경로 — 적용 완료 (소스 `a54a6a0`, PIE "잘됨")

아래는 원인 기록. 같은 종류가 또 나오면 `GameplayCue.PrintGameplayCueNotifyMap`부터.

**이펙트가 안 나오는 진짜 원인.** 에셋은 다 있는데 큐 매니저가 그 폴더를 안 뒤진다.

```
Config/DefaultGame.ini:17-19  현재
  /Game/Blueprints/AbilitySystem/GameplayCueNotify   ✅ 존재 (여기 3개만 살아 있다)
  /Game/Blueprints/Player/GC                          ❌ 폴더 없음 (길동 잔재)
  /Game/TrickalFanGame/GAS/GC                         ❌ 폴더 없음 (버터 잔재)

우리 큐 6개 실제 위치 = /Game/SB_Style_GameProject/GAS/GC/   ← 목록에 없다
```

`GameplayCue.PrintGameplayCueNotifyMap` 실측 결과 — **연결 3개 / 미연결 9개**:

| 상태 | 태그 |
|---|---|
✅ 연결 | `Enemy.Telegraph`(+3리프), `Combat.ParryClash`, `Combat.HitImpact.Light` |
❌ unmapped | `Combat.PerfectParry.SlowMo`, `Combat.PerfectDodge`, `Combat.PlayerHitConfirm`, `Combat.CounterThrust`, `Combat.HitReact`, `Combat.Staggered`, `Combat.Execution`, `Camera.Execution`, `Camera.DashTrail` |

**고칠 것**: `DefaultGame.ini`에 `+GameplayCueNotifyPaths=/Game/SB_Style_GameProject/GAS/GC` 추가, 없는 폴더 2줄 삭제. 옛 폴더 줄은 살아 있는 큐 3개가 아직 거기 있으니 **남긴다**.

**에디터 재시작 필요** (이 목록은 `AbilitySystemGlobals` 초기화 때 1회만 읽는다).
검증 = 재시작 후 같은 콘솔 명령 → `unmapped`가 숫자로 바뀌는지.

> 미확정: 각 GCN의 `GameplayCueTag`를 조회하니 6개 다 비어 보였는데 **잘 되는 옛 3개도 똑같이 비어 보였다** → 조회 방식이 부정확. 태그 문제로 단정하지 말 것. 경로만 확정 원인이고, 재시작 후에도 unmapped면 그때 BP 디테일에서 태그를 직접 박는다.

### ✅ ② 입력 버퍼 창 확대 — 적용 완료 (2026-07-30)

`BP_SBPlayer` → `InputBuffer` 컴포넌트 디테일에서 `BufferTimeWindow` 0.2 → **0.5**, `MaxBufferSize` 4 → **2**. 코드 수정 0줄(둘 다 `EditAnywhere`, `ClampMax`가 정확히 0.5).

지상 콤보 15개 중 8개에서 입력이 증발하던 문제 → **12개 해결.**
**남은 3개**: `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74)는 `ANS_CancelWindow`가 너무 늦게 열려 0.5초로도 못 덮는다 → **노티를 앞으로 당길지 별도 판단.**

---

## 3. 보류 목록 (B레인, 우선순위 순)

| # | 항목 | 내용 |
|---|---|---|
1 | **`DA_ComboTree` 값 채우기** ★ | 칸과 배선은 깔렸다(`FComboNode.InputWindow`). **값이 전부 0 = 아직 1.5초 공용값으로 돈다.** A레인 전투 수치표의 `DamageMultiplier`도 같은 상태 → **DA 한 번 열어 두 값을 같이** 넣는 게 효율적. SB 실측: 1~2타 0.7~0.8 / 3~4타 0.9~1.2 / 마무리 1.4~2.0 / 회피 0.8 / 저스트회피 1.5 |
2 | **캔슬 윈도우 늦은 몽타주 3개** | `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74). 버퍼 0.5초로도 못 덮는다. `ANS_CancelWindow`를 앞으로 당기는 게 유일한 해법 — 단 안무 자체가 후딜이 긴 동작일 수 있어 포즈 재확인 필요 |
3 | **`ANS_EnemyAttackWindow`의 `AttackWindowTag` 확인** | 헤더에 기본값이 없고 생성자도 안 넣는다 → **몽타주마다 손으로 `State.Combat.EnemyAttackHitWindow`를 넣어야** 태그가 붙는다. 비어 있으면 퍼펙트 회피가 영원히 안 뜬다. 적 공격 몽타주 전수 점검 필요 |
4 | **발사체 리팩토링 3건** | §1-B 참조. **총 작업 착수와 함께** 처리하기로 결정(2026-07-31 승환). ②번은 플레이어 총이 붙으면 확실히 터진다 |
5 | `EnterNode`가 `Context`를 안 받는다 | 트리를 지상→공중 순차 조회로 우회 중. 노드 ID가 안 겹쳐서 지금은 확실하지만, 겹치는 ID가 생기면 깨진다 |
6 | `OnInActionTagChanged` 재호출 | GA가 겹치면 `NewCount` 1→2로 재호출. 같은 소켓 재부착이라 결과 동일. **제약**: `AttachWeaponToHand()`에 1회성 작업(사운드·이펙트) 넣지 말 것 |
7 | **트레일 NS 변수 검증** | `SwordLength`/`TrailWidth`가 새 NS에 먹는지 PIE 확인. 안 먹으면 NS User Parameter 이름을 맞춰야 함 |

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

## 4. 설계 미결 (A레인) ★

코드가 아니라 **판단이 필요한** 것들. 여기가 A레인의 일감이다.

### ① 스탠스 체계
SB는 `Default(=Sword) / Tachy / Fusion / Gun계열 / Fishing / 특수(사망·동결·튜토리얼)`로 나뉜다. **평시 스탠스가 없다** — Eve는 항상 무장 상태다.
우리 초안(승환): `Default(평시 전투 = Sword&Gun) / Gun(진짜 사격만) / 사망`.
→ **사격 로직 착수 전까지 보류 결정됨**(YAGNI). 사격을 시작할 때 이 표를 확정해야 한다.

> **2026-07-31 갱신 — 이제 확정할 재료가 다 모였다.** §1-B의 사격 조사 참조.
> 팩 구성이 **두 설계를 다 지원한다**: (a) SB식 홀드 조준(임시 모드, 놓으면 검 복귀) / (b) 스탠스 전환식(검·총 대등, `Combo_Attack_Shoot` 4타 사용).
> **B레인 권장은 (a)** — 검 콤보 구조를 안 건드리고 MM Chooser 컬럼 하나로 조준 로코모션이 들어온다. (b)는 `DA_GunComboTree`가 하나 더 필요하고, 그건 아직 값도 안 채운 콤보 트리를 하나 더 이고 가는 것.
> **이 판단이 A레인 결정 대기 항목이다.**

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

> **사운드가 이제 제일 큰 구멍이다.** 트레일 27개가 붙어서 **볼 건 생겼는데 들을 게 없다.** 검이 지나가는 소리·타격음·발소리가 전부 없으면 트레일만으로는 타격감이 안 산다.
> 노티 자리는 이미 잡혀 있다(`ANS_MeleeTrace` 위치 그대로) — 사운드 노티를 얹는 작업이다. SB도 `FootStepL/R` + `CheckPhyMat`(물리재질 연동)으로 발소리를 따로 관리한다.

### ⑥ 카메라 (2026-07-31 신규)
1단계 값 표는 §1-B에 **완성돼 있다**(BP 6 + 코드 1줄, 30분). 2단계 스플라인 돌리도 실현 가능 확인됨.
**A레인 판단 필요**: 카메라를 지금 하나, 사격 뒤로 미루나. 승환은 "시점이 중요하다"고 했고, 값 이식은 30분이라 사격 전에 끼워넣을 수 있다.

---

## 5. 세션 시작 시 읽을 것

프로젝트 `CLAUDE.md §0 세션 시작 프로토콜`이 단일 진실이다. 요약:

1. `docs/PROJECT_OVERVIEW.md` — 부트 문서
2. `docs/INDEX.md` — 전체 카탈로그
3. **이 핸드오프** + 이번 작업 영역의 최신 dev-log
4. 코드 작업 세션이면 `.h` 전수 훑기(4,000줄이라 감당 가능), `.cpp`는 닿는 것만
5. **코드 설명서 = 옵시디언 볼트 `ProjectKD/notes/코드구조/`** (8문서 2,776줄) — 클래스별 용도·함수 기능·흐름·핵심 코드 발췌. `00_코드구조_MOC`부터

**에셋 값은 MCP로 조회해 확인할 것** (B레인만). 코드 기본값과 다를 수 있고, 폐기된 라인의 값이 남아 있던 사례가 실제로 있었다 — 2026-07-28 공격 GA 5개가 길동 창 소켓 `Spear_Tip`을 물고 있어 근접 판정이 죽어 있었다.

---

## 6. 오늘 배운 함정 3개 (재발 방지)

1. **`.h` 선언과 `.cpp` 정의는 짝이다.** `InAction`이 `.cpp`만 있어서 빌드가 깨졌다. 태그 추가 시 양쪽 확인
2. **구독과 해제는 같은 태그로.** `RegisterGameplayTagEvent(A).Add()` 하고 `RegisterGameplayTagEvent(B).Remove()` 하면 조용히 실패한다 — 핸들 리셋은 그대로 돌아서 코드가 깨끗해 보인다
3. **`search_assets`는 패턴을 무시하고 100개를 통째로 반환한다.** 목록·집계는 `execute_python`으로. 실측: 821개 중 100개가 돌아와 1만 토큰 낭비
