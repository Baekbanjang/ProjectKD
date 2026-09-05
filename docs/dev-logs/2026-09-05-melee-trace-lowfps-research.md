# 2026-09-05 — 저프레임 판정 누락 진단 + 프레임 보간 조사

승환 관측 = **"TipLine 이든 Sweep 이든 프레임이 내려가면 트레이스가 안 나온다."**

~~이 세션은 **조사와 진단만** 했다. 구현은 아직 없다. 코드 변경 0.~~

> ## 🟢 갱신 (2026-09-05 오후·야간 — 오전 기록이 반나절 만에 낡았다)
>
> 위 문장은 **오전 세션 기준**이고 지금은 거짓이다. 볼트 세션이 13:36 에 소스 mtime 으로 잡아냈다.
> 📌 교훈 = **"아직 안 했다" 도 "했다" 만큼 빨리 낡는다.** 오전에 *"구현 0, 고쳤다고 적지 말 것"* 이라 못 박아서 볼트 노트 5곳이 같이 낡았다.
>
> ```
> 구현 완료   모드 4종(Sweep · TipLine · ArcSweep · ArcTri) + 공통 3건    빌드·PIE 통과
> 추가 구현   ArcBulge · TraceSegments 프로퍼티 노출 + 디버그 점 2색       빌드 통과
> GA 배정     LightCombo=ArcTri(촬영 후 ArcSweep 으로 되돌림) · HeavyCombo=ArcSweep · SprintAttack=Sweep
> 촬영        E 근접 모드 4종 · E 프레임보간 30/60  (탑다운 시점)
> ```
>
> **PIE 실측 (t.MaxFPS 10, 승환)**
> ```
> ArcSweep   판정 잘 나온다
> ArcTri     판정은 나오는데 궤적에 짤리는 구간이 있다
>            → 원인 = TraceSegments 3 이라 칼 축 방향 점이 4개뿐 (간격 33cm)
>              ArcSweep 은 캡슐 몸통이 그 사이를 덮어서 안 짤린다
>              ⚠️ 서브스텝(스윙 진행 방향)과 다른 축이다. 이 구분이 세션 내내 가장 헷갈렸다
> ```
>
> **🔴 ArcBulge 는 1.0 이 정답이다** — 승환이 5 를 넣었을 때 궤적이 꽃잎처럼 부풀었다.
> 그 화면을 보고 KD 가 *"제어점 공식이 두 겹으로 틀렸다(OutDir 공용 + 미는 거리 임의값)"* 고 진단했으나 **틀렸다.**
> 1.0 에서 정상이므로 공식은 쓸 만하다. ⚠️ `OutDir` 을 자루·칼끝에 공용으로 쓰는 구조적 문제는 이론상 남아 있으나 1.0 에서 눈에 띄지 않는다.
>
> **🟡 미해결 — 프레임당 회전각이 큰 스윙**
> 10fps 큰 횡베기는 판정창에 실제 프레임이 2~3개뿐인데 칼은 그 사이 크게 돈다. 베지어는 **두 점을 부드럽게 잇는 것**이지 회전 경로를 복원하지 않는다.
> ```
> 다음 수 후보 = FQuat::Slerp 회전 보간 (칼 축 방향 2개로 그 사이 회전을 구한다)
>   한계 = 프레임당 180도. 넘으면 FindBetweenVectors 가 짧은 쪽으로 해석해 반대로 돈다
>   ⚠️ 우리 AM_SB_Combo_01_01 이 판정창에서 몇 도 도는지 측정 안 했다 → 효과 불확실
> 결정 = 포폴 문장("프레임 보간을 구현했다")이 안 바뀌므로 구현하지 않는다 (승환·KD 합의)
> ```
>
> 별건 = `2026-09-05-approach-warp-rotation-flip.md` (접근 워프 목표점 뒤집힘, 같은 날 별개 주제)

목표 = 포폴에 **"프레임 보간을 구현했다"** 를 넣는 것(승환).

---

## 1. 현행 판정 구조 실측

### 흐름

```
ANS_MeleeTrace NotifyBegin
  -> Event.Montage.TraceBegin
  -> UKDGameplayAbility_MeleeTrace::OnTraceBeginEvent
  -> UKDAbilityTask_MeleeTrace 생성 + ReadyForActivation
  -> 매 틱 TickTask
ANS_MeleeTrace NotifyEnd
  -> Event.Montage.TraceEnd
  -> OnTraceEndEvent -> EndTask()
```

### 위치를 어디서 읽나

```cpp
// KDAbilityTask_MeleeTrace.cpp:60-61
const FVector CurStart = WeaponMesh->GetSocketLocation(StartSocket);
const FVector CurEnd   = WeaponMesh->GetSocketLocation(EndSocket);
```

★ **`GetSocketLocation` = 그 프레임에 렌더링된 결과 포즈.** 애니 원본이 아니다. **이 줄이 모든 문제의 뿌리다** — 화면에 그려진 것만 읽으니, 안 그려진 순간의 검 위치는 알 방법이 없다.

### 모드 2개

```
TipLine  :118-134   PrevEnd -> CurEnd 라인 1개. 두께 0. 서브스텝 없음
Sweep    :136-181   무기 축 캡슐을 Mid0 -> Mid1 로 스윕. 서브스텝 1~8
```

```cpp
// :140-144
const float TravelDist = (CurMid - PrevMid).Size();
constexpr float StepDist = 5.0f; // cm
const int32 SubSteps = FMath::Clamp(FMath::CeilToInt(TravelDist / StepDist), 1, 8);
```

### 실제 설정값 (MCP 실측)

```
GA BP CDO 10개        전부 Sweep
                      CapsuleRadius 20  (GA_CounterSlash 만 3.0)
                      bDrawDebug True 9 / GA_CounterSlash 만 False
ANS_MeleeTrace 48개   bOverrideTraceMode 전부 False
                      = 노티가 덮는 곳 0. GA CDO 값이 그대로 간다
```

🔴 **`GA_CounterSlash` 만 `CapsuleRadius = 3.0`.** `PROJECT_OVERVIEW §2-5` 의 *"새 근접 GA BP 는 20으로 올려라"* 에 걸린 상태. 08-28 신설 때 누락. **이번 범위 밖이라 손대지 않았다.**

📌 3개월째 미정리였던 헤더/생성자 불일치도 확정했다 — `.h:60` 이 `TipLine` 인데 `.cpp:22` 생성자가 `Sweep` 으로 덮고, **BP CDO 도 전부 Sweep** 이다.

---

## 2. ★★ 원인 셋 — 판정 구간의 60% 를 안 보고 있다

### ① 첫 구간을 버린다

```cpp
// KDAbilityTask_MeleeTrace.cpp:64-70
if (!bHasPrevFrame)
{
    PrevStart = CurStart;
    PrevEnd = CurEnd;
    bHasPrevFrame = true;
    return;              // 판정 없이 나감
}
```

비교할 `Prev` 가 없어 첫 틱은 위치만 적고 끝낸다. **고프레임에선 티가 안 나고 저프레임에선 치명적이다.**

### ② 마지막 구간도 버린다

```cpp
// KDGameplayAbility_MeleeTrace.cpp  OnTraceEndEvent
ActiveWindow = nullptr;
if (ActiveTraceTask)
{
    ActiveTraceTask->EndTask();      // 마지막 판정 없이 종료
    ActiveTraceTask = nullptr;
}
```

**마지막 `TickTask` 이후 ~ `NotifyEnd` 사이 구간을 한 번도 판정하지 않는다.**

⚠️ **이건 승환이 짚어서 찾았다.** *"첫 프레임과 마지막 프레임은 무조건 살아있어야 하는 거 아니냐"* — 나는 시작만 보고 끝은 안 봤다.

### 숫자 — `AM_SB_Combo_01_01` 판정 구간 0.167초 기준

```
fps    프레임 간격   구간 내 틱   첫 틱 버림   실제 판정
60      0.0167s       10          -1          9
30      0.0333s        5          -1          4
15      0.0667s        2          -1          1
10      0.1000s        1          -1          0    <- 판정 0회
```

15fps 실제 타임라인:

```
0.000  노티 시작        누구도 이 위치를 안 읽는다      [버림]
0.067  1틱  위치만 적고 return                        [버림]
0.134  2틱  0.067~0.134 판정                          ✅ 유일
0.167  노티 끝          0.134~0.167 판정 없음          [버림]

판정된 구간 = 0.067 / 0.167 = 40%
```

⚠️ `PlayRate` 가 콤보 템포로 올라가면 구간이 더 짧아진다(1.5배면 0.111초).
⚠️ **미확인** — `ReadyForActivation()` 한 프레임에 `TickTask` 가 도는지 다음 프레임부터인지 엔진 코드를 안 봤다. 다음 프레임부터라면 틱이 하나 더 빠진다.

### ③ 직선으로 잇는다

```cpp
// :151-154
const FVector S1 = FMath::Lerp(PrevStart, CurStart, A1);
const FVector E1 = FMath::Lerp(PrevEnd,   CurEnd,   A1);
```

검은 어깨를 축으로 도니 칼끝은 **호**를 그린다. `Lerp` 는 두 끝점을 **현(chord)** 으로 질러버린다. 서브스텝을 8개로 쪼개도 8점이 전부 그 직선 위라 **곡률 오차는 안 줄어든다.**

★ **`CapsuleRadius 20` 이 이 오차를 덮고 있었다.** *"새 근접 GA 는 반지름 20으로 올려라"* 는 규칙의 정체가 이것이다 — 곡률 문제를 두께로 때우고 있었다.

---

## 3. SB 가 한 것 (발표 03:43~05:35 재판독)

원본 = `2026-08-31-SB언리얼페스트2024-전수판독.md` §4. 영상 = `youtube.com/watch?v=IL9j4NchTvA`

### 판정 방식이 둘이고 역할이 나뉜다 (03:54 · 04:06)

```
Triangle-Hitbox Intersection
  소켓들의 이전/현재 프레임 위치를 Triangle 로 구축하여 판정
  용도 = 검처럼 얇은 무기 · 정확한 검출

Hitbox Sweep Trace
  ShapeComponent 의 이전 -> 현재 위치를 Sweep Trace
  용도 = 몸통 박치기 · 느리고 큰 무기 · 영역 판정
  ⚠️ 삼각형보다 정확도가 낮다
```

🔴 **우리는 아래쪽이고, SB 가 "큰 무기용"이라 분류한 걸 검에 쓰고 있다.**

### 삼각형 구성 (04:12)

```
5Frame 소켓 2점(칼끝·칼자루) + 6Frame 소켓 2점
  -> 사각형을 만들고 대각선을 그어 삼각형 2개
  -> 이 삼각형과 히트박스의 교차로 판정
```

### 그런데 삼각형만으로 부족했다 (04:3x)

```
정확도가 떨어지는 두 경우
  ① 공격이 빠른 경우      궤적이 직선으로 끊김
  ② FPS 가 낮은 경우      궤적 중간이 통째로 누락
```

★★ **발표 순서가 답이다.** 삼각형을 먼저 소개하고 **그다음에** 이 문제를 꺼내 보간을 도입한다. **SB 도 삼각형만으로는 안 됐다.**

### 해결 = Transform 보간 (05:00)

```
UAnimSequence 의 RawAnimationData 를 이용
Socket 의 Parent Bone Transform 을 보간하여,
이전 프레임과 현재 프레임 사이의 Bone Transform 계산
```

결과 = 30FPS 에서도 60FPS 와 동등한 궤적 밀도(05:24 좌우 비교).

### ⚠️ SB 가 밟은 2차 함정 (§4-4)

```
IK 회전 후의 실제 본 위치 ≠ RawAnimationData 에 저장된 본 위치  -> 궤적 어긋남 재발
2차 보정 = 원본 Transform 과 Animated Bone Transform 의 차이를 구해 보간에 적용
```

📌 **우리도 해당된다** — 모션 워핑을 쓴다(`AnimNotifyState_MotionWarping`, 01_01 t=0.183).

---

## 4. 웹 조사 — 알려진 문제다

### 영어권

Epic 포럼에 동일 스레드 다수. 원인 진술이 우리 진단과 같다 — *"히트 판정이 애니메이션 틱에 묶여 있어, 프레임이 스킵되면 트레이스도 스킵된다."*

```
❌ CCD (Use CCD)     직선 발사체용. 호를 그리는 검엔 한계. "켜도 스킵된다" 보고 있음
✅ 표준 해결          이전 틱 -> 현재 틱 스윕 + 이동거리 기반 서브스텝
   MeleeTrace (GitHub, rlewicki) · Enhanced Melee Trace (itch.io)
```

★ **그 표준이 우리 `:142-144` 와 같다.** 구조가 틀린 게 아니었다.

### 🔴 UE5.6 API 벽 — SB 방식의 편한 길이 막혔다

```
UAnimationBlueprintLibrary::GetBonePoseForTime / GetBonePosesForTime
  UE5.2 부터 deprecated + Editor 모듈 소속 = 패키징된 런타임에서 사용 불가
AnimPose.h 도 같은 Editor 모듈 = 툴 전용

런타임 가능:
UAnimSequence::GetBoneTransform(FTransform& OutAtom, FSkeletonPoseBoneIndex BoneIndex,
                                const FAnimExtractContext&, bool bUseRawData)   AnimSequence.h:532
  -> 본 하나의 부모 기준 Transform 만 준다
  -> 월드 좌표는 부모 체인을 루트까지 직접 곱해 올라가야 한다
```

조사 결론 = **"프레임 사이 애니 원본에서 본 궤적을 복원하는 깨끗한 런타임 공식 API 는 UE5.6 에 없다."**

⚠️ 우리는 한 단계 더 붙는다 — **검이 캐릭터 뼈가 아니라 `hand_r` 에 부착된 별도 StaticMesh** 라 `hand_r` 월드 Transform 에 부착 오프셋을 다시 곱해야 한다.

### 🔴 벽 하나 더 — 삼각형을 콜리전 도구로 못 쓴다 (엔진 소스 실측)

승환이 *"적 콜리전이 삼각형에 안 걸린다는 거냐"* 고 물어서 확인했다. **적 문제가 아니라 우리가 던질 도형이 없는 것**이다.

```
Runtime/PhysicsCore/Public/CollisionShape.h
  :284 MakeBox   :292 MakeBox(3f)   :300 MakeSphere   :308 MakeCapsule   :316 MakeCapsule(Extent)

박스 · 구 · 캡슐 3종이 전부.  MakeTriangle 없음
-> SweepMulti / OverlapMulti 에 삼각형을 넘길 창구가 없다
```

⚠️ **`MakeLine` 도 없다.** 세션 중 내가 "있다"고 말했으나 목록에 없다 — 선 검사는 `FCollisionShape` 이 아니라 `LineTraceMultiByObjectType` 이라는 **별도 함수**다.

```
FMath::SegmentTriangleIntersection   UnrealMathUtility.h:2157
  선분 vs 삼각형.  순수 수학 함수라 월드 콜리전과 무관 = 적이 어디 있는지 모른다
  쓰려면 적 캡슐을 직접 수집해 인자로 넣어야 한다
```

→ **우회 = 삼각형의 변을 `LineTrace` 로 훑는다.** 이게 `ArcTri` 가 선 다발인 이유다.

### 🔴 캡슐 스윕은 회전을 보간하지 않는다 (시그니처에서 확정)

```cpp
SweepMultiByObjectType(Hits, Mid0, Mid1, Rot1, ...)   // World.h:2189
//                                       ^^^^ 회전 인자가 하나뿐
```

**캡슐이 `Rot1` 자세로 고정된 채 평행이동만 한다.** 칼이 크게 회전하는 구간에서 근사가 거칠어진다. 서브스텝이 촘촘할수록 줄어든다.

★ **이게 삼각형 방식의 두 번째 이유다.** 삼각형은 네 꼭짓점이 실제 소켓 위치라 회전이 자동으로 반영된다. **세션 중반에 "삼각형 실익이 작다"고 했던 내 판단을 이걸로 정정했다** (§6 ④).

### 서브스텝 = 칸 수. 점은 그보다 하나 많다

```
SubSteps = 5  ->  경계 6개  ->  칼 6개  ->  점 12개 (자루 6 + 칼끝 6)
```

**프레임이 아니라 한 틱 안에서 계산으로 만든 중간 위치**다. 세션 내내 이 구분이 가장 많이 헷갈렸다.

### SB 디버그 색 규칙 (출처 = 승환)

```
빨강 = 현재 프레임 위치     노랑 = 보간으로 만든 위치
```

발표 화면에서 **노란 점이 빨간 점보다 훨씬 많다** = 프레임 하나 사이에 샘플을 여러 개 끼워 넣는다는 뜻이다. **우리 서브스텝과 같은 층**이고, 다른 건 좌표 출처뿐이다(애니 원본 vs `Lerp`).

⚠️ 발표에 색 범례가 없어 **KD 는 이 규칙을 몰랐다.** 승환이 알려준 것이다.

### 한국 자료 — 방식이 셋으로 갈린다

| 출처 | 방식 | 곡률 |
|---|---|---|
| **velog `hoi000115`** | `PerformTriangleTrace` (삼각형) + **2차 베지어** | ✅ |
| `funfunhanblog.tistory.com/554` | 선 5개 + 3분할 = 선 8개 (그물) | ❌ |
| 우리 · 플러그인 2개 | 캡슐 스윕 | ❌ |

**velog 저자가 겪은 문제 원문:**
> 1. 프레임이 떨어지게 되면 충돌처리가 발생하지 않을 수 있다.
> 2. 공격속도가 빨라지게 되면 충돌처리가 발생하지 않을 수 있다.

**승환 관측과 같은 문장이고, 이 사람도 언리얼페스타 SB 세션을 보고 전환했다.**

★★ **애니 원본을 안 읽고 곡률을 만든다 — 2차 베지어.**

```cpp
FVector ControlPointStart = (PrevStartLocation + CurrentStartLocation) / 2;
FVector ControlPointEnd   = (PrevEndLocation   + CurrentEndLocation)   / 2;
FVector ControlPointDirection = (ControlPointStart - ControlPointEnd).GetSafeNormal();
ControlPointStart = ControlPointStart + ControlPointDirection * -NumStepsStart;
FVector InterpolatedStart = BezierCurve(PrevStartLocation, ControlPointStart,
                                        CurrentStartLocation, Alpha);
```

```cpp
float InterpolationStep = 5.0f;
int32 NumSteps = FMath::CeilToInt(DistanceStart / InterpolationStep);
```

📌 **보간 간격 `5.0f` 와 `CeilToInt` 나눗셈이 우리 `:143-144` 와 동일하다.** 다른 건 둘뿐 — **우리는 상한 8 로 자르고, 우리는 `Lerp`(직선) 이다.**

⚠️ `PerformTriangleTrace` 와 `BezierCurve` 본문은 **비공개**. 호출부만 공개돼 있다.

---

## 5. 결정 — 4단계 + 나중에 삼각형

### 왜 베지어가 먼저이고 삼각형이 나중인가

승환 질문 = *"삼각형을 넣어 면으로 만들면 검출이 나아지는 것 아닌가? 베지어까지 굳이?"*

**아니다. 둘은 다른 문제를 푼다.**

```
삼각형   프레임 사이를 면으로 채운다   -> "선 사이가 빈다" 해결
베지어   그 면의 변을 휘게 한다        -> "직선이라 호 바깥이 빈다" 해결
```

삼각형도 **4점을 직선으로 이어 만든 도형**이라, 바깥 테두리는 여전히 직선이다.

🔴 **그리고 삼각형만 넣으면 검출이 오히려 나빠진다.**

```
                경로    두께      결과
지금 (캡슐)      직선    20cm     곡률 오차를 두께로 때움
삼각형만         직선    0        🔴 완충이 사라져 더 나빠짐
삼각형 + 베지어   곡선    0        곡률 해결
캡슐 + 베지어     곡선    20cm     가장 넉넉
```

### 채우는 방식 — 큰 면 1개가 아니라 작은 면 여러 개

```
직선   큰 사각형 1개 -> 삼각형 2개
곡선   작은 사각형 N개 -> 삼각형 2N개.  조각들이 곡선을 따라 늘어선다
```

★ **쪼개는 구조는 이미 있다**(`:146-165` 의 `for (Step)` 루프). **점 좌표 공식만 `Lerp` → `Bezier` 로 바뀐다.**

### 실행 순서

```
1  ① 첫 구간 살리기      Activate() 에서 Prev 미리 채움              3줄
   ② 마지막 구간 살리기   OnTraceEndEvent 에서 최종 판정 1회          10줄 안팎
   ③ 서브스텝 상한 해제   Clamp(...,1,8) -> 32                       1줄
   ④ Lerp -> 베지어      제어점 = 중간점을 회전중심 반대로 NumSteps 만큼  20줄 안팎
   -> t.MaxFPS 15 로 before/after 측정

2  ⑤ 캡슐 -> 삼각형 면    SB 방식. 04:12 다이어그램 재판독 선행 필요
```

📌 **①② 만으로 15fps 판정 구간이 40% → 100% 가 된다.** ④는 그 구간 안의 정확도다.

### 제어점 방향 — 회전 중심의 **반대쪽**

```
❌ 회전 중심(어깨·칼자루)에 두면  ->  곡선이 안으로 오목
✅ 중간점을 회전 중심 반대(칼끝 방향)로 밀면  ->  바깥으로 볼록
미는 거리 = NumSteps = 이동거리 ÷ 5  ->  저프레임일수록 자동으로 더 휜다
```

우리 구조가 velog 와 같다 — `StartSocket = Sword_Bottom`(자루, 회전중심 역할) / `EndSocket`(칼끝, 바깥).

### 🔴 미확인 — 삼각형 교차를 UE5 에서 어떻게 하나

```
아는 것    "사각형에 대각선 그어 삼각형 2개로 판정"  (발표 04:12)
모르는 것   그 면-히트박스 교차의 구현.  LineTrace 로 변을 훑나? 별도 수학인가?
```

발표에 안 나오고 velog 본문도 비공개. **⑤로 가기 전 04:00~04:30 프레임 재판독이 필요하다.**

---

## 6. ★ 이번 세션에 내가 틀린 것

**① "우리 방식이 표준이다" 는 절반만 맞았다**
플러그인 2개 기준으로는 맞지만, **SB 기준으로는 "정확도 낮은 쪽"** 이다. 처음엔 표준이라고만 말했다.

**② 마지막 구간 누락을 놓쳤다**
`Activate()` 의 첫 틱만 보고 `OnTraceEndEvent` 는 안 봤다. **승환이 물어서 찾았다.**

**③ "첫 틱 살리기로 해결된다" 고 말했다**
승환 지적 = *"근본적인 원인을 해결할 수 있는 건 아니지 않나."* **맞다.** 그건 판정 횟수를 1 늘리는 응급처치고, 뿌리는 `GetSocketLocation` 이 프레임에 묶여 있다는 것이다. 얼버무렸다.

**④ 서브스텝을 "이미 있으니 괜찮다" 는 뉘앙스로 말했다**
서브스텝이 막는 건 **터널링(듬성듬성 건너뜀)** 이고, 지금 문제는 **틱이 아예 안 도는 것 + 곡률**이다. 다른 문제다.

---

## 7. 검증 / 측정 방법

```
t.MaxFPS 15        프레임 강제 (UnrealEngine.cpp:11745 CVarMaxFPS "Caps FPS to the given value")
                   ⚠️ VSync 켜져 있으면 안 먹는다 -> r.VSync 0
stat fps           먹었는지 확인
slomo 0.2          시간만 늦춤. 프레임 수는 그대로 = 판정은 안 샌다 (눈 검증용)
KD.ShowDamage 1    피해량 표시 = 판정 누락 확인
bDrawDebug         GA CDO 체크박스. 9개 이미 True
```

★ **선/캡슐 개수가 곧 판정 횟수다.** `TipLine` 은 틱당 선 1개(`:132`), `Sweep` 은 서브스텝마다 캡슐 1개(`:178`).

---

## 8. 구현 — 코드 입력 완료 · 빌드 대기

승환이 직접 입력했고 KD 가 검토했다. **빌드는 아직 안 돌렸다.**

### 모드 4종 구조

```cpp
enum class ETraceMode : uint8
{
    Sweep,      // 캡슐 + 직선     기존. 손대지 않음 (대조군)
    TipLine,    // 칼끝 라인 1개    기존. 손대지 않음
    ArcSweep,   // 캡슐 + 베지어    두께 20 유지
    ArcTri      // 선 격자 + 베지어  두께 0. SB 방식
};
```

★ **enum 은 끝에만 붙였다.** `uint8` 열거형은 에셋에 숫자로 저장되므로 중간에 끼우면 기존 GA BP 의 저장값이 다른 항목을 가리킨다.

### 공통 수정 3건 — 모드와 무관

```
① Activate() 에서 Prev 프리필        판정 창 진입 시점 위치를 미리 채운다
                                    KDAbilityTask_MeleeTrace.cpp:70-72
② TickTask -> TraceOnce() 분리       GA 가 종료 직전에 한 번 더 부를 수 있게 public
                                    태스크 :94 · GA :183
③ Arc 서브스텝 상한 32               MaxSubStepsArc. Sweep 은 8 유지
```

🔴 **①에서 버그가 하나 났다** — `bHasPrevFrame` 을 `false` 로 둬서 첫 틱이 그 값을 덮어쓰고 `return` 했다. **위치를 채워놓고도 첫 구간이 그대로 버려지는 상태**였다. `true` 로 고쳤다.

### 함수 분리 (승환 요청)

`TraceOnce` 안의 `if/else` 덩어리를 모드별 함수로 갈랐다.

```
TraceOnce()        공통 준비 -> switch 분기 -> Prev 갱신        약 40줄
 ├ TraceTipLine()  PrevEnd -> CurEnd 선 1개
 ├ TraceSweep()    Lerp 좌표 + 캡슐 스윕
 └ TraceArc()      Bezier2 좌표 + ArcSweep/ArcTri 분기
ProcessHits()      히트 필터링 + 액터당 1회 브로드캐스트
IsWallBlocking()   벽 차단 유무
```

`World` · `Owner` 는 인자로 안 넘긴다 — 각 함수에서 `GetWorld()` · `GetAvatarActor()` 로 구한다. 인자가 4개로 줄었다.

📌 죽은 변수 `bAnyHit` 을 제거했다. 읽는 곳이 한 군데도 없었다.
📌 `TraceSweep` 안의 로컬 `constexpr float StepDist` 와 하드코딩 `8` 을 네임스페이스 상수로 통일했다.

### 튜닝 값은 에디터에 노출하지 않았다 (승환 판단)

```cpp
constexpr float StepDist         = 5.0f;   // 서브스텝 간격 cm
constexpr int32 MaxSubStepsSweep = 8;
constexpr int32 MaxSubStepsArc   = 32;
constexpr float ArcBulge         = 1.0f;   // 베지어 볼록 배율
constexpr int32 TraceSegments    = 3;      // 칼 축 분할 수
```

`.cpp` 익명 네임스페이스라 **바꾸려면 재컴파일**이 필요하다. 처음엔 `UPROPERTY` 노출을 제안했으나 승환이 뺐다.

### ⚠️ 소켓은 추가하지 않았다

승환이 *"칼 메쉬에 축을 찍는 게 나을까"* 물었으나 **불필요**하다. 칼이 직선이라 자루·칼끝 두 소켓만 있으면 중간은 `Lerp` 로 나온다. 소켓으로 박으면 분할 수를 못 바꾼다.

⚠️ **예외** — 검이 휘어 있으면(곡도) 중간 소켓이 필요하다. **우리 `Sword.uasset` 이 직선인지 확인 안 했다.**

## 9. 남은 것

```
빌드              ⚠️ .h 전방선언 struct FCollisionQueryParams / FCollisionObjectQueryParams
                 엔진이 class 로 선언했으면 컴파일 에러. 그때는 include 로 교체
GA CDO 설정        LightCombo=ArcTri · HeavyCombo=ArcSweep · SprintAttack=Sweep
PIE 측정          t.MaxFPS 10 + KD.ShowDamage 1 로 before/after
ArcTri 튜닝        두께 완충이 사라져 덜 맞을 수 있다 -> TraceSegments 3 -> 5 재컴파일
EndSocket 이름     확인 안 했다 (StartSocket = Sword_Bottom 만 확인)
검 메쉬 직선 여부   곡도면 중간 소켓 필요
GA_CounterSlash   CapsuleRadius 3.0 -> 20 (이번 범위 밖. 별건)
삼각형 면 교차     Unreal Fest 04:00~04:30 재판독 · Gold Coast 2024 영상 미시청
                 선 다발로 부족하면 그때 (점 4개 만드는 부분은 공통이라 판정 함수만 교체)
ReadyForActivation 같은 프레임에 TickTask 가 도는지 엔진 코드 미확인
```

## 커밋

```
코드      KDAbilityTask_MeleeTrace.h / .cpp     모드 2종 · 함수 분리 · 베지어
          KDGameplayAbility_MeleeTrace.cpp      TraceOnce() 호출 1줄
          ⚠️ 빌드 전. 통과 확인 후 커밋할 것
Content   GA_LightCombo (TipLine 전환, 진단용)
          ⚠️ Map/LV0_Test · GA_HeavyCombo · GA_ShotBlast = 내가 안 건드린 것
```

## 참고 링크

```
Epic 포럼   forums.unrealengine.com/t/melee-hit-trace-detection-at-lower-fps/484749
            forums.unrealengine.com/t/sphere-trace-melee-on-lower-fps/137216
            forums.unrealengine.com/t/even-with-ccd-on-sword-collision-skips-overlaps.../103622
플러그인     github.com/rlewicki/MeleeTrace
            pantheraonline.itch.io/enhanced-melee-trace-for-ue5
한국         velog.io/@hoi000115  (삼각형 + 베지어. 이 세션 최고 참고자료)
            funfunhanblog.tistory.com/554  (선 그물)
발표         youtube.com/watch?v=IL9j4NchTvA  (SB, 03:43~05:35)
            dev.epicgames.com/community/learning/talks-and-demos/0y7a/  (Gold Coast 2024)
```
