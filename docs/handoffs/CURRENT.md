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

## 🟢 2026-09-05 (2) — 워프 뒤집힘 수정 + 포폴 촬영 11개 **(새 세션은 여기부터)**

dev-log = `docs/dev-logs/2026-09-05-approach-warp-rotation-flip.md`

**적에게 등 돌리는 결함을 코드 1글자로 잡았고, 포폴 클립 11개를 새로 찍었다. 빌드·PIE 통과.**

---

### ✅ 완료 — 코드

```
저프레임 판정   ArcBulge · TraceSegments 프로퍼티 노출 (GA "Action|Weapon")
                디버그 점 2색 — 🔴 실제 프레임 / 🟡 보간 위치
                대각선 히트색 노랑 → 마젠타 (보간점과 겹쳐서)
                🔴 익명 네임스페이스 상수 2개 삭제 — 안 지우면 멤버를 가린다

워프 뒤집힘     KDGameplayAbility_PlayerMelee.cpp:172  bFollowComponent true → false
                목표점을 발동 시점 좌표에 고정
```

**🔴 워프 뒤집힘 = 게이트는 1회, 목표점은 매 프레임**

```
목표점 = 적 + 140 × (적→나 방향)   →   나→목표점 = (140 − d) × 방향
d 가 ApproachStopDistance(140) 아래로 내려가면 음수 = 목표점이 내 등 뒤로
:168 게이트가 정확히 이걸 막는 줄인데 발동 시 1회만 돈다
엔진 근거 = RootMotionModifier.cpp:400-405 · :74-86 · :127-140 (UE5.6 실측)
```

🔴 **`CLAUDE.md §1-5` 세 번째다** — 08-20 접근워프 · 08-31 반격워프 · 09-05 이번. 전부 `OnActivated` 워프 블록, 전부 *"발동 시점에 정한 것이 시간이 지나며 틀려진다"*.

⚠️ **틀린 가설 2개** — ①부채꼴 90도 밖으로 나가 다른 적 선택(화면 `dist 298` 이 근거였으나 그 값은 발동 순간 스냅샷) ②루트모션이 통과시킴(추측이었고 원인 아님). 둘 다 엔진 소스를 읽고서야 정정됐다.

### ✅ 완료 — 에셋 (MCP)

```
bDrawDebug OFF   플레이어 GA 12개 + 적 GA 7개 = 19개
                 🔴 적 GA 는 BP_ 접두사 + /Game/Blueprints/AbilitySystem/Ability/Enemy/
                    GA_* 검색으로는 안 잡힌다. DA_EnemyDef 6개의 StartupAbilities 역추적으로 찾음
GA_CounterSlash  CapsuleRadius 3.0 → 20  (08-28 신설 때 누락 — 반격이 거의 안 맞던 상태)
AutoAimFilter    LightCombo·HeavyCombo 의 bDrawDebug — 승환이 직접 (USTRUCT 라 파이썬 쓰기 금지)
Content 세이프포인트  77ac8aa · 24ba88b
```

### ✅ 완료 — 포폴 촬영 11개

`C:\Users\asdasd\Desktop\PJ_New 포폴영상\`

```
B1 훅 ver2 (97.4s)                검 궤적 + 스킬 연출 포함
B8 스킬 1·2·3·4                    신규 절
B6&B7 퍼펙트 패링 및 처형 Ver2      반격 반경 20 반영
E 근접 모드 4종                    CapusleSweep · TipLine · AriCapusleSweep · AriTri
E 프레임보간 30 · 60               ★★ 탑다운 시점 — 이번 촬영 최고 판단
```

★ **프레임보간 30/60 이 제일 잘 나왔다.** 위에서 내려다보니 궤적 전체가 한눈에 들어온다. 🔴 빨간 사각형(실제 프레임)은 30fps 가 절반인데 초록 궤적 밀도는 같다 = **자막 없이 그림만으로 전달된다.** SB 발표 05:24 와 같은 구조.

⚠️ **`E 근접 CapusleSweep` · `AriTri` 두 클립은 6초 지점에 궤적이 안 보였다.** 한 프레임만 뽑아본 것이라 판정 구간이 아니었을 수 있다 — **승환이 전체 재생해서 확인할 것.**

---

### 🔴 다음 할 일

**1단계 — 촬영 마무리**

```
□ A1 콤보          옛 클립 5개가 궤적 없음 + 창모드 + 좌상단 디버그 텍스트
                   ⚠️ 2~3개로 줄여도 된다 (기존 편집본은 4절 사용)
□ A2 분기 · A3 접근 워프   같은 이유 + 워프 수정 반영
□ C 계열 7개 확인   창모드·디버그 텍스트 있는지 → 크롭으로 될지 재촬영일지
□ A4 조준 사격 · E 총격    총알 NS 교체(NS_ArrowTrail_Basic) 반영 여부 판단
□ B4 퍼펙트 회피    반격 궤적 나오면 재촬영
```

**2단계 — 프레임 보간 절 보강 (선택)**

지금 30/60 은 **둘 다 After** 다. *"보간을 끄면 이렇게 성기다"* 대조가 없다.

```
MaxSubSteps 를 GA 프로퍼티로 노출  →  서브스텝 1(OFF) vs 32(ON) 를 촬영 중 전환
2×2 매트릭스 = (30fps · 60fps) × (보간 OFF · ON)
🔴 지금 Sweep 은 30fps 에서도 서브스텝이 8까지 늘어 구멍이 안 보인다 → 진짜 Before 는 서브스텝 1
작업 4곳 = GA .h / 태스크 .h / 태스크 .cpp(상수 2개 삭제) / GA .cpp
```

**3단계 — 워프 A안 (촬영 뒤)**

```
몽타주 MotionWarping 노티의 Warp Rotation OFF  (~21개, 파이썬으로 Notifies 못 읽어 수동)
회전 권한을 워핑에서 떼어 :147 SetActorRotation 스냅에 몰아준다 (GoW 방식)
A=방향 고정·위치 추적 / B=위치 고정·방향은 워핑 → 겹치지 않고 A+B 가 완성형
```

---

### ⚠️ 알아둘 것

```
자동조준 vs 락온 필터가 값이 정반대다 (실측)
  자동조준  Radius 800  · HalfAngle 90 · CharacterForward · Nearest(거리순)
  락온      Radius 1000 · HalfAngle 45 · Camera          · SmallestAngle(각도순)
  → 자동조준은 각도로 후보를 거른 뒤 거리만 본다. 업계 표준은 거리·각도 가중합
    (KDLockOnComponent.cpp:158-165 의 if/else 를 가중합으로 바꾸는 게 근본)

GA_SprintAttack ApproachStopDistance = 200   다른 GA 는 140. 승환 조정인지 원래값인지 미확인
GA_LightCombo TraceMode = ArcSweep           촬영 때 ArcTri 로 바꿨다가 되돌린 상태
GA_LightCombo bDrawDebug = True              촬영용. 본편 클립 찍기 전 꺼야 한다
CVar 5종   KD.ShowApproach / ShowDodge / ShowDamage / ShowKnock / StaminaGainRate
           GA 체크박스와 별개. 화면 좌상단 글자는 이쪽이다
ffmpeg 직접 사용   watch 스킬이 한글 경로에서 cp949 로 죽는다 → ffmpeg -ss -vf fps 로 우회
```

### 🧹 정리 대상

```
bOncePerActor       GA 프로퍼티인데 태스크 팩토리 인자에 없다 = 체크박스가 아무 일도 안 한다
cpp:107 죽은 분기    Activate 가 항상 bHasPrevFrame=true 로 만들어 도달 불가
TraceSweep/TraceArc  캡슐 스윕 블록이 거의 동일 = 함수 분리 대상
루프 안 TArray Hits  ArcTri 틱당 224회 힙 할당 → 루프 밖으로 빼고 Reset()
파일명 오타          E 근접 AriCapusleSweep / AriTri  (Arc Capsule)
(이월) NS_SB_Charge_02/_03 · AM_SB_Skill_03_Start/_Loop/_End · GE_StaminaCost_Dodge 등
```

---

## ✅ 2026-09-05 — 저프레임 판정 누락: 진단 + 구현

dev-log = `docs/dev-logs/2026-09-05-melee-trace-lowfps-research.md`

**승환 관측 = "프레임이 내려가면 트레이스가 안 나온다." 원인 셋을 확정하고 코드까지 넣었다. 빌드 통과. PIE 미검증.**

목표 = 포폴에 **"프레임 보간을 구현했다"** 를 넣는 것.

---

### ✅ 완료 — 코드 (빌드 통과)

```
모드 4종     Sweep · TipLine (기존, 무변경)  +  ArcSweep · ArcTri (신설)
             ⚠️ enum 은 끝에만 추가. uint8 이라 에셋에 숫자로 저장됨

공통 3건     ① Activate() 에서 Prev 프리필      태스크 :70-72
             ② TickTask -> TraceOnce() 분리     태스크 :94 · GA :183
             ③ Arc 서브스텝 상한 32             Sweep 은 8 유지 (대조군)

함수 분리    TraceOnce -> TraceTipLine / TraceSweep / TraceArc
             + ProcessHits / IsWallBlocking
             죽은 변수 bAnyHit 제거 · 로컬 StepDist 중복 제거

파일         KDAbilityTask_MeleeTrace.h / .cpp · KDGameplayAbility_MeleeTrace.cpp
```

🔴 **입력 중 버그 하나 있었다** — `bHasPrevFrame` 을 `false` 로 둬서 첫 틱이 값을 덮어쓰고 `return` 했다. **프리필이 통째로 무효**였다. `true` 로 고쳤다.

📌 `EndSocket` 이름 확정 = **`Sword_Tip`** (`KDGameplayAbility_MeleeTrace.h:57`). 종전 미확인 항목이 닫혔다.

---

### 🔴 새 세션 첫 할 일 — 순서대로

**0단계 — MCP 확인 (30초)**
```
에디터를 켠 채로 세션을 시작할 것
● MCP :3000 확인
```
⚠️ **MCP 는 세션 시작 때 한 번만 연결을 시도한다.** 09-05 세션은 에디터가 꺼진 채 시작해서 끝까지 못 붙었다. 포트는 LISTENING 이었는데도 `ConnectionRefused` 였다.

**1단계 — GA 3개에 모드 배정 (에디터)**

경로 = `Content/SB_Style_GameProject/GAS/Abilities/Attack/`
디테일 패널 → **`Action | Weapon`** → `Trace Mode`

```
GA_LightCombo     Arc Triangle Lines (SB style)      SB 방식
GA_HeavyCombo     Arc Capsule Sweep (curved)         캡슐 + 곡선
GA_SprintAttack   Capsule Sweep (whole shaft)        원본 대조군
```
⚠️ `GA_LightCombo` 는 지금 **`Tip LineTrace`** 다(09-04 진단용). 덮어쓸 것.
📌 `Draw Debug` 는 셋 다 이미 `True`.

**2단계 — PIE 측정**
```
t.MaxFPS 10        승환이 검출 실패를 확인한 조건
KD.ShowDamage 1
stat fps           안 내려가면 r.VSync 0 먼저
```

볼 것
```
① 판정이 나는가        before = 헛나감 / after = 맞아야 한다
② 모드마다 그림이 다른가  Sweep 캡슐 직선 / ArcSweep 캡슐 곡선 / ArcTri 선 격자
③ 창 양끝에 그림이 있나  없으면 ①② 가 안 먹은 것
```

**3단계 — 프로퍼티 노출 (코드, 미착수)**

승환 판단 = *"ArcBulge 는 에디터에서 조정하는 게 낫지 않나"* → **맞다.** `CLAUDE.md §3` 의 *"수치 변경 빈도 높은 값은 UPROPERTY 노출"* 에 해당한다. 처음엔 상수로 넣었으나 되돌린다.

```
노출     ArcBulge (0~5) · TraceSegments (1~10)   ->  GA "Action|Weapon"
상수 유지 StepDist 5.0 · MaxSubStepsSweep 8 · MaxSubStepsArc 32
```

작업 4곳
```
GA .h                프로퍼티 2개 추가 (CapsuleRadius 아래)
태스크 .h             팩토리 인자 2개 (기본값이라 맨 뒤) + 멤버 2개
태스크 .cpp           🔴 익명 네임스페이스의 ArcBulge · TraceSegments 상수 삭제
                     + 팩토리에서 멤버 대입
GA .cpp :166         MeleeTrace(...) 호출에 인자 2개 전달
```
🔴 **상수를 안 지우면 멤버를 가린다.** 컴파일은 되는데 에디터 값이 안 먹는 조용한 버그가 된다.

---

### 결과별 대처

| 증상 | 원인 | 조치 |
|---|---|---|
| 여전히 안 맞음 | 틱 0회 | `Activate` 프리필 · `bHasPrevFrame = true` 확인 |
| `ArcTri` 만 덜 맞음 | 두께 완충 없음 | `TraceSegments` 3 → 5 |
| 곡선이 과함 | `ArcBulge` 과다 | 1.0 → 0.5 |
| 그림이 셋 다 같음 | switch 안 탐 | GA 모드 배정 확인 |
| 선이 많아 느림 | 서브스텝 × 축분할 | `MaxSubStepsArc` 32 → 16 |

```
① 첫 구간 버림    KDAbilityTask_MeleeTrace.cpp:64-70   Prev 없어서 첫 틱은 위치만 적고 return
② 끝 구간 버림    OnTraceEndEvent 가 마지막 판정 없이 EndTask()
③ 직선 보간       :151-154  FMath::Lerp

15fps · 구간 0.167s  ->  판정된 구간 40%.   10fps 면 판정 0회
```

### 🔴 이번에 드러난 것

**① `CapsuleRadius 20` 이 곡률 오차를 덮고 있었다**
*"새 근접 GA 는 반지름 20으로 올려라"* 규칙의 정체가 이것이다. 삼각형으로 바꾸면 이 완충이 사라져 **오히려 나빠진다** — 그래서 베지어가 삼각형보다 먼저다.

**② SB 는 판정 방식이 둘이고 우리는 "정확도 낮은 쪽"이다**
```
Triangle-Hitbox Intersection   얇은 무기 · 정확       <- SB 의 검
Hitbox Sweep Trace             큰 무기 · 영역 판정    <- 우리 (발표에 "정확도 낮다" 명시)
실측 = GA CDO 10개 전부 Sweep · 노티 bOverrideTraceMode 48개 전부 False
```

**③ 삼각형만으로는 SB 도 부족했다 — 발표 순서가 증거**
삼각형 소개(04:06) → *"정확도가 떨어지는 두 경우: 빠른 공격 / 낮은 FPS"*(04:3x) → **Transform 보간**(05:00).

**④ UE5.6 에서 애니 원본 읽기가 막혔다**
```
❌ UAnimationBlueprintLibrary::GetBonePoseForTime   5.2 deprecated + Editor 모듈 = 런타임 불가
🟡 UAnimSequence::GetBoneTransform (AnimSequence.h:532)  부모 기준 Transform
   -> 루트까지 체인을 곱해야 하고 검은 hand_r 부착 StaticMesh 라 한 단계 더
✅ 우회 = 2차 베지어 (velog @hoi000115)
```

**⑤ 삼각형을 콜리전 도구로 못 쓴다 (엔진 실측)**
```
CollisionShape.h:284~316   MakeBox · MakeSphere · MakeCapsule 3종이 전부
                           MakeTriangle 없음 = 삼각형을 SweepMulti 에 못 넘긴다
                           ⚠️ MakeLine 도 없다 (LineTrace 는 별도 함수)
FMath::SegmentTriangleIntersection  UnrealMathUtility.h:2157
                           선분 vs 삼각형. 순수 수학이라 월드 콜리전과 무관
-> 우회 = 삼각형의 변을 LineTrace 로 훑는다. 이게 ArcTri 가 선 다발인 이유
```

**⑥ 캡슐 스윕은 회전을 보간하지 않는다**
```
SweepMultiByObjectType(Hits, Mid0, Mid1, Rot1, ...)   회전 인자 하나뿐
-> Rot1 자세로 고정된 채 평행이동만. 칼이 크게 돌면 근사가 거칠다
-> 삼각형은 네 꼭짓점이 실제 소켓 위치라 이 문제가 없다
```
📌 세션 중반에 *"삼각형 실익이 작다"* 고 했던 판단을 이걸로 정정했다.

### ⚠️ 알아둘 것

```
제어점 방향        회전 중심에 두면 안으로 오목.  반대(칼끝 방향)로 밀어야 바깥 볼록
                  미는 거리 = 이동거리 ÷ 5  ->  저프레임일수록 자동으로 더 휜다
velog 와 우리      InterpolationStep 5.0f · CeilToInt 나눗셈이 동일
                  다른 건 상한 8 과 Lerp(직선) 둘뿐
측정               t.MaxFPS 15 (UnrealEngine.cpp:11745) ⚠️VSync 켜지면 안 먹음 -> r.VSync 0
                  선/캡슐 개수 = 판정 횟수.  bDrawDebug 9개 이미 True
CCD                답이 아니다. 직선 발사체용. "켜도 스킵된다" 보고 있음
SB 2차 함정        IK 후 실제 본 위치 ≠ RawAnimationData.  우리도 모션워핑 써서 해당됨
```

### 🧹 정리 대상 · 미확인

```
GA_LightCombo       TipLine 상태. 1단계에서 ArcTri 로 덮으면 해소
GA_CounterSlash     CapsuleRadius 3.0 -> 20 (별건. 이번 범위 밖)
검 메쉬 직선 여부     곡도면 축 분할에 중간 소켓이 필요하다. Sword.uasset 눈으로 확인 안 함
삼각형 면 교차       선 다발로 부족하면 그때. Unreal Fest 04:00~04:30 재판독 선행
                    Gold Coast 2024 "Melee Hit Detection with No Compromises" 영상 미시청
ReadyForActivation   같은 프레임에 TickTask 가 도는지 엔진 코드 미확인
                    ⚠️ 프리필 덕에 설계엔 영향 없음
```

### 📦 미커밋 상태 (2026-09-05 종료 시점)

```
코드     KDAbilityTask_MeleeTrace.h / .cpp · KDGameplayAbility_MeleeTrace.cpp
         docs/INDEX.md · docs/handoffs/CURRENT.md
         ?? docs/dev-logs/2026-09-05-melee-trace-lowfps-research.md  (신규)
Content  GA_LightCombo (TipLine 전환, 09-04 진단용)
         ⚠️ Map/LV0_Test · GA_HeavyCombo · GA_ShotBlast = KD 가 안 건드린 것
```
📌 **빌드는 통과했으나 PIE 미검증이라 커밋 안 했다.** 2단계 측정 후 판단할 것.

### 🖼️ 볼트 요청 진행중

```
다이어그램 13장   D:\tmp\kd_diagrams\   2장 등재 완료 · 11장 요청 중
                 ⚠️ 볼트가 "원본 지워도 된다"고 했으나 11장 등재 전까진 두라고 회신함
```

---

## ✅ 2026-09-04 — 검 궤적 리본→소켓 전환 + 총알 NS 교체

dev-log = `docs/dev-logs/2026-09-04-sword-trail-ns-swap.md`

**검 궤적을 리본에서 소켓 단발 방식으로 갈아탔다. 총알도 데모팩 잔재를 걷어냈다. 승환 확인 = "좋다잉".**

```
검 궤적 27건   WeaponTrail(리본) -> PlayNiagaraEffect(소켓 단발)
               NS = NS_SB_Slash_Trail_01 · 소켓 Sword_FXSocket · off (0,0,50)
               새 트랙 "SlashFX" · 시각은 원래 WeaponTrail 시작 지점 그대로
               콤보 24(몽타주 21) · 반격 L/R 2 · 질주공격 1
스킬 13건      리본 유지 (승환 판단) - 09-03 전용 NS 와 겹치면 뭉갠다
총알           BP_Bullet 의 NS_Laser -> NS_ArrowTrail_Basic
안 건드림       Gun / Muzzle 36건 (총 머즐)
커밋           Content 미커밋 32개 - 되돌리기 = git checkout -- . (세이프포인트 b873162)
```

### 🔴 이번에 드러난 함정 3개

**① 소켓이 어느 메시 것이냐가 방식을 결정한다**
```
Sword_Bottom     검 StaticMesh 소켓        엔진 노티로 손댈 수 없다
Muzzle           총 StaticMesh 소켓        엔진 노티로 손댈 수 없다
Sword_FXSocket   캐릭터 SKM 소켓 -> hand_r  ✅
soc_fx_root      캐릭터 SKM 소켓 -> root    ✅
```
`AnimNotify_PlayNiagaraEffect` 는 **캐릭터 스켈레탈 메시에만** 붙는다. ★ **그래서 `UKDAnimNotifyState_WeaponTrail` 이 존재한다** — 무기 메시를 `WeaponMeshComponentTag` 로 직접 찾아가려고 만든 클래스다. 09-02 의 *"`Muzzle` 이 `WeaponTrail` 인 이유 = GC 가 총 메시를 못 가리킨다"* 와 같은 뿌리.
⚠️ **전환 후 궤적은 검이 아니라 손을 따라간다.** 검이 길거나 각지면 칼끝과 어긋난다.

**② `WeaponTrail` 노티는 검 전용이 아니다**
76건이 **검 40 + 총 머즐 36** 으로 갈린다. 가르는 유일한 기준이 `WeaponMeshComponentTag`(`.h:33`) 인데 **1차 보고에서 클래스 이름만 보고 전부 "검 궤적"이라 뭉갰다.** 승환이 *"총은 머즐 말고 조준 모드일 때의 발사체"* 라 짚어줘서 갈랐다.
📌 교훈 = **이름이 좁은데 용도가 넓어진 클래스는 일괄 변경의 함정. 기준 필드를 먼저 찾아라.**

**③ BP 컴포넌트 편집 통로 = `SubobjectDataSubsystem`**
```
❌ bp.get_editor_property("simple_construction_script")   Blueprint 에 없는 프로퍼티
❌ MCP inspect set_component_property                     actorName 요구 = 월드 액터 전용
✅ unreal.SubobjectDataSubsystem
     k2_gather_subobject_data_for_blueprint -> k2_find_subobject_data_from_handle
     -> SubobjectDataBlueprintFunctionLibrary.get_object / get_variable_name
     -> set_editor_property -> compile_blueprint -> save_asset
```

### 🔴 다음 할 일

```
1  총알 원소 확정      Basic 이 안 맞으면 Fire / Ice / Holy / Magic / Nature / Water / Basic_02
                    ⚠️ 속도 4000 이라 꼬리가 짧으면 눈에 안 걸릴 수 있다
2  무기 IK 스냅       히트스톱 순간 검을 타격 지점에 붙이기
                    SB 히트스톱 0.07 = 우리 0.08 과 거의 동일
3  Vertex Shake      머티리얼 WorldPositionOffset. 코드 0줄
                    UHitFeedbackComponent 의 Bone Shake 와 짝
```

### ⚠️ 알아둘 것

```
총 발사체 경로      GA_Shoot -> BP_Bullet (참조자 그것 하나) · 속도 4000 · 중력 0 · 수명 3초
                  Bullet StaticMeshComponent 는 비어 있다 = 보이는 건 나이아가라뿐
NS_Laser 컴포넌트   이름은 그대로 두고 내용물만 바꿨다. 개명하면 참조 노드가 끊길 수 있다
파이썬 API 벽 4개   AnimMontage.Notifies · AnimNotify.export_text · NiagaraSystem 내부
                  · Mesh.Sockets  전부 protected 또는 부재
                  -> 노티 시각은 FAnimNotifyEvent.export_text 의 LinkValue
                  -> 소켓은 find_socket(name) 으로 하나씩 두드린다
                  -> NS 가 리본인지 단발인지는 PIE 눈 검증만
일괄 투입 크래시    09-02 기록 때문에 몽타주 하나씩 처리 + 즉시 저장 -> 무사 완주
```

### 🧹 정리 대상

**이번에 생긴 것**
```
빈 WeaponTrail 27건    NS=None 인 껍데기. 동작에 무해하나 타임라인이 지저분하다
                      ⚠️ 지우면 되돌리기가 어려워지므로 방식이 확정된 뒤에
파라미터 잔류           그 껍데기에 Lifetime_Trail 0.12 · Trail Width 200 이 남아 있다
                      NS 가 None 이라 무동작. 껍데기와 같이 정리
NS_Laser 컴포넌트명     내용물이 NS_ArrowTrail_Basic 인데 이름이 안 맞는다
```

**이월**
```
NS_SB_Charge_02 / _03                            유저 파라미터 방식 전환으로 미사용
AM_SB_Skill_03_Start / _Loop / _End              참조자 0 인 고아 3개
GE_StaminaCost_Dodge · GE_FullSprintStaminaCost  참조자 0 (회피·질주 소모 없음)
KDGameplayTags.h:95 주석                          UKDStaminaComponent 는 실재하지 않는다
취소 시 카메라 줌 잔류                              End 섹션을 안 지나는 경로. 재현 확인 후 판단
```

---

## ✅ 2026-09-03 (2) — 스킬 카메라 · 슈퍼아머 · 스태미나 개편

dev-log = `docs/dev-logs/2026-09-03-skill-camera-superarmor-stamina.md`

**스킬 연출이 끝났고, 스태미나가 "때려서 버는 자원"이 됐다. PIE 통과.**

```
카메라      Skill_02  CAS_Skill_02_Impact  (VFX·슬로우·쉐이크와 같은 60프레임)
            Skill_03  CAS_Charge  단계별 줌 + FOV  ·  End 섹션에서 복귀
            Skill_01 · 04 는 연출 없이 현행 유지 (승환 판단)
슈퍼아머     GA_HitReact Blocked + 스킬 4개 Owned  =  BP 5개 · 코드 0줄
스태미나     리젠 제거 → 평타 명중 시 피해 x 0.2 회복
            스킬 10 소모 · 회피·질주 소모 0
커밋        코드 a316cae · a2cc24c / Content ea7e294 · 298cf68 · 4073a16 · b873162
```

### 🔴 이번에 드러난 함정 3개

**① `ASC::MakeEffectContext` 는 `SetAbility` 를 안 한다**
`UGameplayAbility::MakeEffectContext` 만 `SetAbility(this)` 를 부른다. `KDAbilityStatics.cpp:103` 이 ASC 쪽을 써서 **`GetAbility()` 가 항상 nullptr** 이었다. 리젠이 가려주고 있었을 뿐 **처음부터 스태미나가 안 차고 있었다.**

**② 카메라 애니메이션 FOV 는 절대값이 아니다**
`ResetDefaultValues` 가 **매 프레임 현재 게임 FOV 에서 기준 초점거리를 역산**한다. 게임 FOV 가 낮으면 같은 값이 **반대로(넓어짐)** 작동한다. 락온 중엔 `DefaultFOV` 75 고정이라 그때 테스트할 것.
⚠️ **시네카메라 센서폭은 36mm 가 아니라 23.76mm.** 15.5mm 가 75도 기준선.

**③ 슈퍼아머 태그가 반쪽만 깔려 있었다**
`EnemyHitReact.cpp:21` 이 차단은 하는데 **부여하는 GA 가 17개 중 0건**이었다. 스킬이 끊기던 건 GAS 취소가 아니라 **리액션 몽타주가 같은 슬롯을 덮어서**다.

### 🔴 다음 할 일 (2026-09-04 승환 지시로 순서 변경)

```
1  연출 폴리싱          ← 여기부터
   1-1  총 발사 나이아가라   새로 추가한 것 중 어색한 것 수정
   1-2  검 공격 나이아가라   수정
2  무기 IK 스냅          히트스톱 순간 검을 타격 지점에 붙이기
                        SB 후보 2순위. SB 히트스톱 0.07초 = 우리 0.08 과 거의 동일
                        ⚠️ 검이 안 닿거나 지나쳐서 멈추는 문제를 IK 로 보완하는 것
3  Vertex Shake         머티리얼 WorldPositionOffset. 코드 0줄
                        UHitFeedbackComponent 의 Bone Shake 와 짝. SB 후보 1순위
```

📌 **폴리싱을 앞으로 뺐다.** 스킬 연출을 막 끝낸 상태라 눈이 맞춰져 있을 때 손보는 게 낫다.
📌 SB 조사 기준 비용 순서는 `Vertex Shake → IK` 였으나 **승환이 IK 를 먼저**로 정했다.

### 🧹 정리 대상

```
NS_SB_Charge_02 / _03                   유저 파라미터 방식으로 바뀌며 미사용
AM_SB_Skill_03_Start / _Loop / _End     참조자 0 인 고아 3개
GE_StaminaCost_Dodge · GE_FullSprintStaminaCost   참조자 0 (회피·질주 소모 없음)
KDGameplayTags.h:95 주석                UKDStaminaComponent 는 실재하지 않는다
취소 시 카메라 줌 잔류                    End 섹션을 안 지나는 경로. 재현 확인 후 판단
```

### ⚠️ 알아둘 것

```
KD.StaminaGainRate  0.2 확정. PIE 콘솔로 즉시 조정 가능
KD.ShowDamage 1     피해량 표시 - 회복 계산 검증에 같이 쓴다
TemplateSequence    플러그인 수동 활성 필요. 에셋 메뉴는 Cinematics 아래
StopAllCameraAnimationsOf  시퀀스 에셋만으로 정지 = 핸들 저장 불필요
GetAbility()        복제 X. 멀티 전환 시 GE AssetTag 방식으로 교체
```

### 🤖 세션 마감 자동화 (2026-09-04 신설)

**Stop 훅이 마감 시점을 감지하면 묻지 말고 `wrap-up` 스킬을 실행한다.** 규칙 = `CLAUDE.md §0` · 스킬 목록 = `§7`.

```
.claude/hooks/wrapup-nudge.sh      감지 조건 3개 (AND)
                                     ① 미커밋 변경 있음 (두 repo 합산)
                                     ② 마지막 커밋 후 25분 경과
                                     ③ 오늘 날짜 dev-log 없음
.claude/skills/wrap-up/SKILL.md    마감 6단계 + 하청 경계
.claude/settings.local.json        Stop 훅 등록
```

🔴 **`.claude/` 는 gitignore 라 repo 에 없다**(§5-3 로컬 개인 자산). 날아가면 위 3파일을 다시 만들어야 한다.

⚠️ **안 뜨는 게 더 흔한 실패다.** 조건 셋을 다 통과해야 한다. 마감할 때가 됐는데 조용하면 직접 확인:
```bash
bash .claude/hooks/wrapup-nudge.sh     # 출력 없으면 조건 미충족
```

📌 커밋·푸시는 **반드시 승인**받는다. 문서 갱신까지만 무승인.

---

## ✅ 2026-09-03 (1) — 스킬 나이아가라 연출 (Skill_01·02·03)

dev-log = `docs/dev-logs/2026-09-03-skill-charge-vfx.md`

**스킬 1·2·3 에 연출이 붙었다. PIE 통과. Skill_02 는 VFX·슬로우·쉐이크 3종 완비.**

```
Skill_01   NS_SB_Telegraph_Red      t=0.100  Z=100      방사형 링
Skill_02   NS_SB_Free_Magic_Circle2 t=0.999             Ray·Mesh1 만 사용
           WindupSlow               0.999~1.241  0.2배
           CameraShake_SB_XL        t=0.999
Skill_03   GCN_SkillCharge -> NS_SB_Charge_01           홀드 단계별 색
           C++ 4파일 +61줄 · GameplayCue.Skill.Charge 신설
           ChargeSteps 0 / 3 / 5 · MaxHoldTime 6
커밋       코드 7567920 · 8ad0584 / Content b1bdb60 ~ 6c73630 (8개)
```

### 🔴 홀드 중에는 AnimNotify 가 전부 죽는다

`UKDGameplayAbility_SkillCharge` 가 진입 섹션 끝(0.167s)에서 `Montage_Pause` 를 건다. 홀드 구간엔 몽타주 시간이 안 흘러 **노티가 에러도 로그도 없이 하나도 안 터진다.** 그래서 Skill_03 만 **GA 타이머 → GameplayCue** 경로로 갔다. Skill_01·02·04 는 해당 없음.

### ★ 슬로우와 나이아가라 길이는 묶여 있다

`WindupSlow` 는 `Montage_SetPlayRate` 로 **몽타주만** 늦춘다. 나이아가라·카메라 쉐이크는 **실제 시간**으로 흐른다.

```
필요 NS 길이 = 구간 애니 길이 ÷ SlowRate = 0.242 ÷ 0.2 = 1.21초 (60fps 73프레임)
🔴 SlowRate 를 먼저 확정하고 NS 를 자른다
📌 사라지는 시각 = Loop Duration + Lifetime
⚠️ SlowRate 하한 0.05 — 0 이면 NotifyEnd 가 영영 안 온다 (완전 정지는 UKDHitStopComponent)
```

### 🔴 다음 할 일

```
1  카메라 연출 나머지    Skill_02 만 AN_CameraShake 붙었다. 01 · 03 · 04 는 없음
                       Effect/CameraShake/CameraShake_SB_{SS,S,M,L,XL,SP} 대기 중
2  ★ 원소 톤 결정       아직 미결. Hit · Sword · Aura 가 세트로 따라온다
                       후보 = Scifi · Lightning · Mystic · Dark · Sand
3  DA_HitSkillAttack   ImpactVFX 를 평타와 다른 원소로 + CameraShakeClass
                       GA_Skill_01~04 · GA_AreaBlast 의 HitConfirmProfile 교체. 코드 0줄
4  ST 리젠 제거         설계 예정 (아래 절)
```

### 🧹 정리 대상

```
NS_SB_Charge_02 / _03            유저 파라미터 방식으로 바뀌며 미사용
AM_SB_Skill_03_Start/_Loop/_End  참조자 0 인 고아 3개
NE_Chromatic (Charge_01)         색수차라 User.LinearColor 를 안 따를 가능성
```

### ⚠️ 알아둘 것

```
GC BP 경로       DefaultGame.ini:19-20 스캔 경로 안이어야 한다. 밖이면 조용히 안 잡힌다
Niagara 노드     Set Niagara Variable "By String" 은 5.3 deprecated. FName 버전 쓸 것
NS 수명          Loop Duration · Lifetime 은 MCP·파이썬으로 못 읽는다 = PIE 눈 검증만
Select 노드      출력을 먼저 연결해야 Wildcard 타입이 굳는다
AuraFX 구분      SwordLength = 검용 / Sphere Radius = 몸용 (이름은 다 같은 "Aura")
```

---

## ✅ 2026-09-02 (저녁) — 우하단 스킬 슬롯 UI

dev-log = `docs/dev-logs/2026-09-02-skill-slot-ui.md`

**스킬 4종에 UI가 붙었다. 코드 0줄 — 텍스처·머티리얼·위젯만. PIE 통과.**

```
아이콘 4장   gpt_image_2 라인아트. 배경 순검정 -> Additive 머티리얼로 알파 불필요
머티리얼     M_UI_SlotRing (텍스처 0장, UV 로 원 계산) · M_UI_SkillIcon
            인스턴스 3 = MI_SlotRing / MI_KeyBadge / MI_SlotDim  + MI_Icon_Skill01~04
위젯        WBP_SkillSlots 십자 4칸 + 숫자 배지 4 -> WBP_MainHUD 우하단
쿨다운      원형 스윕 (Progress 파라미터 · 위젯 애니메이션 머티리얼 트랙)
커밋        Content 6054ede · 2073d4a · 088c899 · eed3fc9 · 4192a94 · a0f9e11 · a088ee6
```

> ✅ **슬롯 크기 확대 · `Skill_04` 아이콘 — 둘 다 종결 (2026-09-02 승환).** 크기는 직접 재계산, 아이콘은 현행 유지.

### 🔴 다음 할 일 — 스킬 연출 + 나이아가라

```
1  ★ 원소 톤 결정       Hit · Sword · Aura 가 같은 이름 체계라 하나 정하면 세트로 따라온다
                      후보 = Scifi(푸른 기계톤, SB 에 가장 가깝다) · Lightning · Mystic · Dark · Fire
                      -> 게임의 색을 정하는 결정. 연출의 첫 갈림길
2  DA_HitSkillAttack   ImpactVFX 를 평타(NS_Hit_Basic_Once)와 다른 원소로
                      + CameraShakeClass = LCS_CounterAttack
                      GA_Skill_01~04 · GA_AreaBlast 의 HitConfirmProfile 교체 + Mag 1.5
                      ★ DA 하나 만들고 BP 5개 값만 바꾸면 끝. 코드 0줄
3  땅 찍기 충격파       Vefects 의 VFX_Shockwave_* 를 AreaBlast 판정 위치에 스폰
                      ⚠️ 반경 300 에 맞는 스케일은 눈으로 봐야 안다
4  스킬 궤적 차별화     지금 스킬 4개가 전부 평타와 같은 NS_SlashTrail_Basic_Loop
                      NS_Slash_XL / SP01 등으로 갈라면 노티 값만 바꾸면 된다
⛔ 슬로모는 안 넣는다   (승환 판단 — 내 시간이 느려지는 건 스킬 성격에 안 맞다)
```

**나이아가라 재고 = 살 것이 없다.** dev-log `2026-09-02-skill-slot-ui.md §8` 에 전수.
```
GhostSamurai 18 (Slash_Ground/Fall/XL/SP01/PowerUp_Burst...) · SlashTrail Hit 32 (원소 16 × Loop/Once)
Sword 오라 13 · AuraFX 16 · Vefects Shockwave 84 (형태 4 × 크기 3 × 색 12)
```

### 🟡 설계 예정 — ST 리젠 제거

승환 방향 = **스태미나 리젠을 없애고 명중 시 회복.** 스태미나가 "시간이 주는 자원"에서 **"때려서 버는 자원"**이 된다. 스킬을 쓰려면 먼저 평타를 맞혀야 하니 공방 리듬이 생긴다.
📌 방금 만든 스킬 UI가 그 정보를 보여주는 자리가 된다 — 지금도 스태미나 20 미만이면 4칸이 흐려진다.

### ⚠️ 알아둘 것

```
MCP import      프로젝트 밖 파일 거부(SECURITY_VIOLATION). unreal.AssetImportTask 파이썬으로
UI 텍스처       Compression UserInterface2D(=TC_EDITOR_ICON) · Group UI · NoMipmaps · MaxSize 256
Image_Dim       Render Opacity 는 1.0. 안 보이는 건 MI_SlotDim 의 Progress=0 담당
StaminaCost 20  GA CDO 와 위젯 변수 두 곳에 적혀 있다. 갈리면 UI 가 거짓말한다
```

---

## ✅ 2026-09-02 (오후) — 땅 찍기 AoE + 스킬 연출 배선

dev-log = `docs/dev-logs/2026-09-02-area-blast-ga.md`

**스킬 4종이 판정·궤적·소리를 갖췄고, 스킬2에 땅 찍기 AoE 가 붙었다. 빌드 0/0 · PIE 통과.**

```
신규 클래스   UKDGameplayAbility_AreaBlast     범위 판정 (원기둥 | 부채꼴)
태그 2개      Ability.Player.AreaBlast · Event.Montage.AreaBlast
노티          C++ 신설 X — 범용 BP AN_SendGameplayEvent 가 이미 있었다
에셋          GA_AreaBlast_GroundSlam · 스킬 몽타주 4개에 연출 노티 44개
커밋          코드 bcd5a2e · 6ae6d3a   Content 037218b · 68d1b4f · 5bd49df
```

### 🔴 다음 할 일 (순서)

```
1  bDrawDebug 끄기       GA_AreaBlast_GroundSlam CDO. 검증용으로 켜둔 상태
2  스킬 연출              스킬 전용 HitConfirmProfile · 슬로모
                        ⚠️ AreaBlast 는 PlayerMelee 자식이 아니라 슬로모 칸 3개가 없다
3  우하단 스킬 UI          SB식 슬롯
```

### 🟡 눈·귀로 판정할 것 2건

```
Skill_03 소리 밀도     0.9초에 검격 6 + 총성 6. 뭉개지면 줄인다
                     ⚠️ Concurrency 의 Prevent New 가 뒷소리를 죽일 수 있다
Skill_03 트레일 겹침    궤적 6개가 서로 물린다. 하나로 길게 뽑을지
```

### ⚠️ 알아둘 것

```
Build.bat        컴파일 에러에도 exit 0. Result: Failed 문자열로 판정할 것
Muzzle 노티      세 값이 한 세트 — ns · socket · weapon_mesh_component_tag(기본값 Sword)
                Shot 을 옮겨도 안 따라온다. 트랙 통째 재생성이 해법(멱등)
새 GA BP         Cooldown / ActivationBlockedTags 를 비울 것
                스킬이 Cooldown.Player.Skill 과 State.Combat.Attacking 을 이미 들고 있다
PoiseDamage      적 DA 에 Skill1~4 · CounterSlash · AreaBlast 키가 없다 = 경직 X
```

---

## ✅ 2026-09-02 (오전) — 캐릭터 스킬 4종 구현 완료

dev-log = `docs/dev-logs/2026-09-02-skill-charge-ga.md` (직전 = `2026-09-01-skill-anim-notify.md`)

**스킬 1·2·3·4 가 키 입력으로 돌아간다. PIE 전반 통과(승환).**

```
신규 클래스   UKDGameplayAbility_Skill        단발 (스킬 1·2·4)
             UKDGameplayAbility_SkillCharge  차지 (스킬 3)
태그 6개      Ability.Player.Skill1~4 · Cooldown.Player.Skill · Event.Skill.HoldRelease
입력          숫자키 1~4 + 차지 키 뗌.  Controller → Character → AbilityInputComponent
에셋          GA BP 4개 · GE_SkillCooldown · IA 4개 · AM_SB_Skill_03 통합(섹션 5개)
             + GA 폴더 재편 15개 이동 (Attack/Guard_and_Parry/Move/Shot/Skill)
커밋          코드 b90beb5·1e2ca7b·1af29e5 (푸시) / Content 3b2a424·13038aa·d799c85
```

### 확정 사양

```
캔슬 없음 · Stamina 20 소모 · 쿨다운 1초(4개 공유 태그) · 상단 숫자열 1~4
차지 3단계 = 0.4초 / 0.8초 기준.  이동 706 / 806 / 979cm
```

> ✅ **스킬2 땅 찍기 AoE 는 오후에 완료** — 위 절 참조. 노티는 1.417 이 아니라 **1.008 초**에 넣었다(승환 의도).

### 🟡 눈으로 보면 끝나는 것 2건

```
Charge1 (몽타주 f24.35) 점프 지점    포즈차 59도로 실측된 자리. 튀면 f35 근처로 옮긴다
                                    (f35 = 21.6도. 노티 안 걸리는 구간 중 최선)
첫 Shot (0.1731초)                  정지 시점(0.1667) 6ms 뒤. 차지 시작에 한 발 나갈 수 있다
                                    어색하면 노티를 0.19 로 민다
```

### ⚠️ 알아둘 것

```
PoiseDamageByAttack     적 DA 맵에 Skill1~4 항목이 없다 = 스킬로 경직이 안 걸린다
                        (CounterSlash 도 같은 상태. 적 밸런싱 때 같이)
Montage_Pause           SkillCharge 가 프로젝트 유일 사용처.
                        히트스톱을 몽타주 정지 방식으로 되돌리면 여기가 깨진다
CapsuleRadius           헤더 기본 3. 새 근접 GA BP 만들 때마다 20 으로 올려야 한다
```

---

## 🔵 2026-08-28 — 플레이어 폴리싱: 패링 재설계

승환 지시 = **"패링이 구리다. 1부터 100까지 손봐야 한다"** + **"SB 레퍼런스니까 SB 대로 가고 싶다"**.
SB 덤프 전수 조사 + 우리 현행 전수 실측을 마쳤다. **아직 착수 전 — 이건 계획이다.**

SB 실측 전문 = 볼트 브릿지 `SB 노트 신규 등재 요청 — 패링·막기 체계와 연출 전수` (2026-08-28)
우리 현행 실측 = 아래 §진단

### 진단 — 부품이 없는 게 아니라 사슬이 끊겨 있다

```
애니       Block 5종 · Parry_L/R · Parry_Counter_Attack_L/R  전부 배선됨. 부족하지 않다
연출       GCN_PerfectParry(노드 21) · LCS_PerfectParry · GA_PerfectParryReaction(노드 13)
           전부 배선돼 있다.  ⚠️ 하청이 "비어 있다" 고 오판했으나 그래프 이름이
           EventGraph 가 아니라 "Gameplay Ability Graph" 라서 못 찾은 것
창         GE_PerfectParry Duration 0.5   <- SB 0.15 의 3.3배
보상       퍼펙트 패링 = 데미지 0 + Poise 4.0 이 전부. 파생기 없음
```

🔴 **핵심 = 퍼펙트 패링이 막다른 길이다.**
`State.Combat.CounterReady` 를 부여하는 곳이 **`GA_Dodge` 하나뿐**(소스 grep 확인).
즉 **반격은 퍼펙트 회피에만 붙어 있고 퍼펙트 패링에는 없다.**
그런데 `GA_CounterThrust` 가 재생하는 몽타주 이름이 `AM_SB_Parry_Counter_Attack_L` 이다 —
**패링용 애니를 회피 반격이 쓰고 있고, 짝인 `_R` 은 참조 0건으로 놀고 있다.**

성공이 흔하고(창 0.5) 성공의 출구가 없다(파생기 0). 세 축이 각각 고장난 게 아니라 한 뿌리다.

### SB 대조 (실측)

```
                  SB                          우리
입력 구조         홀드 하나 + 내부 타이밍 분기    같다 (갈아엎을 필요 없음)
저스트 창         0.15                        0.5
막기의 대가       실드 소모 -> 가드 붕괴 4초     없다. 고정 50% 감소만
저스트 보상       격파 -1 + 반격기(2.5~10.4)    데미지 0 + Poise 4.0
일반 막기 연출     전무                         전무 (구조 일치)
슬로모            0.25 · 0.15초 · 진입 0.025    0.3 · Timeline 커브(길이 미확인)
진동 / 카메라랙제어  있음                        둘 다 없음
```
⛔ **못 가져오는 것 3종** — SB 에셋 자체(저작권) / `CS_PC_JustParry_01` 쉐이크 수치(별도 uasset,
못 읽음) / 진동(패드 전제). **가져올 수 있는 건 타이밍·배율·구조**이고 손맛의 대부분이 거기다.

---

### 🔴 작업 순서

**0단계 — 확인 하나 (승환, 5분)**
```
GCN_PerfectParry 의 TimeLine 노드 커브 길이를 본다
```
`Set Global Time Dilation 0.3` -> Timeline -> `1.0` 구조인데 **Timeline 길이를 파이썬으로 못 읽는다**
(커브는 파이썬에 안 열린다 — 2026-07-31 함정 4번). SB 는 0.15초.
우리 게 0.5초쯤이면 증상이 "밋밋하다" 가 아니라 **"늘어진다"** 다. 1단계 내용이 여기서 갈린다.

**1단계 — 값 맞추기 (에셋만, 반나절)**
```
GE_PerfectParry    Duration 0.5 -> 0.15
GCN_PerfectParry   Time Dilation 0.3 -> 0.25 / Timeline 0.15초 / 진입 즉발(SB BlendIn 0.025)
LCS_PerfectParry   Duration 0.2 · BlendOut 0.1 유지 / FOV 진폭 3 -> 15~20 시험
```
★ **1단계가 진단 도구를 겸한다.** 여기서 PIE 를 돌리면 두 갈래로 갈린다.
```
재밌어졌다  -> 원인은 창이었다. 2단계로
너무 어렵다 -> 원인은 "적 공격이 언제 오는지 안 보인다" -> 5단계를 먼저
```

**2단계 — 반격 사슬 (핵심. 여기가 제일 크다)**
```
2-1  퍼펙트 패링이 CounterReady 를 부여하게
     KDCombatAttributeSet.cpp:118 이 이벤트만 쏘고 끝나는 자리에 GE 를 하나 더
     기간 = SB 의 P_Eve_JustParry LifeTime 0.1초 참고
     ⚠️ 우리 입력 버퍼가 0.8초라 조율 필요
2-2  반격기를 Light / Strong 두 갈래로
     GA_CounterThrust 재사용 + 놀고 있는 AM_SB_Parry_Counter_Attack_R 을 Strong 에
     계수 = SB Light 2.5 / Strong 5.0
2-3  Poise 차감을 저스트 전용으로   ⚠️ 4단계와 묶여 있다 (아래)
```

> ### ✅ 2단계 완료 (2026-08-28) — 사슬이 이어졌다
> ```
> 2-1  GE_PerfectParry_Counter 신설(HasDuration 1.0 · State.Combat.CounterReady)
>      GA_PerfectParryReaction 에 ApplyGameplayEffectToOwner 추가
>      Send Gameplay Event to Actor -> [여기] -> Execute GameplayCue On Owner
>      C++ 0줄. PIE 통과
> 2-2  Ability.Player.CounterSlash 태그 + UKDGameplayAbility_CounterSlash 신설
>      (부모 = CounterThrust. 생성자에서 AbilityTags.Reset() 후 새 태그만)
>      TryHeavyAttack() 에 CounterReady 분기 — 캔슬 판정보다 앞
>      GA_CounterSlash BP = AM_SB_Parry_Counter_Attack_R. PIE 통과
> ```
> ⚠️ `AM_SB_Parry_Counter_Attack_R` 은 **노티 0개**다. 재생만 되고 판정이 없다.
> `_L` 을 참고해 찍어야 한다 — 길이가 95f 로 같아 같은 시각이 대체로 맞는다.

### 🟡 보류 — 패링 반격과 회피 반격을 나눌 것인가 (2026-08-28 승환 제기)

지금은 `State.Combat.CounterReady` 하나를 **패링과 회피가 공유**한다. 좌/우 반격도 공용이다.
**SB 는 나눠져 있다** — 반격기에 `CheckActiveEffectAliasArray=['P_Eve_JustParry']` 게이트가
걸려 있어 회피는 그 파생을 못 쓴다.

🔴 **지금 균형이 어긋나 있다.**
```
퍼펙트 패링   창 0.15초 — 시간으로 재는 좁은 창
퍼펙트 회피   적 ANS_EnemyAttackWindow 태그가 켜진 동안 — 공격 모션 길이만큼
-> 패링이 훨씬 어려운데 보상이 같다
```
⚠️ 회피 창이 실제로 몇 프레임인지는 **안 재봤다.** 노티가 방금 살아났으니 이제 잴 수 있다.

**지금 안 나누는 이유 = 애니 재고.** 반격 애니가 `AS_Parry_Counter_Attack_L/R` 둘뿐이고
이름부터 패링용이다. 패링만 떼어가면 **회피 반격이 사라진다** = 지금보다 나빠진다.

**나누려면 필요한 것**
```
① 태그 분리    State.Combat.ParryCounterReady 신설. 오늘 한 것과 같은 모양
② 회피용 애니   미사용 재고에서 배분 (AS_Skill_02 105f · AS_Skill_04 155f 등)
               ⚠️ 스킬용으로 찜해둔 것들이라 지금 빼 쓰면 스킬 재고가 준다
```

📌 **착수 시점 = 4단계 뒤.** 지금 나누면 회피에서 뺏는 모양이라 손해만 본다.
막기에 대가가 붙어 패링 가치가 올라간 뒤에 나눠야 "패링이 더 좋다" 가 자연스럽다.

**3단계 — 연출 이식 (2단계 뒤에)**
축하할 사건이 생긴 다음에 붙여야 값어치가 있다. 지금 하면 허공에 터진다.
```
3-1  반격기에 연출 몰아주기   SB 가 그렇게 한다. 저스트 자체보다 반격이 화려하다
     Light  FOV진폭 -70 · TimeScale 0.2(0.15초) 뒤 0.3(0.2초)  두 번 건다
     Strong FOV진폭 -70 + 위치쉐이크 · TimeScale 0.1(0.15초) 거의 정지
3-2  CustomTimeDilation 1.5   슬로모 중에도 파티클만 빠르게 (나이아가라 파라미터 하나)
3-3  화면 효과                우리는 MPC 방식 — M_PP_ScreenEffect 에 파라미터 추가
3-4  카메라 랙 동적 제어       스프링암에 랙은 이미 있다. 값 갈아끼우는 함수 + 태그 구독
     SB 반격기 = TargetSpeed 1.0 · MaxDistance 300 · 0.35초 (평시 19)
```

> ### ✅ 4단계 ① 완료 (2026-08-28) — 빌드·PIE 통과 · dev-log `2026-08-28-parry-redesign.md`
> ```
> 커밋        코드 11db1ff · 에셋 9276fac
> 실드 리젠    GE_ShieldRegen_InCombat 신규 (Require InCombat · 초당 0.4)
>             기존 GE_ShieldRegen 은 Ignore InCombat · 초당 3.0 유지 = SB 8배 비율
> 리다이렉트   PropertyRedirects 2줄 걸고 -> 강제 저장 -> 바이너리 0건 확인 -> 삭제 완료
> 잡은 버그    ① return 이 Absorbed 대신 AbsorbRate  ② DA 에 옛 값 10 이 넘어와 데미지 음수
> ```
> **다음 = 4단계 ② 가드 붕괴.** 실드 0 이면 지금은 "막아도 데미지가 다 들어옴" 까지만 된다.
> ⚠️ 감지 위치가 함정 — `PostGameplayEffectExecute` 는 "사후 로직 금지" 주석이 붙은 자리다.
>
> <details><summary>착수 시점 기록 (접힘)</summary>
>
> ### 🔵 4단계 ① 진행 중 (2026-08-28) — 코드 완료 · **빌드 대기**
> ```
> 어트리뷰트 3개     Defense 삭제
>                   DamageReductionRate      비율 (0~1 클램프)   <- Defense 대체
>                   ShieldAbsorbRate         평소 실드 흡수율 0.4
>                   BlockShieldAbsorbRate    정면 방어 중 0.8
> 계산              Damage - Defense   ->   Damage x (1 - DamageReductionRate)
>                   흡수율을 bBlocked 로 고른다 (방향 판정 유지 — 등 뒤는 막기 무효)
> DA 필드           KDEnemyDefinitionDataAsset.Defense -> DamageReductionRate (ClampMax 1.0)
> ini               PropertyRedirects 2줄 (임시 — 에셋 3개 재저장 후 삭제)
> ```
> **왜 비율로 바꿨나** — SB 에 뺄셈식 방어력이 없다(덤프 실측). `CharacterTable` 의 감소 컬럼이
> 전부 `~Rate` 다. 그리고 뺄셈은 약한 공격일수록 손해가 커서, 데미지 10짜리를 만들면
> `Defense 10` 인 엘리트에겐 통째로 0이 된다.
>
> 🔴 **빌드 후 반드시 확인할 것**
> ```
> DA_Axe_Elite · DA_Parry_Bandit   Defense 10 이 그대로 넘어오면 ClampMax 1.0 에 걸려
>                                  1.0 = 데미지 완전 무효 = 엘리트 무적이 된다
>                                  -> 0.25 로 고친 뒤 PIE 를 돌릴 것
> GE_InitPlayerStats               Modifier 가 DamageReductionRate 를 가리키나
>                                  ⚠️ 어트리뷰트 PropertyRedirects 가 먹는지는 미확인.
>                                     빈칸이면 손으로 다시 지정 (값 0이라 잃을 것 없음)
> 검증                             안 막고 40%/60% · 막고 80%/20% · 실드 0이면 전부 HP
>                                  · 등 뒤에서 맞으면 막고 있어도 40%
> ```
> 📌 곁들여 버그 하나 잡음 — `return FMath::Max(Mitigated - AbsorbRate, ...)` 가
> `Absorbed` 여야 했다. 그대로 뒀으면 실드만 닳고 HP 는 거의 다 받았을 것이다.
>
> </details>

**4단계 — 막기의 대가 ✅확정 (2026-08-28 승환 = "나도 SB 처럼")**

> 🔴 **종전 판의 "조건부 · 비싸다" 는 과대평가였다. 정정한다.**
> 근거 = 적 DA 6개 Shield 실측(아래). **Shield 를 가진 건 셋뿐이고 셋 다 막을 줄 아는 존재다.**
> 값을 안 가진 적이 넷이라 바뀔 게 별로 없다. `.cpp` 두 자리 + GE 하나 + 몽타주 하나 = **하루짜리**.

```
① ApplyMitigation 에서 Shield 흡수 블록 제거      KDCombatAttributeSet.cpp:145~157
② bBlockedHit 분기에서 Shield 차감                같은 파일 :60~61 근처
③ Shield 0 -> 가드 붕괴 태그 + GE + 몽타주        AM_SB_Block_Hit_Break 가 이미 있다
                                                 ⚠️ 실제로 쓰이는지는 미확인
④ Shield 회복 규칙                               SB = 전투 중 10/s · 평시 80/s
```

**Shield 실측 + SB 비율 (2026-08-28)**
```
우리 현재                        SB 비율                 SB 정합 목표
플레이어  Sh  50 / HP  100 = 50%   플레이어 = HP의 25%      Shield 25
Parry_Bandit  30 /  200 = 15%   적 = HP의 2~4%          Shield 6
Axe_Elite     60 / 1000 =  6%   (중앙 3%)                Shield 30
Sword_Bandit   0 /  200         SB 는 Normal1 94% ·      Shield 6
Arrow_Bandit   0 /   60          Normal3 96% · Elite      Shield 2
Dummy 2종      0                 92% · Boss 100% 보유     0 유지(불멸/경직 더미)
```
✅ **승환 결정 = SB 와 같은 비율로 간다.** 값 변경은 **4단계와 함께** — 지금 값만 바꾸면
Shield 가 아직 상시 완충재라 그냥 얇아지기만 한다.

⚠️ **SB 실드 계산식은 못 찾았다.** `CharacterTable` 에 상수는 있으나(`ShieldBlock 2.0` ·
`DamageReductionPerShieldBock 0.175` · `BaseDamageReductionByShield 0.0`) **공식이 JSON 에 없다**
(엔진 네이티브 추정). 그래서 아래는 **SB 실측이 아니라 우리 안**이다.
```
막기 성공     데미지 100% 를 Shield 에서 뺀다. HP 로는 0
Shield 부족   남은 몫만 HP 로 (Shield 20 인데 30 맞으면 -> Shield 0 · HP 10)
Shield 0      가드 붕괴. 일정 시간 막기 불가
```
플레이어 Shield 25 · 적 `AttackPower` 기준 **막기 2~3대면 붕괴** 예상. PIE 로 조인다.
📌 ⚠️ **SB 적의 Shield 3% 가 플레이어의 25% 와 같은 역할인지는 확인 못 했다.** 적은 너무 얇아
(한 대면 깨짐) 막기 자원이라기보다 얇은 완충재로 보이나 **추측이다.**

⚠️ **2-3(Poise 저스트 전용화)은 4단계와 짝이다.** 막기가 공짜인 채로 Poise 만 못 깎게 하면
막기가 더 좋아진다. 4단계와 같이 넣는다.

**5단계 — 적 전조(읽기).** 가장 비싸다(적 애니·큐·AI). **1단계에서 "못 읽겠다" 가 나올 때만.**

### 🔴 실행 순서 — 4를 3 앞으로 (2026-08-28 변경)

```
0  Timeline 길이 확인    승환 · 5분
1  값 3개 + PIE 진단     에셋만 · 반나절      <- 여기서 방향이 갈린다
2  반격 사슬            코드+에셋 · 가장 큼
4  막기의 대가          코드+에셋 · 하루     <- 3보다 앞으로 당김
3  연출 이식            에셋 위주 · 가드 붕괴 연출도 여기서 같이
5  적 전조              조건부
```
**번호는 그대로 두고 실행 순서만 4를 3 앞으로 옮겼다.** 이유는 비용이 아니라 **게임이 계속
재밌어야 해서**다.
```
1 -> 4 -> 2   창은 좁아지고 막기는 닳는데 저스트로 얻는 게 아직 없다 = 어렵기만 하다
1 -> 2 -> 4   저스트가 이득이 된 뒤에 대가가 붙는다 = 대가가 "저스트를 노리게 하는 장치" 가 된다
```
그리고 4를 먼저 넣으면 **가드 붕괴 연출을 3단계에서 한 번에** 처리할 수 있다.

**1과 2가 80% 다.** 3은 그 위의 화장이고, 5는 1단계 결과를 보고 정한다.

### 곁가지 — 스킬 재고 (미사용 17개, 전부 현행 스켈레톤)

```
Skill_01  Start40 / Loop40 / Shoot90     차지 후 발사
Skill_02 105 · Skill_03 215 · Skill_04 155
Skill_05  Start/Loop/Stop 28x3           짧은 반복
Speed_Attack      Start10 / Loop50 / End75    홀드 연타   (공중판 세트 별도 존재)
Aim_the_Target    Run_Shoot40 · Walk_Shoot62  이동 사격
```
승환 = **"재료 보고 정하자"**. 구조가 뚜렷한 셋 = `Skill_01`(차지) · `Skill_05`(반복) · `Speed_Attack`(홀드 연타).

---

## ✅ 2026-08-27 (2) — DA 5개 로드 불능 복구 + 상태 바 전제 정정

dev-log = `2026-08-27-dataasset-load-failure.md`

```
발견      상태 바 폭을 보려다 걸렸다. DA 6개 중 3개가 값을 못 읽었다
원인      개명 전수 재저장(f408086)에서 5개 누락 -> 리다이렉트 삭제(19a931a)로 다리 끊김
영향      엘리트·패링밴딧·경직더미가 스탯 DA 를 못 읽고 코드 기본값으로 돌고 있었다
복구      리다이렉트 임시 복원 -> 재저장 -> 삭제 -> 재시작 재검사 통과 (값 손실 0)
커밋      Content 79a4193 · 6c3ba1e   코드 repo 는 ini 원상복구라 변경 0
```

★ **08-26 검증이 놓친 이유** = "옛 참조 0건" 검사는 **에셋을 열어야** 참조를 센다.
열리지 않는 에셋은 검사에서 통째로 빠져 0건으로 나온다. 개명·리다이렉트 삭제의 관문은
참조 검사가 아니라 **전수 로드 검사**다. 검사 스크립트는 dev-log 에 박아뒀다(BP 는 별도 루프).

### 🔴 적 상태 바 칸 폭 — 종전 기록이 거짓이었다 (실물 실측)

```
실물   Overlay_Root > VerticalBox_Main > SizeBox_HP(200x20) · _Poise(30x10) · _Shield(200x10)
배선   SizeBox_Poise.SetWidthOverride( Min(MaxPoiseCache, 10) * 10 )
```

- **"엘리트 250px > 바 폭 198" 은 거짓.** `Min` 클램프가 이미 있어 최대 100px, 안 넘친다
- `198` 이라는 숫자는 위젯 어디에도 없다 (HP 바가 200)
- **칸(세그먼트) 구조가 없다** — `ProgressBar` 하나뿐. dev-log 의 A안은 없는 구조를 전제한 것
- 진짜 문제는 반대 = `MaxPoise` 10 이상이 **전부 같은 100px** (10/15/25/99999 전부). 체급이 안 보인다

```
1안(권함)  EventGraph 의 Min (Float) 노드 B 핀 10 -> 20    핀 하나. 100/150/200/200
2안        곱셈 체인 삭제 + 폭 200 고정                     HP·Shield 와 통일, 체급 표현 포기
3안        현행 유지                                        전부 100
```
노드 위치 = `EventGraph` `x1538 y1204` `Min (Float)` -> 오른쪽 `float * float` -> `Set Width Override`

### ✅ 실행 완료 (2026-08-27) — Content `2982fea` · PIE 통과

```
Min (Float)     A <- Get MaxPoiseCache   B  10 -> 20     칸 개수 상한
float * float   A <- Min 출력             B  10 유지       칸당 픽셀
=> 폭 = Min(MaxPoise x 10, 200)
   궁수 100 / 밴딧·패리밴딧·경직더미 150 / 엘리트 200 / 불멸더미 200(상한)
```
⚠️ **핀을 헷갈리기 쉽다** — 두 노드 다 B 핀에 10 이 들어 있었다. 구분 = `A` 핀에 무엇이 꽂혔나.
`Min` 은 `Get MaxPoiseCache` 가 직접 / 곱셈은 `Min` 의 출력이 꽂힌다. 처음에 곱셈 쪽을 고쳐
전부 200 으로 붙었다(고치기 전보다 나빴다). **상한을 안 올리고 칸당 픽셀만 2배 하면 전부 상한에 걸린다.**

### ✅ 결정 (2026-08-27 승환) — **C안: 지금은 폭만 막고, 점(Dot) 방식은 적 밸런싱 때**

SB 덤프 실측 결과 **SB 는 격파 게이지를 바가 아니라 점 나열로 그린다.** 그리고 그 UI 는
**값이 작아야 성립한다**(SB 잡몹 2~5 · 전체 최대 28 / 우리 10·15·25). 즉 점 방식으로 가려면
`MaxPoise` 재설계가 선행이고, 그건 UI 작업이 아니라 밸런싱 작업이다.

```
지금    1안(Min 핀 10 -> 20)으로 넘침만 막는다. 위젯에 더 시간 쓰지 않는다
나중    적 밸런싱 때 MaxPoise 를 SB 급으로 내리고(비율 1/4 유지) 위젯을 점 나열로 교체
        -> 그때는 폭 계산이 필요 없어진다. 점 개수가 곧 값이다
```

SB 실측 = 메모리 `reference_sb_combat_numbers` §적 격파 게이지 · `reference_sb_ui_hud_structure` §적 상태 바 위젯
```
SB 잡몹 MaxStamina 2~5 (Normal2 는 42%만 게이지 보유 · Animal 은 0%)
SB 엘리트 6~28(중앙 12) · 보스 10~24(최빈 20)
우리 밴딧 15 ÷ 패링 4.0 = 약 4회  ==  SB 잡몹 4~5 ÷ 패링 1 = 4~5회   <- 체감은 이미 같다
차이는 눈금 크기뿐 (우리가 평타로도 깎으려고 4배 잘게 씀)
```

---

## ✅ 2026-08-27 (1) — 대상 탐색 필터 구조 + 자동 조준 결함 6건 + 적 상태 바

dev-log = `2026-08-27-target-filter-autoaim.md` · `2026-08-27-enemy-statebar-visibility.md`

```
신규        Combat/Data/KDTargetFilter.h — Arc|Cylinder · 반각 · 높이 · 기준벡터 · 정렬 · bDrawDebug
분리        FindBestTarget(락온) / FindTargetByFilter(공용) / GatherCandidates / GetFilterBasis
해결 6건     등뒤 스냅 · 콘360 무효화 · 멀어도 중앙이면 뽑힘 · 카메라 기준 · 위아래 안 잘림 · 함수 공용
검증        빌드 Succeeded(에러 0) · PIE 통과(승환)
```

### ✅ 종전 "새 세션 첫 할 일" 2건 — 둘 다 닫힘 (08-27 저녁)

```
0  빌드 확인     ✅ 완료. DLL 19:55 > 소스 19:25
1  MCP 재연결    ✅ 붙어 있다
```

### 다음 (순서)
```
1  적 상태 바 칸 폭  ⚠️ 전제가 거짓이었다 — 위 08-27(2) 절 참조. 안 3개 갱신됨
1b 적 상태 바 표시  ✅ 완료 — 락온 OR 최근피격 타이머 (빌드·PIE 통과)
2  워프 ⛔보류        MaxApproachRange 700 은 총 넉백 기준으로 정한 값이다(승환).
                  따로 손대지 말고 **총 넉백 + 검 넉백을 합산해 계산한 뒤** 정한다
3  ShotBlast 이관    지금은 임시 필터를 매번 생성 — 멤버로 올리면 에디터 조절 가능
4  캔슬 윈도우 02_02  ✅**닫힘 — 62f 유지 (2026-08-27 승환)**
```

### ✅ 02_02 캔슬 윈도우 = 62f 유지 (2026-08-27, 항목 종결)

승환 관측 = "02_02 의 캔슬 지점 포즈가 02_03 과 같아서 거기 찍은 것 같다."
**관측은 정확한데 그건 실수가 아니라 설계다.** 20타 전부 그렇게 찍혀 있다.

```
개별 클립 = [Idle] -> 공격 -> [공격 끝 포즈] -> Idle 복귀 -> [Idle]
다음 타는 클립 끝이 아니라 "공격 끝 포즈" 에서 시작
-> 캔슬 지점 포즈 == 다음 타 시작 포즈 가 정상. 20개 중 19개가 포즈 차이 0.0
62f = 2026-07-27 에 통짜 _ALL 클립과 포즈 매칭으로 역추출한 확정값 (임의 배치 X)
```

**"73% 이상치"는 비율이 만든 착시였다.** 2타끼리 보면 이렇다.
```
2타 캔슬 프레임   C05 35 · C01 44 · C04 45 · C03 57 · C02 62
```
늦은 편이지만 가족 범위 안이다. 클립이 84f 로 짧아 비율만 튀었다.
남는 사실 = 판정 종료 19f 후 **공백 43f(0.72초)**. 안무 자체가 회수가 긴 클립이다.
⛔ 당기려면 다음 몽타주 BlendIn 을 0.1~0.15s 로 올려야 한다(포즈가 어긋나므로). **지금은 안 한다.**

### ✅ 결정 (2026-08-27 승환) — 죽은 노브 `PerfectParryWindowSec` 는 **삭제**

난이도 조정 같은 기능이 생기면 그때 배선한다. 지금 안 쓰는 값을 노브로 남겨두지 않는다.

### 🟠 `GE_PerfectParry` Duration = **0.5 유지 (임시)** — 되돌릴 것

```
설계값 0.2 · SB 실측 0.15 · 현재 0.5
```
촬영용으로 올린 값이 안 돌아온 것이었으나, **승환이 계속 테스트할 거라 당분간 0.5 로 둔다**(2026-08-27).

🔴 **이 값은 반드시 되돌려야 한다.** 08-24 에 "원복했다"고 기록됐지만 실제로 원복된 건
죽은 노브 쪽이었고 진짜 값은 3일간 0.5 로 남아 있었다. **같은 사고를 두 번 내지 말 것.**
```
확인 경로   /Game/SB_Style_GameProject/GAS/GE/BlockAndParry/GE_PerfectParry
            Duration Magnitude -> Scalable Float Magnitude -> Value
이제 노브는 하나뿐이다   GA 쪽 PerfectParryWindowSec 를 지웠으므로 여기가 유일한 진실
```

🟡 **미해결로 남긴 것** — `UKDLockOnComponent` 가 400줄대로 늘어 §1 Component 300줄 선 초과. 분리 검토 대상.

---

## ✅ 2026-08-26 — 클래스 개명 68개 (종결)

dev-log = `2026-08-26-class-naming-convention.md` · `2026-08-26-poise-context-channel.md`

### 지금 상태

```
빌드            통과 (E·D·C 한 번에)
정적 검사 3종    전부 0건 — 깨진 include / generated.h 불일치 / 구 이름 잔존
커밋            456fc3d(E) · a510235(D) · 56ff9a4(C) + 개명 dev-log
에디터          켜져 있음. ⚠️ MCP 연결이 끊긴 상태라 조회를 못 했다
```

### ✅ 1~7 전부 완료 (2026-08-26 오후) — 개명 에디터 검증 종결

> dev-log = `2026-08-26-rename-editor-verification.md`. 요약:
> 검증 1~4 통과 → PIE 에서 **처형 GC 만 사망** → 원인 = 큐 매니저 스캔에 ClassRedirects
> 안 먹음(개명 부모를 둔 GCN BP 3개가 맵 누락, 전조 링·히트 임팩트도 같이 죽어 있었다)
> → BP 3개 재저장 + 에디터 재시작으로 복구(맵 재덤프 + PIE 확인) → **전수 재저장 312개**
> (Content `372dcd6`·`f408086`) → 검증 3중(풀패스 옛 참조 0건) → 코드 푸시 완료.
>
> ✅ **ClassRedirects 80줄 삭제까지 완료 (`19a931a`).** 맵 검사에서 걸린 `LV0_Test` 잔존
> 임포트 13개도 재저장(Content `7303945`) 후 삭제. 재시작 검증(큐 맵·PIE) 통과.
> ⚠️ 두 폴더 밖 killdong 잔재 몽타주의 노티는 이제 죽어 있다(폐기 라인, 감수 방침).
> **개명 작업 완전 종결 — 다음 = C1 콤보 노드 값 채우기.**

<details><summary>종전 할 일 목록 원문 (완료됨, 접힘)</summary>

```
1  MCP 연결 확인          에디터 우하단 ● MCP :3000 / 안 되면 /mcp 재연결
                         그래도 안 되면 아래 "MCP 없이" 절 참조

2  ★ BP 19개 부모·태그     GA 22개가 한꺼번에 바뀐 자리. 개명의 진짜 관문
                         부모가 Project_KD.KDGameplayAbility_* 로 나오나
                         ★ AbilityTags 가 빈 BP 가 있나
                         (2026-08-26 아침 GA_SprintAttack 이 정확히 여기서 죽어 있었다)

3  ★ 몽타주 노티            Notifies 배열이 비지 않았나 = 리다이렉트가 먹는가
                         ANS_MeleeTrace 의 per-window 오버라이드 값도

4  DA 값                   적 DA 6개 MaxPoise 15/10/25 · KnockbackDistance 200
                         · PoiseDamageByAttack 3키 (Parry 4.0 / Light 0.5 / Heavy 0.9)
                         DA_ComboTree 2개 노드 26개

5  PIE 한 바퀴             콤보 · 패링 3종 · 회피 · 처형 · 총격 · 넉백 · 공중 콤보

6  몽타주 전수 재저장        C그룹 마무리. 리다이렉트 의존을 끊는다 -> Content 커밋
                         ⚠️ 대량 쓰기라 승환 승인 후

7  푸시                    현재 로컬에만 4커밋 쌓여 있다
```

> 🔴 **2~5 통과 전엔 아무 에셋도 저장하지 말 것.** 리다이렉트가 실패한 상태로 저장하면
> 깨진 상태가 에셋에 구워진다. 되돌릴 지점 = Content `f5da3fe`.

> 🔴 **6 전에 `DefaultEngine.ini` 의 ClassRedirects 68줄을 지우면 안 된다.**
> 지금 에셋들을 붙잡고 있는 유일한 다리다.

</details>

### MCP 없이 검증하는 법

에디터 `Output Log` 하단 Cmd 드롭다운 -> `Python` 에서 조회 스크립트를 직접 돌린다.
읽기 전용이라 안전하다. 스크립트는 아래 항목을 뽑으면 된다.

```
BP        get_tag_value('ParentClass') + CDO 의 AbilityTags
적 DA     MaxPoise · KnockbackDistance · PoiseDamageByAttack
콤보 트리  Nodes 길이
몽타주     notifies 배열 길이
```

### 문서 파급 — 아직 안 함

```
docs/PROJECT_OVERVIEW.md   §2 시스템 맵 · 상속 트리에 옛 클래스명이 전부 남아 있다
볼트 notes/코드구조/ 10문서  브릿지로 갱신 요청 등록함 (2026-08-26)
docs/reference/네이밍 규약   Op 예시만 KD 로 (🔒 승환)
```

---

## ✅ 2026-08-25 (2) — B2 완료 + 상태 바 버그 해결

dev-log = `2026-08-25-refactor-b2-onhitreceived.md` · `2026-08-25-enemy-statebar-resubscribe.md`

### 끝난 것

```
커밋·푸시 6개      리팩토링 A/E/B1 + 처형 큐 태그 주석 + CLAUDE.md 이사분 + docs
B2                OnHitReceived 135줄 -> 4함수. 빌드·PIE 통과
                  (경직 진입하는 타에 안 밀림 = ApplyPoiseDamage 반환값 검증)
상태 바 버그       재락온 후 갱신 정지. Construct 에 재구독 배선. 08-18 부터 있던 버그
보류 13번          닫힘 — PoiseDamageByAttack 키는 전 DA 가 Ability.Player.Parry 하나뿐
```

### C2 완료 (2026-08-25)

dev-log = `2026-08-25-knockback-component.md`

```
UKnockbackComponent 신설     LaunchCharacter(속도) -> RootMotionSource(거리)
DA 필드                      KnockbackStrength(cm/s) -> KnockbackDistance(cm)
                             ⚠️ 뜻이 바뀌어 리다이렉트 일부러 안 걸었다
brain 정지                   Pawn 유지 (StaggerComponent 선례 - brain 소유자 = Pawn)
디버그                       목표 cm 대비 실제 cm. 재현성 확인됨
```

**남은 값 작업** — 더미만 `KnockbackDistance` 200. 적 5종은 체급 기준으로 나중에.

### C3·C4·C5 완료 (2026-08-26)

dev-log = `2026-08-26-refactor-c3-c4-c5.md`

```
C4  락온 LoS 통일        HasLineOfSightTo 헬퍼 — ObjectType 으로 통일
                        (근거 = AT_MeleeTrace.cpp:89, ECC_Visibility 는 Pawn 이 무시)
C5  PostGEExec 4분해     본문 56줄 + 헬퍼 4개. Health 차감은 마지막 유지
C3  빈 상속 층 제거       UGA_PlayerOneShotAttack 삭제 (멤버 0)
                        자식이 셋이었다 — SprintAttack / CounterThrust / AirAttackBase
                        BP 리페어런팅 불필요 (직접 상속 BP 0개)
```

★ **부수 수확 — 달리기 공격이 죽어 있었다.** `GA_SprintAttack` BP 가 `UGA_SprintAttack`
을 건너뛰어 `AbilityTags` 가 비어 있었고, 발동은 태그로만 한다. 리페어런팅으로 해소,
PIE 확인 완료. **언제부터였는지는 `.uasset` 바이너리라 미상.**

GA 태그 전수 감사 19개 — 위 1건 빼고 정상. `BP_DaggerShot` 만 미사용 습작(참조 0건).

### Poise 2단계 완료 (2026-08-26)

dev-log = `2026-08-26-poise-context-channel.md`

```
FKDGameplayEffectContext   커스텀 EffectContext — PoiseMultiplier 운반
UKDAbilitySystemGlobals    ASC 가 우리 Context 를 만들게 함
DefaultGame.ini:12         AbilitySystemGlobalsClassName 교체 (되돌리려면 이 줄만)
배선 10군데                기존 DamageMultiplier · KnockbackMultiplier 와 같은 모양
```

**노드 값은 안 채웠다 — 통로만 뚫었다.** 26노드 전부 0 이라 동작은 종전과 동일.

✅ **통로 관통 확인 완료** — 노드 하나에 `PoiseMultiplier = 3.0` 을 넣으니 Poise 칸이
3배로 깎였다. 확인 후 원복. C1 로 넘어가도 된다.

### 다음 (순서)

```
1  클래스 개명            E 완료 -> D -> C 그룹 순. 아래 절
2  C1                    콤보 노드 4칸 값 채우기 (PoiseMultiplier 포함)
3  볼트 브릿지 회수       FGameplayEffectContext 학습 노트 요청 (08-26 등록)
```

**이월 리팩토링은 C1 만 남았다.** A·B1·B2·C2·C3·C4·C5 완료.

### 🔤 클래스 개명 (착수 예정)

문서(`docs/reference/UE5-GAS-Naming-Convention.md`)가 표준이고 **코드가 비표준**이다.
엔진 실측 — `UAbilitySystemComponent` · `UGameplayAbility` 처럼 타입 접두를 안 쓴다.
`GA_` `GE_` `AS_` 는 **에셋 이름 규칙**이지 C++ 클래스 규칙이 아니다.

```
A  이미 준수        17개   AKDPlayerCharacter 등            변경 X
B  엔진 관례        9개    UBTTask_ · UBTService_ · UEnvQueryContext_   변경 X (엔진과 동일)
E  접두 없음        21개   UWeaponComponent -> UKDWeaponComponent      🟢 1단계
D  GAS 에셋 접두    36개   UAS_ · UGA_ · UGE_ · UGCN_                  🟠 2단계 (BP 19개가 부모)
C  엔진 관례 축약    11개   UANS_ · UAN_ · UAT_ -> 풀네임               🔴 3단계 (몽타주에 인스턴스)
```

**C 그룹은 개명 후 몽타주 전수 재저장이 필수.** 노티는 몽타주 안에 인스턴스로 박혀 있어
`+ClassRedirects` 는 임시 다리일 뿐이다. 재저장 안 하면 나중에 그 줄을 지우는 순간 조용히 죽는다.
✅ 노티 11개가 전부 `GetNotifyName_Implementation` 을 오버라이드하므로 **타임라인 표시명은 안 바뀐다.**

⚠️ `DefaultGame.ini:12` 의 `KDAbilitySystemGlobals` 는 문자열 경로라 리다이렉트가 안 먹는다.

### 값 작업 완료 (2026-08-25, Content `65fb89d`)

```
KnockbackDistance   6종 전부 200 (C2 로 cm 단위가 된 뒤 첫 실값)
MaxPoise            체급비 5배 유지 — 밴딧·패리밴딧·더미 15 / 궁수 10 / 엘리트 25
PoiseDamageByAttack Parry 4.0 / Light 0.5 / Heavy 0.9  <- 평타로도 깎이게 확장
                    DA_Dummy_Immortal 은 99999 + 빈 맵 유지 (불멸 더미 의도)
```

새 템포 — 밴딧 평타 30대 / 강공 17대 / 패링 4회. 길면 Light 를 0.8~1.0 으로.

### 🟡 미해결

```
적 상태 바 표시 조건  ✅**결정(08-26 승환)** = `락온됨 OR 최근피격(N초 타이머)`. 전투 진입 트리거는 **안 넣는다**
                    현재는 락온이 유일한 스위치(`OnTargeted_Implementation` -> `SetVisibility`, BP 개입 0)
                    ⚠️ 두 이유가 각자 SetVisibility 하면 락온 해제가 피격 표시까지 꺼버린다 -> OR 로 합칠 것
                    피격 훅은 이미 있다 = `KDEnemyBaseCharacter::OnHitReceived`(.cpp:425)
적 상태 바 칸 폭     엘리트 MaxPoise 25 = 250px 인데 바 폭이 198. 넘친다.
                    위젯이 SizeBox_Poise 폭을 MaxPoise x 10 으로 잡는 구조.
                    -> 위젯 조정 보류 (승환 판단)
공중 넉백           AccumulateMode::Override 가 중력 누르는지 미검증
Combat.Execution 큐  받는 GCN 이 프로젝트 전체에 0개 (08-26 레지스트리 실측).
                    KDExecutionComponent.cpp:92 가 허공에 쏜다. 피니셔 연출은 몽타주 쪽.
                    GCN 을 배선하든 호출을 지우든 — 처형 폴리싱 때 판단
오사 본 셰이크       적이 화살 막을 때 뼈 흔들림 뜨는지 미구별.
                    PlayHitFeedback 이 C++ 유일 호출처 — 로그 한 줄로 C++/BP 갈림
Content 잔여        Robot3/ 만 미추적 유지. 나머지는 전부 커밋됨
ShieldAttackDamageRate  ✅해결(08-26) — 별도 Shield 축 확정. CharacterTable 에 MaxShield/MaxStamina
                    독립 공존 + 계산 타입 enum 3축(Physic/Shield/StaminaDamage). 상세 = 세션브릿지 KD 판정
```

### 🧹 정리 잔가지

```
KDEnemyBaseCharacter.cpp:30   #include "HAL/IConsoleManager.h" 고아
                              (CVarShowKnock 이 컴포넌트로 갔는데 include 만 남음)
KnockbackBrainPause 0.15 < Duration 0.2   측정엔 영향 없음 확인. 연출 판단만
```

---

## ✅ 2026-08-25 (1) — 리팩토링 A·E·B1 완료

dev-log = `docs/dev-logs/2026-08-25-refactor-dedup-deadcode.md` (진단 전문·SB 구조 비교 포함)

### ✅ 1순위 — 해결됨 (2026-08-25 빌드·재시작 완료)

**에디터 종료 → 빌드 → 재시작으로 태그 복구 반영. 피니셔 정상 출력 확인.**
아래는 원인 기록 — 같은 실수 재발 방지용.

#### 원인 — 태그 2개 복구분이 미반영이었다

**A5에서 죽은 코드로 지운 태그 2개가 실은 에셋이 이름으로 쓰던 것이었다.**

```
GameplayCue.Combat.Execution   ← DA_Execution_Axe · DA_Execution_Bandit  (피니셔 VFX)
GameplayCue.Camera.Execution   ← DA_PlayerExecution · GCN_ExcutionCamera  (처형 시네 카메라)
```

증상 = **처형 시 설정한 에셋 시점이 안 나온다** (승환 PIE 실측). C++ 참조는 0이었지만 **에셋이 문자열로 들고 있으면 네이티브 등록을 지우는 순간 태그 해석이 조용히 실패**한다.

→ `KDGameplayTags.h/.cpp` 에 **복구 코드는 이미 넣어뒀다.** 빌드만 남았다.

⚠️ **라이브코딩(Ctrl+Alt+F11)으로는 안 살아날 가능성이 높다.** `UE_DEFINE_GAMEPLAY_TAG` 는 모듈 로드 시 정적 초기화로 등록되는데 라이브코딩 패치는 그걸 다시 돌리지 않는다.

```
에디터 종료 → 빌드 → 에디터 재시작 → 처형 카메라 확인 + 피니셔 VFX(적 2종) 확인
                                     ^^^^ 2026-08-25 실행 완료. 피니셔 정상
```

📌 **교훈 = 태그는 "C++ 참조 0" 만으로 죽은 코드 판정하면 안 된다.** 에셋 문자열 참조를 grep 해야 한다. 복구한 두 줄에 주석으로 박아뒀다.

### 🟡 2순위 — 미조사 1건 (리팩토링 무관으로 보임)

**적끼리 오사 — 데미지는 정상적으로 안 들어가는데, 적이 화살을 막을 때 본 셰이크 같은 게 발동하는 것 같다** (승환 관측).

`KDProjectile::OnSphereOverlap` 은 friendly fire 면 `SendHitEvent` 전에 return 하므로 **히트 이벤트 경로는 아니다.** 후보 = `BP_Arrow` 자체 오버랩 연출(Niagara/사운드) 또는 Destroy 시 이펙트. 미확인.

### PIE 검증 결과 (리팩토링 회귀)

**위 처형 카메라 1건 빼고 전부 이상 없음.** 지상·공중 콤보 / 회피 합류 / 이동 캔슬 / 총격·화살 / 패링 3종 / 카운터 FOV 펀치 통과.

### ✅ 커밋 완료 (2026-08-25, `058349c` ~ `dd4a4c7` 푸시됨)

```
리팩토링   A(소품5) · E(공통화2) · B1(콤보통합)   34파일
태그 복구   KDGameplayTags.h/.cpp                  ← 빌드·확인 후 함께
docs       dev-logs/2026-08-25-*.md · INDEX.md
```

커밋 제안 4분할
```
[refactor] 캔슬 판단 InputComponent 이사 + 회피 진입 DA화 + 죽은 코드 정리
[refactor] 데미지 파이프라인·Self GE 공통화 - KDAbilityStatics / GA_ActionBase
[refactor] 콤보 노드 소비를 GA_PlayerMeleeAttackBase 로 통합
[fix] 처형 큐 태그 2개 복구 - 에셋이 이름으로 참조 중이었다
[doc] 08-25 dev-log
```

### 이월 — 리팩토링 잔여 (목록·판정 근거 = dev-log)

```
B2  OnHitReceived 4분해          독립. SB ResultTable 축(인지/수치/이동/연출)
C1  DA_ComboTree InputWindow 값   B1 끝나 선행조건 해소. SB 실측치 준비됨
C2  넉백 RootMotionSource + KnockbackComponent 추출   B2 선행이면 수월
C3  GA_PlayerOneShotAttack 층 제거   공중 콤보 재설계 때 (BP 리페어런팅 동반)
C4  락온 LoS 방식 통일               락온 재작업 때
C5  PostGEExec 헬퍼 분리            다음 데미지 로직 붙일 때
```

### 📌 룰 파일 재편 (2026-08-25)

전역 `~/.claude/CLAUDE.md` 175행 → 122행 다이어트. 잘라낸 상세는 **`~/.claude/reference/CLAUDE-md-이관-2026-08-25.md`**(자동 로드 X)에 보존.
본 프로젝트 `CLAUDE.md` 에 이사분 3건 반영 완료 — dev-log 자동 생성(§0) · 새 클래스 파일 목록 승인(§0) · MCP 대량 조회 규칙 + **Fable 세션 하청 룰**(§7).

🟡 **승환 할 일** — 취업 `CLAUDE.md` **110~122행 삭제**. 그 폴더엔 §스킬 파이프라인(63~78행)이 이미 있는데 이관 블록이 덧붙어 **중복 2벌**이 됐다(그 파일 37행이 경고하는 바로 그 상황). 원본이 더 최신이라 붙인 쪽을 지우면 된다.

---

## ✅ 2026-08-24 — 문서 대정리 완료

dev-log = `docs/dev-logs/2026-08-22-debug-console-player-init.md` (직전 = `2026-08-21-aim-knockback-camera.md`)

### ✅ `Project_KD` 는 깨끗하다 — 변경 0 / 미푸시 0

오늘 커밋 11개 전부 푸시됨.

```
fc4b1d3  [docs] INDEX dev-log 표를 최신순 + 최근 15개만 펼침
ac03f99  [docs] 핸드오프를 CURRENT + archive 로 분리 + 버터맵 통합본 삭제
5000aef  [docs] design/ 참고용 복구 - 삭제 대신 거짓 목록을 붙여둔다
f31bf02  [docs] design/ 폴더 삭제
c74ccf6  [doc]  KDPlayerState 주석의 Dosul 을 Ammo 로
05b3abe  [doc]  핸드오프를 실제 상태로 + 작동 안 하는 복구 명령 표시
9da756c  [doc]  두 세션 역할 구분 + 세션 브릿지 등록
09c40f7  [docs] 길동 시대 문서 정리                      <- 볼트 세션
e71cd87  [docs] 08-21 / 08-22 dev-log + 현재 상태        <- 볼트 세션
5c32389  [Input] 입력 버퍼 보관 기한 상한 0.8
a975cb2  [refactor] 개발용 온스크린 표시를 콘솔 변수로

Content (원격 없음, 로컬 세이브포인트)
716f0c0  [BP] 촬영용 임시값 원복
070f3b2  [BP] 촬영용 디버그 표시 원복 - 누락 4개
```

### 오늘 한 것 요약

**코드** — 온스크린 디버그 4종을 콘솔 변수로(`KD.ShowDamage`/`ShowDodge`/`ShowApproach`/`ShowKnock`, `ECVF_Cheat`) / 입력 버퍼 `ClampMax` 0.5 → 0.8(기본값 0.2 유지) / `KDPlayerState.h:33` 주석 `Dosul` → `Ammo`

**Content** — 촬영용 임시값 원복(밴딧 Poise 1→3, 퍼펙트 패링 창 0.5→0.2) + 디버그 표시 원복 누락 4건(`GA_AirLightAttack`·`CounterThrust`·`ShotBlast`·`SprintAttack`). **에디터 안 켜고 git 으로만** 했다

**문서** — `design/` 삭제 후 참고용 복구(거짓 4건 표 부착) / 핸드오프 1,052행 → `CURRENT.md` 233 + `archive/` 834 / 버터맵 통합본 2,381행 삭제 / `INDEX` dev-log 표 최신순 + 접기. **저장소 MD 13,648 → 11,267행**

**운영** — 세션 브릿지 신설(볼트 세션과 역할·파일별 담당 확정) / Fable 5 원인 규명

### 🟡 남은 것 = `Content` 8개 (7 M + `Robot3/`)

촬영 관련은 전부 커밋됐다. 아래는 **촬영 커밋 6개가 건드리지도 않은** 잔여분이다.

```
🟡 확인 필요   BP_Dummy         -688   마지막 커밋 180ab96 (08-19)
🟡 확인 필요   BP_Bandit_Parry  -450   마지막 커밋 662390c (MCP 도입 전)
⬜ 노이즈      BP_Bandit -7 / BP_Axe_Elite +8 / Arrow ±0 / Arrow2 ±0
⬜ 맵          LV0_Test.umap    -4699  촬영 조명·노출 원복으로 추정
⬜ 미추적      Robot3/          유지
```

⚠️ **앞의 둘은 크기가 줄었다 = 내용이 실제로 빠졌는데 무엇인지 모른다.** 에디터 켤 일 있을 때 같이 볼 것. 급하지 않음.

### 🔒 승환만 가능 (잠금 영역)

```
docs/reference/README.md:27   "시스템 설계 -> docs/design/" 이 부정확 (design 은 참고용)
GA_MeleeTraceBase.h:60        TipLine 인데 .cpp:21 생성자가 Sweep 로 덮는다. 3개월째
                              헤더만 읽으면 오해한다. 한 줄 수정
```

### 🔀 세션 브릿지 — 볼트 세션과의 유일한 통신로

```
C:/Users/asdasd/Desktop/Obsidian_organize/ProjectKD/notes/_세션브릿지.md
```

**세션 시작 시 이 파일부터 읽는다.** 역할 구분·파일별 편집 담당·대기 항목이 거기 있다.
판정 권한 원칙 = **자기 눈으로 원본을 볼 수 있는 쪽만 단정한다.** 코드·에셋·빌드 = KD / 문서·노트 = 볼트.

### 📌 오늘 세운 방법 (재사용)

- **`.uasset` 원복 판정** — 값이 같아도 재저장하면 바이트가 바뀌어 해시 비교가 무의미. **크기 이력**(`git cat-file -s`)을 본다. 같은 bool 토글은 항상 같은 바이트(-44)라 `변경전 → 변경 → 현재` 세 값이면 판정된다
- **에디터 없이 에셋 되돌리기** — ①에디터가 꺼져 있고 ②되돌릴 커밋 이후 그 파일을 건드린 커밋이 0개면 `git checkout <커밋>^ -- <파일>`. **에디터가 켜져 있으면 금지**(메모리의 옛 버전으로 덮어씀)
- **문서는 낡는 게 아니라 거짓이 된다** — `design/` 을 코드와 대조하니 없는 클래스를 인용하고, 쓰이는 채널을 "미사용"이라 하고, 끝난 구현을 "대기"라 하고 있었다. stale 경고만으로는 어느 줄이 거짓인지 모른다

---

## 3. 보류 목록 (우선순위 순)

> 🔴 **읽기 전에** — 아래 항목 다수가 **2026-07-31 ~ 08-12 에 적힌 것**이라, 그 뒤 dev-log 20편이 쌓이는 동안 이미 해결된 게 섞여 있다.
> **착수 전에 코드로 확인할 것.** 실제 사례 = 1번의 "`DamageMultiplier` 칸 자체가 없다"는 2026-08-24 실측 결과 **거짓**이었다
> (`ComboTreeDataAsset.h:66` 에 있고, 곱하는 자리도 `GA_MeleeTraceBase.cpp:215` 에 배선돼 있다 — 08-19 에 들어옴).


| # | 항목 | 내용 |
|---|---|---|
1 | ~~**`DA_ComboTree` 값 채우기**~~ ✅**칸은 둘 다 생겼다** (2026-08-24 실측: `InputWindow` = `ComboTreeDataAsset.h:62`, `DamageMultiplier` = `:66`, 곱셈 = `GA_MeleeTraceBase.cpp:215`). **남은 건 값 채우기뿐** — 아래 SB 실측치 참조 | **두 값의 상태가 다르다 — 헷갈리지 말 것** (2026-07-31 A레인 지적으로 정정)<br>· **`InputWindow`** = **칸 있음 / 값 전부 0** → 아직 `ComboResetTime 1.5f` 공용값으로 돈다<br>· **`DamageMultiplier`** = **칸 자체가 없다.** `.h` 실측 확인 — DA를 열어도 그 칸은 안 보인다. `FComboNode`에 추가부터 해야 함(`InputWindow` 바로 아랫줄, 같은 형식)<br>SB 입력창 실측: 1~2타 0.7~0.8 / 3~4타 0.9~1.2 / 마무리 1.4~2.0 / 회피 0.8 / 저스트회피 1.5<br>⚠️ **DA는 2개다** — `DA_ComboTree` + `DA_AirComboTree`(같은 `FComboNode` 구조)<br>⚠️ `FComboNode`에 **`DamageEffectClass`(노드별 GE)가 이미 있다** — 계수를 float으로 넣을지 노드별 GE로 갈지 먼저 정할 것. 26노드 × 개별 GE = 에셋 26개라 **float 계수가 가볍다** |
2 | **캔슬 윈도우 — 전수 비교 후 대상이 1개로 줄었다** (2026-08-26 20타 실측) | 콤보 20타 창 열림 비율 = 대부분 **30~52%**. 마무리타 `_04` 4개는 창 없음(정상).<br>🔴 **`Combo_02_02` 만 진짜 이상** — 길이 1.42s 인데 창이 1.03s(**73%**). 구조가 혼자 다르다. → 0.75s 근처(평균)로 당기는 안<br>🟢 `05_03`(1.17s/52%) · `04_03`(1.20s/57%) · `02_03`(1.23s/45%) = **비율은 평범, 몽타주가 길 뿐**(2.1~2.7s). 당기면 안무가 잘린다 → 현행 유지 권고<br>⚠️ 옛 기록의 "늦은 3개"에 `04_03` 이 빠져 있었다. 절대 시각만 보면 넷이 같은 무리<br>버퍼 0.8s 기준 실제 증발 = `02_02` 는 타 시작 0.23s 이내, `02_03` 은 0.43s 이내 입력만. 판단=승환 |
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
11 | ~~일반 공격 자동 조준(미착수)~~ ✅**구현돼 있음** (2026-08-26 실측) | `UKDGameplayAbility::FindAutoAimTarget()`(락온 중=락온 타겟 / 아니면 `FindBestTarget` 재사용) + `KDGameplayAbility_PlayerMelee.cpp:127`에서 Range 500 · 콘 180° 로 사용 중. 🔴 **"공격이 엉뚱한 적 쪽으로 튼다" 의 원인 = 아래 두 개가 겹친 것 (2026-08-26 규명)**<br>① BP CDO 실측 `AutoAimConeAngle = 360` (헤더 기본 180 을 덮음) -> `cos(180°) = -1` 이라 `Dot < -1` 이 영원히 거짓 = **시야콘 필터가 통째로 무효.** 반경 800 안 **전방위** 적이 전부 후보<br>② 그 상태에서 아래 `DeltaYaw` 가드가 없다 -> **등 뒤 적에게도 180도 스냅**<br>즉 콘 360 은 의도(후보는 넓게 줍고)였고, **각도 제한은 가드가 맡기로 돼 있었는데 그 가드가 비어 있다.** 가드 한 줄이 이 증상의 해결책<br>⬜ 반증됨 — 모션워핑 회전은 무죄. 노티 `rotation_type = Facing` 이고 엔진 실측상 Facing 은 타깃 **위치**로만 방향을 만든다(적의 facing 미참조, `RootMotionModifier.cpp:393-406`)<br>기타 CDO 실값 = `AutoAimRange 800` · `MaxApproachRange 700` · `ApproachStopDistance 140` · 워프명 `ComboTarget`(노티와 일치)<br><br>🔴 **결함 1건 실측 (2026-08-26)** — `KDGameplayAbility_PlayerMelee.cpp:135` 의 `DeltaYaw` 가 **계산만 되고 미사용**(파일 전체 등장 1회). 주석은 "뒤쪽 135도 초과 제외"인데 가드가 없어 **등 뒤 적에게 180도 스냅**한다. 수정 = `SetActorRotation` 앞에 `if (FMath::Abs(DeltaYaw) > MaxAutoAimTurnAngle) return;` + 헤더에 `MaxAutoAimTurnAngle 135.f` 노출<br>✅ **결정 (2026-08-26 승환)** — **함수를 갈라낸다.** 근거 = "멀리 있어도 중앙이면 뽑히는 건 잘못"<br>　`하드락(락온)` = 화면 중앙 각도 **+ 거리** 가중 / `소프트락(자동조준)` = **캐릭터 정면 기준 최단거리**<br>　오버랩 수집·자격 필터(인터페이스·LoS·중복 제거)는 private 헬퍼로 공유. 회전 스냅은 유지(GoW 표준)<br>　근거 조사 = 메모리 `reference_melee_autoaim_research`<br>🟡 판단 2건 — ①타겟 선정이 `FindBestTarget` 재사용이라 **거리 아닌 "화면 중앙에 가까운 순"**(`KDLockOnComponent.cpp:193` Dot 최대). 근접에선 코앞 적을 두고 먼 적을 고를 수 있다 ②콘 기준이 **캐릭터가 아니라 카메라 forward**(`:150`) — ①번과 겹치면 큰 회전 |
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

### ① 스탠스 체계 — ✅ (a) 홀드 조준으로 구현·확정돼 있었다 (2026-08-26 실측. 이 항목은 종결)

```
홀드 방식      IA_Aim Started/Completed 바인딩 (KDPlayerController.cpp:84-85) — 누르는 동안만
조준 감속      Aiming 태그 시 Speed = min(Speed, AimMoveSpeed 167)  (KDPlayerCharacter.cpp:266)
              167 = SB 100 이 아니라 팩 걷기 클립 실측 속도에 맞춘 튜닝값
락온 속도      LockOnMoveSpeed 280 (KDPlayerCharacter.h:124) — SB 실측과 동일
조준 로코      MotionMatching/DataBase/Aim/ PSD 3개 -> CHT_LocomotionDatabase -> ABP_SB
              + AO_SB_Aim (9포즈 전부 사용) + Aim_the_Target Start/Loop/Shoot/End 몽타주
Run_Aim 미사용  구멍 아님 — 조준 중 속도가 167 클램프라 달리며 조준하는 상황이 없다
(b) 전환식     완전 미착수 — Gun 콤보트리 없음, Combo_Attack_Shoot 5클립 참조 0건. 안 간다
```

~~남은 잔가지 2개~~ → **둘 다 종결 (2026-08-26)** = ①이동 중 사격 PIE 정상(승환 실측) ②총구 이펙트는 GC 대신
**탄 궤적으로 해결** — `GA_Shoot -> BP_Bullet -> NS_Laser` 체인 실측. 콤보 속 히트스캔(`GA_ShotBlast`)엔 궤적 없음(현행 의도).

아래 SB 스탠스 속도표는 참고 자료로 유지.

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
- **총 스탠스 = 100. 걷기(150)보다 느리다.** "총은 딜링이 아니라 근접의 준비 도구"라는 §1-B 해석이 수치로 확증됐다. (우리 실적용은 167 — 클립 발맞춤)
- **락온 전용 속도가 따로 있다** (500 → 280, 44% 감속). ~~우리는 이 개념이 없다~~ → LockOnMoveSpeed 280 으로 적용돼 있다 (08-26 실측)
- **스프린트가 없다.** SB는 150/300/500 3단계. 우리는 250/500/700/800 4단계로 **전반적으로 빠르다** — 우리 Jog(500)가 SB 최고속과 같다

SB는 `Default(=Sword) / Tachy / Fusion / Gun계열 / Fishing / 특수(사망·동결·튜토리얼)`로 나뉜다. **평시 스탠스가 없다** — Eve는 항상 무장 상태다.
우리 초안(승환): `Default(평시 전투 = Sword&Gun) / Gun(진짜 사격만) / 사망`.
→ ~~사격 로직 착수 전까지 보류~~ → **(a) 홀드 조준으로 구현 완료됐다** (상단 08-26 실측 참조. (a)/(b) 비교 원문은 archive §1-B).

### ② 공중 콤보 재설계 — 뼈대는 완성. 남은 건 "내리꽂기 마무리" 하나 (2026-08-26 실측)
```
DA_AirComboTree   Air_1~4 선형 체인(분기 X) · DamageMult 0.9/1.0/1.2/1.8 · Knockback 0.3/0.3/0.3/1.2
                  InputWindow · PoiseMultiplier 는 4노드 전부 0
몽타주 4개         전부 CancelWindow 배선됨. 판정은 홀짝 교대 —
                  Air_01·03 = ShotBlast(총, MeleeTrace 없음) / Air_02·04 = MeleeTrace(검)
중력 억제          C++ 구현됨 (UKDGameplayAbility_PlayerAirCombo 의 OrigGravityScale 캐시·복원)
소유               콤보트리는 GA 가 아니라 BP_SBPlayer 의 ComboComponent 가 들고 있다
```
**남은 것 = 막타 뒤 내리꽂기로 지상 콤보 연결.** 재료 `02_Attack/14·15·16_Attack_Air_to_Floor`
3세트(각 Start/Loop/End) 전부 미사용 상태로 대기 중. 승환 지시 = 잠시 대기.

<details><summary>종전 서술 (접힘)</summary>
`Air_01`이 **총 클립**이라 1타에 검 판정이 없다(`MeleeTrace` 없음이 의도).
방향: **07 유지 + 14~16 `Attack_Air_to_Floor`를 마무리로 붙여 지상 콤보로 연결**(공중 공격 → 지상 찍기 → 자연스럽게 지상 콤보).
`02_Attack` 폴더는 **전부 루트모션** — InPlace 클립이 없다. "제자리 공중 공격"은 RM을 끄는 게 아니라 **이동량 0인 클립**으로 얻어야 한다.

</details>

### ③ 스태미나 폐기 — ✅ 실행돼 있었다 (2026-08-26 실측. 이 항목은 종결)
```
소모 GE 3개(Dodge/FullSprint/RegenBlock)   참조 0 고아 — 어떤 GA·컴포넌트도 안 씀
GA_Dodge · GA_HeavyCombo Cost              null
SprintComponent                            코스트 프로퍼티 자체가 없음
살아있는 것                                  GE_StaminaRegen (BP_PlayerState) + Stamina 어트리뷰트
스킬 코스트 몫                               C++ UKDGameplayEffect_StaminaCost 뼈대 보존 — 스킬 만들 때 꽂는다
```
잔가지 = 고아 GE 3개를 지울지(에셋 정리 때 같이).

### ④ 락온 애니 — ✅ 항목 자체가 성립 안 했다 (2026-08-26 종결)
**락온 전용 DB 가 없는 게 맞고, 필요도 없다** (승환 확인). 락온하면 전투 상태로 들어가고
**Combat DB 를 여는 스위치는 `bIsInBattleStance`(InCombat)** 이라, 락온 중에도 전투용 이동 애니가 나온다.
```
Walk_Combat 40/40 · Run_Combat 42/55 클립이 PSD_SB_Loco_Combat · Stops_Combat 에 배선 완료
CHT_LocomotionDatabase = Unarmed·Combat·Aim 3세트 x Idle/Loco/Stop 9 PSD 전부 연결
```
미배선 잔여 = `11_Run_Combat_Fast` 13클립(스프린트 락온 이동)뿐. 필요해지면 그때.

### ⑤ 미착수 폴리싱
~~트레일 NS 27개 미배정~~ ✅ 완료 (07-31) / ~~사운드 3대 배선~~ ✅ 완료 (아래) / ~~데미지 GE 26노드~~ ✅ DamageMultiplier 방식으로 대체 배선됨 / LoP식 방사형 회피 이펙트 보류.

> **★ 사운드는 2026-08-26 재실측으로 "구멍" 딱지를 뗐다** — 상세 = 메모리 `reference_project_sound_inventory`
> ```
> 발소리     ✅ 03_Walk 112 + 04_Run 84 시퀀스에 PlaySound 직결 -> Audio/Footstep/ MetaSound 11종
> 검 타격    ✅ 큐 3개를 콤보 몽타주 23개가 참조. Swing 엔 SW_Slash 믹스 적용됨.
>            톤은 "소재 대기"(승환 방침 — 괜찮은 소리 올 때까지 현행 유지). Audio/Sword/ 미배선 후보 6개
> 총성       ✅ Audio/Shotgun/ 땜빵 소재를 총 몽타주 3개에 노티 배선 (승환: 별로지만 유지)
> ```
> **남은 진짜 구멍 3종 (재료 임포트됨·배선 0)** = 발검/납검음(소재도 없음) · 피격 보이스(Audio/HitReact/ 7개) · UI음(Audio/UI/ 4개). 우선순위 낮음 — 뒤로 미룸 확정(2026-08-26 승환).

### ⑥ 카메라 — ✅ 1·2단계 전부 완료 (2026-08-26 실측 확인. 이 항목은 종결)
```
1단계 값 이식     07-31 완료 (Content e542cb3). 08-26 CDO 실측 재확인 —
                 382 / Y40 / 랙 on 19·57 / Probe 10 / 피치 -89~+45 / FOV 75 + 커브 3키 전부 SB와 일치
                 SocketOffset.Z 만 104 (표의 111에서 승환 튜닝, 유지)
2단계 스플라인 돌리  구현돼 있음 — UKDSpringArmComponent(SampleRail) + 레일 2개
                 CameraDollySpline 3점 = SB 기본 궤도 그대로 / AimDollySpline = 자체 튜닝(SB엔 데이터 없음)
```
남은 것 = 작업 아님. ①카메라 레일 재학습(승환 공부 — 완성 코드+볼트 06_스플라인_돌리 같이 보며, 헷갈린 지점 3개)
②참고: LagSpeed 19 는 SB 최고속 500 기준 — 우리 800 에서 같은 체감 원하면 30 (현 체감 문제 없으면 유지)

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

