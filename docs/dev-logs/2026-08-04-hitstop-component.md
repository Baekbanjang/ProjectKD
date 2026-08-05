# 2026-08-04 — 히트스톱을 액터 소유 컴포넌트로 + 노티 재발화 버그 해결

## 왜 했나

다단 타격에서 히트스톱이 덜컥거렸다. 조사해보니 증상은 표면이었고 **한 번 휘두를 때 데미지가 여러 번 들어가고 있었다.**

```
BEGIN 20.622 → HIT 20.661     데미지 1회
BEGIN 21.092 → HIT 21.120     데미지 2회   ← 한 번 휘둘렀는데
BEGIN 21.540 → HIT 21.559     데미지 3회
```

기존 `ApplyHitStop`은 `GA_PlayerMeleeAttackBase`에 인라인이었고 `FTimerHandle`이 지역 변수였다. 주석에 이미 흔적이 있었다 — "SetPlayRate(0) 대신 Pause — 겹친 두 번째 타격이 rate=0을 물어 영구 정지하는 것 방지". **한 번 데인 자리였고, 이번이 세 번째다.** CLAUDE.md §1-5(같은 함수 두 번째 버그 = 설계 의심)에 해당한다.

## 근본 원인 — 엔진은 "이동했을 때만" 노티 목록을 채운다

```cpp
// AnimMontage.cpp:2597
const bool bHaveMoved = (SubStepResult == EMontageSubStepResult::Moved);
if (bHaveMoved)
{
    HandleEvents(PreviousSubStepPosition, Position, BranchingPointMarker);   // 노티 큐를 채우는 유일한 경로
}

// AnimInstance.cpp:1628 — 이번 프레임 목록에 없는 노티는 전부
AnimNotifyEvent.NotifyStateClass->NotifyEnd(...);
```

히트스톱이 몽타주를 제자리에 세우면 노티 목록이 비고, **엔진은 그걸 "노티 구간이 끝났다"로 읽는다.** 재개하면 다시 목록에 들어오면서 `NotifyBegin`이 재호출된다.

```
정지 → 목록 빔 → NotifyEnd → 판정창 종료
    → 재개 → 목록 복귀 → NotifyBegin → 판정창 재시작
    → AlreadyHitActors.Reset() → 같은 적 재히트 → 데미지·히트스톱 중복
```

### 예외는 `DeltaTime == 0` 하나뿐

```cpp
// AnimMontage.cpp:2511
// Gather active anim state notifies if DeltaTime == 0 (happens when TimeDilation is 0.f),
// so these are not prematurely ended
if (DeltaTime == 0.f)
{
    HandleEvents(Position, Position, nullptr);
}
```

엔진이 이 상황을 위해 만들어둔 유일한 우회로다. 주석에 "TimeDilation이 0일 때"라고 명시돼 있다.

| 정지 방식 | `bPlaying` | `DeltaTime` | 이동 | 결과 |
|---|---|---|---|---|
| `Montage_Pause` | **false** | ≠0 | — | 관문(2484)에서 막힘 → **NotifyEnd** |
| `Montage_SetPlayRate(0)` | true | **≠0** | **없음** | 안전장치 못 탐 + `bHaveMoved=false` → **NotifyEnd** |
| **`CustomTimeDilation = 0`** | true | **0** | 없음 | **안전장치 작동 → 노티 유지** ✅ |

`SetPlayRate(0)`이 최악이다. "시계는 도는데 몽타주는 안 나아가는" 상태라 두 경로 모두에서 빠진다.

> **0 외의 값은 작동하지 않는다.** `0.02`처럼 아주 느리게 흘리면 `DeltaTime`이 0이 아니라 안전장치를 못 탄다. 코드 주석에 이 근거를 남겨뒀다.

## 구현

### 신규 — `Combat/HitStopComponent.h/.cpp`

```
UHitStopComponent : UActorComponent   (플레이어에만 부착)
  RequestHitStop(float Duration)   자기 액터 시계를 0으로, 타이머로 복원
  IsHitStopActive()
```

- 정지 = `Owner->CustomTimeDilation = 0.f` / 복원 = 저장해둔 값
- 재진입 시 **저장을 건너뛰고 타이머만 연장** → `SavedTimeDilation`에 0이 들어가는 경로가 없다
- `EndPlay`에서 타이머 정리 + 복원 → 액터가 멈춘 채 남는 사고 방지
- 타이머는 월드 타이머라 액터 시계가 0이어도 정상 작동

### 의존성 — GA는 컴포넌트를 직접 참조하지 않는다

```
GA_PlayerMeleeAttackBase
   │ SendGameplayEventToActor(자기 아바타, Event.Combat.HitStop, Duration)
   ▼
UHitStopComponent  (ASC의 GenericGameplayEventCallbacks 구독)
```

`ExecutionComponent`가 쓰는 패턴과 동일하다. CLAUDE.md §1-3을 이번에 "GC **또는 GameplayEvent** 경유"로 개정했다 — GC는 BP 큐라 지속시간 전달이 불편하다.

### 판정창별 스위치 — `ANS_MeleeTrace::bIgnoreHitStop`

기본 `false`(= 건다). 다단·광역 판정창에 체크하면 그 창만 히트스톱을 생략한다. SB가 실제로 쓴 방식이다.

`GA_MeleeTraceBase`에 `ActiveWindow`(현재 열린 판정창)를 보관하고 `protected` 접근자로 자식이 읽는다. 창 시작 시 담고 End·Cleanup에서 비운다.

### 피격자 히트스톱 폐기

`VictimHitStopDuration`(0.12) 제거. **SB는 공격자만 멈춘다**(발표 §4-2 "타격 타이밍에 공격자 애니메이션을 잠시 멈추는"). 피격자 반응은 이미 `UHitFeedbackComponent`(Bone Shake)가 적에 붙어 담당한다.

## SB 실측 근거

덤프 132,958개 JSON 전수. 히트스톱 관련 식별자는 **9종뿐이고 시간·강도 수치 필드가 0개**다.

| 표면 | 정체 |
|---|---|
| `bIgnoreHitStop` (1,661 파일) | 연출 조각 단위 옵트아웃 |
| `ActorState_DisableHitStop` | 액터 상태 — **상태의 주인이 액터** |
| `BlockHitStop` / `_Step` / `_Skill` | 면역 버프 3종 (수명만 다름) |

**이브 다단 공격의 판정 위치별 히트스톱** (`NextStepAlias` 체인 검증, 이름 기반과 교차 확인)

| 위치 | 켠 비율 |
|---|---|
| 첫 타 | 25~38% |
| 중간 타 | **11~14%** |
| **마지막 타** | **91~92%** |

**적 공격은 99% 뺀다**(2,054/2,081). 판정 2개 이상은 예외 0건. 히트스톱은 사실상 플레이어 전용 기능이다.

> 한 줄로: **히트스톱은 "때렸다"가 아니라 "이 한 방이 끝이다"를 알리는 신호다.**

지속시간은 SB 실행파일 안에 있어 확인 불가. 우리가 정해야 한다.

## 값

| 항목 | 값 |
|---|---|
| `AttackerHitStopDuration` | 0.08 (체감상 더 줄일 여지 있음 — 0.04~0.05 시험 예정) |
| `bIgnoreHitStop` 기본값 | `false` |
| `CustomTimeDilation` 정지값 | **0.f 고정** (다른 값은 작동 안 함) |
| `bOncePerActor` | GA 5개 전부 `true` (실측 확인) |

## 검증

**PIE 통과 3항목**

| 항목 | 결과 |
|---|---|
| 한 번 휘두르기 = `BEGIN` 1줄 | ✅ (이전 2줄) |
| **부채꼴 트레이스가 끊김 없이 이어짐** | ✅ 판정 공백 소멸 |
| 궤적 가운데 있던 적도 정상 타격 | ✅ |

**중간에 잡은 것들**

- `bOncePerActor`는 전부 켜져 있었다 — 원인이 아니었다
- 몽타주당 `Melee Trace Window`는 1개뿐 — 우리에겐 SB식 다단히트가 아직 없다
- `DefaultEngine.ini`에 방향이 뒤집힌 `PropertyRedirects`가 들어가 있어 제거 (올바른 이름 → 오타로 매핑)

## 원인 추적에서 세 번 틀렸다

| 시점 | 내가 단정한 것 | 실제 |
|---|---|---|
| 1 | `Pause`가 `bPlaying`을 꺼서 → `SetPlayRate(0)`이면 해결 | 로그로 반증. 여전히 재발화 |
| 2 | rate=0은 `A→A` 구간이라 노티를 못 잡음 | 엔진 조건상 `Prev==Cur`이어도 담긴다 |
| 3 | `bHaveMoved` 게이트 + `DeltaTime==0` 안전장치 | **확정** |

`if (bPlaying)` 관문까지만 읽고 그 아래 `bHaveMoved` 게이트를 안 읽은 게 원인이다. **관문 하나를 찾았다고 원인이라 단정하면 안 된다** — 메모리 `feedback_verify_until_certain`에 박아뒀다.

## 남은 것

- **`AttackerHitStopDuration` 튜닝** — 0.08도 길게 느껴짐. 0.04~0.05 시험
- **광역 공격 판정창에 `히트스톱 끄기` 체크** — 적 여럿을 훑는 공격은 안 거는 게 SB 방식
- **적 트레이스 메쉬 오류** — `[KD] Trace mesh not found (source=0) on BP_Bandit_Parry_C_1`. 밴딧 공격 판정이 죽어 있다. 별개 이슈
- ⚠️ **진단 로그 4개를 주석 처리한 채 커밋했다** — `GA_MeleeTraceBase.cpp`의 소켓·메쉬 없음 경고 3개 + `Montage Completed`. 시끄러워서 껐다. **에셋 설정이 틀리면 판정이 조용히 죽고 단서가 없다.** 위 밴딧 문제를 팔 때 되살릴 것
- **콤보 1타 반복** — `montage`가 계속 `AM_SB_Combo_01_01`. `DA_ComboTree` 미설정 의심
- 무기 위치 스냅(SB §4-2) / 플레이어 Bone Shake — 폴리싱

## 관련

- CLAUDE.md §1-3(GameplayEvent 경유 허용) · §1-5(같은 함수 두 번째 버그) — 이번 작업 중 개정
- `docs/dev-logs/2026-08-03-camera-rail-look-rotation.md` — 같은 종류의 "뿌리 하나, 증상 셋"
