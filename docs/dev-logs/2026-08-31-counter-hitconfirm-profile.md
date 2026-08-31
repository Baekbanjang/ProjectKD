# 반격 히트 프로필 — 무음 해소 + SB식 쉐이크 분리

**작성** 2026-08-31 / **코드 변경 0줄** (에셋만)

---

## 문제 — 반격 슬래시가 소리도 이펙트도 안 났다

`GA_CounterSlash.HitConfirmProfile` 이 `None` 이었다. 쉐이크만 빠진 게 아니라 **전부** 안 났다.

원인은 `GCN_PlayerHitConfirm.OnExecute` 의 구조에 있다.

```
On Execute → 부모 On Execute → Break GameplayCueParameters
   ├ SourceObject → Cast To KDHitConfirmProfile        ← 유일한 관문
   │     ├ CameraShakeClass → Client Start Camera Shake  [Shake 핀]
   │     ├ ImpactVFX        → Spawn System at Location
   │     └ HitSound         → Play Sound at Location
   ├ RawMagnitude → Client Start Camera Shake            [Scale 핀]
   ├ Location     → Spawn / PlaySound
   └ Normal       → Spawn
```

**프로필이 `None` 이면 캐스트가 실패해 흐름 전체가 끊긴다.** 소리·VFX·쉐이크가 한 관문에 묶여 있다.

⚠️ 개별 노드의 null 은 안전하다 — 엔진이 막는다.
`PlayerCameraManager.cpp:1249` `if (ShakeClass && CachedCameraShakeMod)` / `GameplayStatics.cpp:1698` `if (!Sound ...) return;`

---

## SB는 평타에 카메라 쉐이크를 안 건다

Show 데이터 전수 조사 결과(`Art/Show/CH_P_EVE_01/`, 511파일):

```
LightAttack 1~4 · StrongAttack 1~2 · FinishAttack 1~4 · Parry_LightAttack1
   -> SBShowCamShakeKey 없음
쉐이크가 붙는 곳 = 저스트패링 반격 · 저스트회피 반격 · 차지슬래시 · LinkAttack
```

**카메라 반응 자체가 "특별한 순간"의 표식**이다. 216개 CamShakeKey가 511파일에 흩어진 이유가 이거다.

반격 쉐이크는 흔들지 않고 **FOV를 당긴다**:
```
P_Eve_Sword_JustParry_LightAttack1
  OscillationDuration 0.4 / BlendIn 0.1 / BlendOut 0.2
  FOVOscillation  Amplitude -70  Frequency 1.5
  LocOscillation · RotOscillation  없음
```
`Freq 1.5` 에 `Duration 0.4` 면 진동이 아니라 **한 번 훅 들어갔다 나오는 렌즈 펀치**다.

---

## 작업 — DA 구조는 그대로 두었다

SB의 3요소가 우리에겐 이미 다 있고, 위치만 다르다. **`UKDHitConfirmProfile` 에 필드를 추가할 이유가 없었다.**

| SB | 우리 | 어디에 |
|---|---|---|
| CamShake 파라미터 | `LCS_*` | 에셋 안 |
| **ShakeScale** | **`HitConfirmMagnitude`** | **GA** — `CueParams.RawMagnitude` → GC의 `Client Start Camera Shake` **Scale 핀에 배선돼 있다** |
| **TimeScale** | **`AttackerHitStopDuration`** | **GA** |
| Sound · Particle | `HitSound` · `ImpactVFX` | DA |

DA에 넣으면 값이 두 군데 생겨 "어느 쪽이 이기나"를 매번 기억해야 한다. SB도 CamShake / TimeScale 이 별개 트랙이지 하나로 묶여 있지 않다.

### 변경 내역
```
LCS_CounterAttack (신규)     LCS_Hit 복제
                              OscDuration 0.4 / BlendIn 0.1 / BlendOut 0.2
                              FOVOscillation  Amplitude -20 · Frequency 1.5 · InitialOffset EOO_OffsetZero
                              Loc · Rot 전부 비움
DA_HitCounterAttack (신규)    DA_HitLightAttack 복제 + CameraShakeClass = LCS_CounterAttack
                              HitSound SC_Sword_Hit / ImpactVFX NS_Hit_Basic_Once 유지
DA_HitLightAttack             CameraShakeClass  LCS_Hit -> None
GA_CounterSlash               HitConfirmProfile  None -> DA_HitCounterAttack
GA_CounterThrust              HitConfirmProfile  DA_HitLightAttack -> DA_HitCounterAttack
두 GA 공통                     AttackerHitStopDuration 0.08 -> 0.15
                              HitConfirmMagnitude 1.0 -> 2.0
GA_ShotBlast                  그대로 — SB도 총 명중엔 쉐이크 X (발사 반동은 GCN_ShootRecoil 별도)
```

`LCS_PlayerHitConfirm`(회전 3축)은 쓰지 않았다 — SB는 근접에서 회전을 안 흔든다. 카메라 레일에도 안전하다.

---

## ★ 함정 — `InitialOffset` 기본값이 `Random` 이다

```cpp
// LegacyCameraShake.h:34   EOO_OffsetRandom  <- 기본값
// LegacyCameraShake.cpp:54
return (Osc.InitialOffset == EOO_OffsetRandom) ? FMath::FRand() * (2.f * PI) : 0.f;
```

`Freq 40` 짜리 빠른 흔들림은 위상이 랜덤이어도 티가 안 난다. 하지만 **`Freq 1.5` FOV 펀치는 시작 위상이 랜덤이면 어떨 땐 줌인, 어떨 땐 줌아웃**이 된다. `Zero` 로 박아야 한다.

---

## 검증

MCP `inspect_cdo` 로 전수 확인 (⚠️ Claude Code 의 MCP 클라이언트가 죽어 있어 JSON-RPC 를 HTTP 로 직접 태웠다 — `D:/tmp/mcp_probe.py`)

```
LCS_CounterAttack    Duration 0.4 / In 0.1 / Out 0.2
                     FOV (Amplitude=-20, Frequency=1.5, InitialOffset=EOO_OffsetZero)
                     Loc (X=(),Y=(),Z=())  Rot (Pitch=(),Yaw=(),Roll=())
GA_Counter* 둘 다     HitConfirmProfile = DA_HitCounterAttack
                     AttackerHitStopDuration 0.15 / HitConfirmMagnitude 2
```

### PIE (승환)
```
1 평타 쉐이크 사라짐     원래 값이 작아서 큰 차이 모르겠음
2 반격 소리·이펙트 나옴   ✅ 본목적 달성
3 FOV 당김               체감 안 됨   -> -20 이 약하다
4 히트스톱 0.15          체감 안 됨   -> 아래 참조
```

---

## ★★ 다음 — 레버를 잘못 잡았다

승환 관찰 = "SB 게임상 히트스톱 느낌이 많이 없다". **맞다.**

```
히트스톱   0.07초 애니 정지       평타·반격 공통. 원래 은은하다
TimeScale  0.2배 0.15초 슬로모    반격에만 붙는다  <- SB "묵직함" 의 정체
```

SB 저스트패링 반격 실측 (Show 데이터)
```
JustParry_LightAttack1   TimeScale 0.2배 0.15s (t=0.08) -> 0.3배 0.2s (t=0.4)   2단
JustParry_StrongAttack1  TimeScale 0.1배 0.15s                     가장 강함
★ 히트스톱이 판정(t=0.150)보다 0.07초 먼저 온다 = 베기 직전부터 느려진다
```

우리는 `UKDSlowMotionSubsystem` 이 이미 있다(2026-08-30 신설). `RequestSlowMo(Scale, Duration, Priority)` — BP 노드 하나.

```
후보  반격 명중 시  RequestSlowMo(0.2, 0.15, Priority 10)
FOV   -20 -> -35 -> -50  (SB 는 -70) 같이 올린다
```

⚠️ `GCN_PerfectParry` 가 이미 `(0.3 / 0.15 / P10)` 을 쓴다. **패링 성공과 반격 명중이 겹치면 두 번 걸린다** — Priority 가 같으면 나중 것이 이긴다(`Recalculate` 가 `>=`). 겹침 확인이 먼저다.

---

## 관련

- `docs/dev-logs/2026-08-31-SB언리얼페스트2024-전수판독.md` — 히트스톱 0.07초 출처
- `docs/dev-logs/2026-08-30-presentation-audit.md` — `UKDSlowMotionSubsystem` 신설
- 메모리 `reference_sb_camshake_action_timeline` · `project_counter_hitconfirm_2026-08-31`
