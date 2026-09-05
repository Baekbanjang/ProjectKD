# 2026-09-05 — 접근 워프 목표점 뒤집힘 (적에게 등 돌리는 현상)

승환 관측 = **"라이트 콤보가 자꾸 정면에서 갑자기 적이 바라보는 방향으로 몸을 틀 때가 있다."**
추가 조건 = **"적이 여러 명인데 다 죽이고 한 마리만 남았을 때 발생."**

수정 = `KDGameplayAbility_PlayerMelee.cpp:172` **한 글자** (`true` → `false`). 빌드 통과 · PIE 확인 완료(승환 = "잘된다잉").

---

## 1. 증상 — 영상 실측

캡처 `2026-09-05 18-22-27.mp4` 38.4~40.0초. ffmpeg 0.2초 간격 추출.

```
38.4   플레이어가 적을 향해 있다            정상
39.0   플레이어 앞 · 적 뒤
39.2  🔴 플레이어가 등을 보인다. 적과 같은 방향을 향함
39.4  🔴 둘이 완전히 겹친다
39.6  🔴 적 뒤쪽에서 엉뚱한 데로 검을 뻗는다
39.8   다시 적을 향한다                     복구
```

**약 0.4초.** 다음 타가 발동하면 자동 조준 스냅이 다시 적을 향하게 해서 저절로 풀린다. 그래서 "잠깐 홱 돌았다 돌아온다"로 보인다.

📌 **"적이 바라보는 방향"이라는 승환 표현이 결함을 정확히 짚은 것이다** — 적 AI는 플레이어를 본다. 플레이어가 적에게 등을 돌리면 = 적과 같은 방향을 본다.

---

## 2. ★ 확정 원인 — 게이트는 1회, 목표점은 매 프레임

### 워프 등록 (수정 전)

```cpp
// KDGameplayAbility_PlayerMelee.cpp:168-175
if (Dist > MaxApproachRange || Dist <= ApproachStopDistance) return;   // 게이트
Warp->AddOrUpdateWarpTargetFromComponent(
    ApproachWarpName, TargetRoot, NAME_None, true,                     // bFollowComponent
    EWarpTargetLocationOffsetDirection::VectorFromTargetToOwner,
    FVector(ApproachStopDistance, 0.f, 0.f), FRotator::ZeroRotator);
```

```
목표점 = 적 위치 + ApproachStopDistance(140) × (적 → 나 방향)
       = "적 앞 140cm, 내 쪽"
```

### 부호가 뒤집힌다

`나 → 목표점` 벡터 = `(140 − d) × (적 → 나 방향)`. **`d`가 140 아래로 내려가면 음수가 된다.**

| 거리 d | 목표점 (나 기준) | |
|---|---|---|
| 300 | 앞 160cm | ✅ |
| 200 | 앞 60cm | ✅ |
| **140** | 내 자리 (0) | 경계 |
| 60 | **뒤 80cm** | 🔴 |
| 20 | **뒤 120cm** | 🔴 |

### 결함 = 검사와 실행의 주기가 다르다

```
게이트 :168     발동 시 1회      "d=300 이니까 안전"
목표점          매 프레임 재계산   적이 달려와 d=60 → 등 뒤로 넘어감
회전            매 프레임 적용     목표점을 바라보라 → 적에게 등을 돌린다
```

**한 번 검사하고 스윙 내내 그 결과를 믿는다.** 게이트는 정확히 이 뒤집힘을 막으려고 있는 줄인데, 1회성이라 못 막았다.

⚠️ 게이트를 매 프레임 돌리려면 `OnActivated`에 Tick이 필요한데 **GA에는 Tick이 없다.** AbilityTask를 새로 만들어야 하고 `CLAUDE.md §2-5`의 AT 깊이 룰에도 걸린다. 그래서 "검사를 늘리는" 대신 **"검사가 늙지 않게"** 고쳤다.

---

## 3. 엔진 소스 근거 (UE 5.6 실측)

`D:\epicStore\UE_5.6\Engine\Plugins\Animation\MotionWarping\`

```
RootMotionModifier.cpp:400-405   Facing 회전 = (오프셋 적용 후 목표점 − 캐릭터 위치) 방향
                                 GetTargetRotation() 이 CachedTargetTransform 을 쓰고,
                                 그 값은 RecalculateOffset() 을 거친 결과다
RootMotionModifier.cpp:74-86     bFollowComponent = true 면 매 호출(매 프레임) RecalculateOffset()
RootMotionModifier.cpp:127-140   VectorFromTargetToOwner 는 LocationOffset 이 X 만 있으면
                                 bCacheForwardOffset = false → 오너의 현재 위치 기준 매 프레임 재계산
                                 (Y 또는 Z 가 0 이 아니면 등록 시점 캐시를 재사용한다)
```

우리 호출이 `FVector(140, 0, 0)` = **X만** → 매 프레임 재계산 경로에 정확히 해당한다.

```
EWarpTargetLocationOffsetDirection (RootMotionModifier.h:171-179)
  TargetsForwardVector    기본. 타겟 정면 X축
  VectorFromTargetToOwner 타겟 → 오너 벡터가 X   ← 우리가 쓰는 것
  WorldSpace              월드 X축
```

📌 **엔진에 "이미 지나쳤으면 0으로 멈춘다"는 클램프가 없다.** 방향만 정규화해서 곱한다.

---

## 4. 왜 "한 마리 남았을 때"인가

🟡 **추론이다.** 확인 안 했다.

이 뒤집힘은 **스윙 도중 적이 140 안으로 들어와야** 난다. `kd.Encounter.MaxAttackers 2` 토큰 때문에 적이 여럿이면 대기하는 개체가 있는데, 하나만 남으면 항상 토큰을 들고 곧장 달려든다 → **붙는 빈도가 올라간다.**

---

## 5. 수정 — B안 (코드 1글자)

```cpp
// :169-172  수정 후
// 워프 목표점 등록 - 발동 시점 좌표 고정 (bFollowComponent = false)
// 오프셋 X = 적에서 나 쪽으로 ApproachStopDistance 만큼
Warp->AddOrUpdateWarpTargetFromComponent(
    ApproachWarpName, TargetRoot, NAME_None, false,
```

**목표점을 0초 좌표에 고정한다.** 적이 달려와도 안 움직이니 뒤집힐 일이 없다. 게이트가 0초에 내린 판단이 **끝까지 유효**해진다.

```
🚩 전   깃발이 적 손에 들려 있다 — 적이 움직이면 목표점도 따라온다
🚩 후   깃발을 땅에 꽂아둔다 — 적이 어디로 가든 그 자리
```

### 고정은 한 타 동안만이다

```
:134   RemoveWarpTarget()          매 발동 시 옛 목표점 제거
:137   FindAutoAimTarget()         타겟 새로 검색
:171   AddOrUpdateWarpTarget...()  새 목표점 등록
```

콤보 1타·2타·3타가 **각각 새로 계산**한다. 새 적이 더 가까워졌으면 다음 타에서 그쪽으로 잡힌다.

📌 `:134`의 `RemoveWarpTarget`은 08-20·08-31에 *"조기 return에 옛 워프 타겟이 살아남아 옛 좌표로 워프"* 를 잡느라 넣은 줄인데, 지금 이 구조를 받쳐주고 있다.

### 대가

⚠️ 스윙 중 적이 **옆으로 이동하면 워프가 적이 있던 자리로 간다.** 추적을 포기한 것이다. PIE에서 눈에 안 걸리면 이대로 두고, 걸리면 A안으로 간다.

---

## 6. 남은 것 — A안 (촬영 뒤)

**몽타주 `MotionWarping` 노티의 `Warp Rotation` 을 끈다.** 콤보 몽타주 약 21개.

```
지금    워핑이 이동 + 회전 둘 다 한다
A 후    워핑은 이동만. 회전은 :147 SetActorRotation 스냅이 발동 시 1회
```

**God of War 방식** — 첫 프레임 스냅 + 이후 회전 봉쇄 (`reference_melee_autoaim_research`).

| | 뒤집힘 | 뒤로 당김 | 어디 | 대가 |
|---|---|---|---|---|
| **A** 회전 떼기 | ✅ | ❌ 남음 | 노티 21개 (에셋) | 스윙 중 방향 추적 X |
| **B** 목표점 고정 (적용) | ✅ | ✅ | 코드 1글자 | 스윙 중 위치 추적 X |

**둘은 서로 다른 걸 고치니 겹치지 않는다.** A는 방향을 고정하고 위치는 쫓고, B는 위치를 고정하고 방향은 워핑이 만진다. **A+B 둘 다가 완성형**이고, B만으로도 증상은 사라진다.

⚠️ **B만 쓰면 회전 권한이 여전히 워핑에 있다.** 목표점이 안 움직여서 뒤집히지 않을 뿐, 구조는 그대로다.

⚠️ 파이썬으로 `AnimMontage.Notifies` 를 못 읽는다(09-04 확인). A는 에디터 수동 작업이다.

---

## 7. 🔴 이 함수 세 번째다 — `CLAUDE.md §1-5` 발동

```
08-20   접근 워프 — 조기 return 에 옛 워프 타겟 잔류
08-31   반격 워프 — 같은 뿌리 (RemoveWarpTarget 누락)
09-05   이번 — 게이트 1회 / 목표점 매 프레임
```

전부 `UKDGameplayAbility_PlayerMelee::OnActivated` 의 워프 블록이고, 전부 같은 모양이다 —
**"발동 시점에 정한 것이 시간이 지나며 틀려진다."**

앞의 둘은 *지난 활성화*의 잔재였고 `RemoveWarpTarget`으로 막았다. 이번은 **같은 활성화 안에서** 조건이 변하는 것이라 그 방어가 안 걸린다.

**입력을 의심하면** — 이 함수는 "발동 순간의 거리"를 입력으로 받아 "스윙 내내 유효한 결정"을 내린다. 적이 움직이는 이상 그 전제는 성립하지 않는다. A·B 둘 다 이 모순을 없애는 방향이고(A=회전을 결정에서 뺌 / B=결정이 늙지 않게 함), **값 조정(각도·거리)은 세 번째 증상 고치기라 하지 않았다.**

---

## 8. 진단 중 틀린 가설 2개

**① "부채꼴 90도 밖으로 나가서 다른 적을 고른다"**
`AutoAimFilter.HalfAngle 90` · `SortType Nearest` 구조상 가능한 시나리오였고, 화면에 `dist 298`·`365`가 찍히는 걸 근거로 삼았다. **틀렸다** — 그 숫자는 `:153`에서 발동 순간 1회 계산되는 스냅샷이라, 스윙 중 적이 달려와 겹쳐도 같은 적일 수 있다. 부채꼴은 "다음 타에 누굴 고르나"에만 걸린다.

**② "루트모션이 적을 통과시킨다"**
프레임에서 몸이 겹치는 걸 보고 추정했다. **확인 안 한 추측이었고 원인도 아니다.** 적을 붙게 하는 건 적 AI고, 방향을 돌리는 건 워핑의 회전 계산이다. 콤보 몽타주 이동량은 손대지 않았다.

📌 두 가설 모두 **화면만 보고 세운 것**이고, 엔진 소스를 읽고서야 확정됐다.

---

## 9. 포폴 트러블슈팅 재료 (승환 판단)

기존 영상 `5:30 디버그 검증 방식(근접 판정)` 절 계열에 붙는다.

```
증상    적에게 등을 돌리고 허공을 벤다 (0.4초)
화면    KD.ShowApproach 1 — "Approach 워프  dist 298  (stop 140 / max 700)"
        몸이 겹쳐 있는데 dist 가 298 로 찍히는 화면이 그대로 증거
진단    엔진 소스 3곳으로 확정 (RootMotionModifier.cpp:400 / :74-86 / :127-140)
수정    코드 1글자
서사    "엔진 규칙을 지켰는데, 그 규칙이 이 상황에서 틀렸다"
```

★ **`PROJECT_OVERVIEW §2-5`에 *"FromComponent로 넘기면 RotationType = Facing — 현행 19개 전부 부합"* 이라 적혀 있다.** 규칙을 지킨 게 맞는데 그 규칙이 뒤집힘을 낳았다. **문서화된 규칙 자체를 고쳐야 하는 사례**라 트러블슈팅 소재로 값이 크다.

---

## 커밋

```
코드    KDGameplayAbility_PlayerMelee.cpp   :172 bFollowComponent true → false + 주석
빌드    통과 · PIE 확인 (승환)
```

## 관련

- `2026-09-05-melee-trace-lowfps-research.md` (같은 날, 별개 주제)
- `2026-08-31-camera-lag-burst.md` (반격 워프 잔류 — 같은 함수 2번째)
- 메모리 `reference_melee_autoaim_research` (GoW 스냅 방식)
