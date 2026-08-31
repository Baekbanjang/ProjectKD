# 슬로모 시계 정정 + 반격 TimeScale 배선

**작성** 2026-08-31 / 코드 4파일 · 에셋 3개

08-31 오전 반격 히트 프로필 작업의 후속. 목표는 **반격에 SB식 슬로모를 거는 것**이었으나, 착수 직전 `UKDSlowMotionSubsystem` 에서 시간 단위 혼용이 나왔다. 그걸 먼저 고쳤다.

---

## 1. ★ 슬로모가 요청한 길이의 `1/Scale` 배로 지속되고 있었다

`KDSlowMotionSubsystem.cpp` 가 **서로 다른 두 시계를 섞어 썼다.**

```cpp
// :18   만료 시각 = 실시간 시계
New.ExpireTime = World->GetRealTimeSeconds() + Duration;

// :66   알람 = 게임 시계 타이머
Timers.SetTimer(RecalcTimer, ..., FMath::Max(NextExpire - Now, KINDA_SMALL_NUMBER), false);
```

실시간 간격을 게임 시계 타이머에 그대로 넘겼다. 알람을 맞추는 시점에 슬로모가 이미 켜져 있으니 **그 알람 자신이 느려진다.**

### 엔진 근거

```
World.h:1879       RealTimeSeconds  "IS NOT dilated/clamped"
LevelTick.cpp:1577 GetTimerManager().Tick(DeltaSeconds)   <- dilated 델타
```

### 실제로 걸리던 값

```
GCN_PerfectParry   요청 (0.3 / 0.15 / P10)   실제 지속 = 0.15 ÷ 0.3 = 실시간 0.50초
GCN_PerfectDodge   요청 (0.7 / 1.5  / P0 )   실제 지속 = 1.5  ÷ 0.7 = 실시간 2.14초
```

📌 08-30 dev-log 는 이걸 *"정리(RemoveAll)만 한 프레임 늦다"* 로 적어뒀다. 한 프레임이 아니라 **배율의 역수배**다. `Recalculate` 안의 만료 판정은 정확했지만, **그 `Recalculate` 가 호출되는 시점 자체가 늦었다.**

### 같은 함정이 히트스톱에도 있었다

`KDHitStopComponent.cpp:47` 의 재개 타이머도 게임 시계다. 평시엔 배율 1이라 정상이지만, **슬로모를 얹는 순간 정지 시간이 같은 비율로 늘어난다.**

```
반격 히트스톱 0.15초  ÷  슬로모 0.2배  =  실시간 0.75초 정지
```

오늘 슬로모를 그냥 붙였으면 캐릭터가 0.75초 얼어붙었다.

---

## 2. 수정 — 실시간 길이를 게임 시계 눈금으로 환산

두 파일 모두 같은 계산이다. **실시간 초 × 현재 배율 = 게임 시계 초.**

### `KDSlowMotionSubsystem.cpp:65~69`

```cpp
// 남은 실시간을 현재 배율의 게임 시간으로 환산 — 타이머 = 게임 시계
const float RemainReal = FMath::Max(NextExpire - Now, KINDA_SMALL_NUMBER);
const float SafeScale = FMath::Max(Winner->Scale, KINDA_SMALL_NUMBER);

Timers.SetTimer(RecalcTimer, this, &UKDSlowMotionSubsystem::Recalculate,
    RemainReal * SafeScale, false);
```

`FMath::Max` 두 개가 각각 다른 사고를 막는다.

```
RemainReal   만료가 이미 지났으면 음수 -> 타이머에 음수 입력 방지
SafeScale    Scale 0 이면 곱이 0 -> 알람 지연 0 = 매 프레임 Recalculate
```

### `KDHitStopComponent.cpp:35~36`

```cpp
// 전역 슬로모 중 정지가 길어지는 것 보정
const float DilatedDuration = Duration * FMath::Max(UGameplayStatics::GetGlobalTimeDilation(World), KINDA_SMALL_NUMBER);
```

아래 `Duration` 사용처 3곳(`:38` 잔여시간 비교 · `:40`/`:47` SetTimer)을 `DilatedDuration` 으로 교체.

⚠️ **`:32` 의 `Duration <= 0.f` 조기 반환은 원본 값으로 둔다.** 입력 검사라 배율을 태우면 안 된다.

⚠️ 한계 — 히트스톱이 도는 **도중에** 슬로모가 새로 켜지면 그만큼 어긋난다. 완전히 막으려면 Tick 이 필요한데, 반격은 슬로모와 히트스톱이 같은 프레임에 시작하므로 이 보정으로 충분하다.

---

## 3. GA 에 슬로모 칸 3개 — SB 의 TimeScale 층

값을 어디에 둘지가 갈렸다. **`UKDGameplayAbility_PlayerMelee` 에 넣었다.**

```cpp
// KDGameplayAbility_PlayerMelee.h:32~42
float HitSlowMoScale = 1.f;      // ClampMin 0.05 / ClampMax 1.0
float HitSlowMoDuration = 0.f;   // ClampMin 0.0  / ClampMax 1.0
int32 HitSlowMoPriority = 10;
```

이유 = `AttackerHitStopDuration` · `HitConfirmMagnitude` 와 같은 자리다. 08-31 오전에 DA 승격을 기각한 논리("값이 두 군데 생기면 어느 쪽이 이기나를 매번 기억해야 한다")를 그대로 잇는다.

**기본값이 곧 안전장치다.** `Scale 1.f` / `Duration 0.f` 라 기존 GA 는 전부 요청을 보내지 않는다. "평타엔 슬로모 X" 가 구조로 강제된다.

```cpp
// KDGameplayAbility_PlayerMelee.cpp:53~61   ExecuteGameplayCue 직후
if (HitSlowMoScale < 1.f && HitSlowMoDuration > 0.f)
{
    if (UKDSlowMotionSubsystem* SlowMo = GetWorld()->GetSubsystem<UKDSlowMotionSubsystem>())
    {
        SlowMo->RequestSlowMo(HitSlowMoScale, HitSlowMoDuration, HitSlowMoPriority);
    }
}
```

★ **위치가 `bIgnoreHitStop` 조기 반환보다 위다.** 슬로모와 히트스톱은 별개 레버라, 히트스톱을 끈 창에서도 슬로모는 살아야 한다.

---

## 4. ★ 정정 — SB 히트스톱은 공격자만 멈춘다

세션 중 *"SB 는 나와 적이 같이 멈춘다"* 고 말했으나 **틀렸다.** Show 데이터의 `TimeScale` 키를 히트스톱으로 묶어 읽은 오독이다.

언리얼페스트 2024 발표 슬라이드 원문 (32:36):
```
타격 타이밍에 맞춰 공격자의 애니메이션을 멈추는 기법
정지시간 : 0.07 sec, 약 4~5 프레임(60 프레임 기준)
```

두 층이 별개라는 건 08-31 오전 판독에서 이미 갈라놓은 것이었다.

```
히트스톱     애니메이션 정지 · 공격자만 · 0.07초       코드 레벨. Show 데이터에 없음
TimeScale    전역 시간 배율 · 반격만 · 0.2배 0.15초    Show 데이터에 있음
```

**우리 구조가 그대로 같다** — `UKDHitStopComponent`(공격자 `CustomTimeDilation = 0`) + `UKDSlowMotionSubsystem`(전역 배율). 적을 멈추는 층은 SB 에도 없다. 적 반응은 Bone Shake · Vertex Shake 가 맡는다.

📌 그래서 어제 올린 `AttackerHitStopDuration 0.08 -> 0.15` 를 **0.08 로 되돌렸다.** SB 0.07 과 거의 같던 값을 체감 부족 때문에 2배로 늘렸던 것인데, 묵직함의 출처는 히트스톱이 아니라 TimeScale 이었다.

---

## 5. 에셋 값

```
GA_CounterSlash    HitSlowMoScale 0.2 / HitSlowMoDuration 0.15 / HitSlowMoPriority 10
                   AttackerHitStopDuration 0.15 -> 0.08
GA_CounterThrust   위와 동일
LCS_CounterAttack  FOVOscillation Amplitude -20 -> -50        (SB 는 -70)
```

MCP `inspect_cdo` 전수 검증 완료.

### 겹침 — 문제 없다

`GCN_PerfectParry` 가 `(0.3 / 0.15 / P10)` 을 쓰고 반격이 `(0.2 / 0.15 / P10)` 이라 Priority 가 같다. 동률이면 나중 등록이 이긴다(`Recalculate` 의 `>=`).

시계를 고쳐 패링 슬로모가 0.5초에서 0.15초로 짧아졌고, 반격 판정은 몽타주 f4(약 0.13초) + 입력·발동 지연 뒤라 그때는 이미 끝나 있다. **버그를 고친 것이 겹침까지 함께 해소했다.**

---

## 6. 검증

```
빌드   Project_KDEditor Win64 Development   통과 (승환)
       .h 에 UPROPERTY 3개 추가 = UHT 재생성 필요 -> 라이브 코딩 불가, 에디터 종료 후 정식 빌드
MCP    GA 2개 · LCS 1개 값 전수 확인
PIE    반격 묵직함 개선 확인 (승환)
```

---

## 7. 곁가지 실측 — 브릿지 대기 2건 해소

에디터를 켠 김에 볼트가 판정 못 하던 에셋값 둘을 뽑았다.

### 입력 버퍼 = **0.8 / 2**

```
BP_SBPlayer.InputBuffer   BufferTimeWindow 0.8   MaxBufferSize 2
코드 기본값               0.2                    4
```

`KDInputBufferComponent.h:25` 주석 = *"상한 0.8 = SB 입력 접수창 0.7~0.8"*. 커밋 `5c32389` 로 상한을 올린 뒤 BP 가 그 상한값을 쓰고 있다.

📌 보류 항목 *"입력 버퍼 0.2초가 짧다 -> 0.5 권장"* 은 **닫는다.** 0.2 로 체감한 게 아니라 이미 SB 상단 범위였다.

⚠️ 남는 의문 = **`MaxBufferSize 2`**. 코드 기본 4를 BP 가 2로 낮췄는데 경위 기록이 없다. 빠르게 3번 누르면 하나가 버려진다.

### 락온 = DA 가 필터를 담는 그릇

볼트 노트의 *"TargetFilter 로 흡수됐다"* 와 *"DA_LockOnConfig 실측"* 이 **둘 다 맞았다.** 양자택일이 아니었다.

```
BP_SBPlayer.LockOnComponent.Config -> /Game/SB_Style_GameProject/UI/LockOn/DA_LockOnConfig_Default
KDLockOnConfig.h:24                   FKDTargetFilter TargetFilter    <- DA 안에 필터가 들어 있다
```

```
DA_LockOnConfig_Default.TargetFilter
  Radius 1000        <- 볼트 기록 1700 이 아니다
  HalfAngle 45 / Height 500 / HeightOffset -150
  SortType SmallestAngle
  bDrawDebug True    ⚠️ 08-24 "촬영용 디버그 원복" 이 이건 안 껐다. 승환이 촬영 직전에 끄기로
```

---

## 남은 것

```
C2  히트스톱 무기 IK 스냅    SB 가 겪고 고친 문제. 슬로모로 느려지면 어긋남이 더 보인다
C1  Vertex Shake (WPO)      머티리얼만. Bone Shake 의 짝
B1  GCN_CounterTrail 배선    구현돼 있고 발신자만 없다
    MaxBufferSize 2         왜 2인지 확인 후 4 복귀 판단
```

## 관련

- `docs/dev-logs/2026-08-31-counter-hitconfirm-profile.md` — 오전 작업. HitConfirm 프로필 무음 해소
- `docs/dev-logs/2026-08-31-SB언리얼페스트2024-전수판독.md` — 히트스톱 0.07초·공격자만 출처
- `docs/dev-logs/2026-08-30-presentation-audit.md` — `UKDSlowMotionSubsystem` 신설
