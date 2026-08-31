# 패링 연출 SB식 정렬 — 접촉점 이펙트 + 중복 발신 제거 + 막힘 쉐이크

**작성** 2026-08-31 / 코드 1줄 · 에셋 6개

같은 날 세 번째 작업. 반격 연출(슬로모·랙 버스트)을 끝내고 **패링 쪽 연출 구멍**으로 넘어갔다.

---

## 1. SB 실측 — 카메라가 흔들리는 건 한 곳뿐이다

`Art/Show/Result/` 가 피격 결과별 연출 데이터다. 전수 조회 결과:

```
Result_Parryed      내 공격이 패링당함   CamShake ✅  Particle ✅  Sound
Result_Parry        내가 패링 성공       Anim · Sound · 패드 진동(SBShowVibrationKey)
Result_Guard        적이 가드로 막음     Anim · Sound 만
Result_Guard_Break  가드 붕괴           Anim · Sound 만
```

**"내 공격이 튕겨나간 순간" 하나에만 쉐이크가 붙는다.** 08-31 오전에 평타 쉐이크를 뺀 것과 같은 원리다 — 카메라 반응은 흔한 일에 쓰면 값이 떨어진다.

📌 **이 실측으로 B4(가드 붕괴 / 막기 임팩트 GC)의 규모가 절반으로 줄었다.** 쉐이크를 만들 필요가 없다.

### 이펙트는 소켓이 아니라 접촉점

```
Result_Parry_PC   NS_ParryBlock_01   Target = HitLocation
Result_Parryed    NS_ParryBlock_01   Target = HitLocation · Axis = HitDirection
```

무기 소켓에 붙이는 게 아니라 **부딪힌 지점에서 튄다.** 방향도 타격 방향으로 돈다.

📌 저스트패링은 4겹이다 — `NS_A_TachySkillParrySpark` · `NS_JustParry_02` · `NS_A_TachySkillParryFlare` · `NS_A_ShapeBlur_01`.

---

## 2. ★ `GCN_PerfectParry` 가 길동 창 소켓을 물고 있었다

```
Content/SB_Style_GameProject 전체 스캔 → Spear_Tip 1건 = GCN_PerfectParry.uasset
Content/Blueprints 전체 스캔          → 0건

Spawn System Attached
   SystemTemplate    NS_Slash_PowerUp_Burst_01
   AttachPointName   "Spear_Tip"        ← 길동 창 소켓. 현행 SKM_Manny_GunSword 에 없다
   AttachToComponent ← Get Components by Tag → Get(사본)
```

메모리 `reference_silent_gas_defects` 의 2026-07-28 사건(공격 GA 5개가 `Spear_Tip` 을 물고 있어 판정이 죽어 있었음)과 같은 잔재다.

### 왜 소켓 방식이었나 — 위치를 받을 통로가 없었다

```cpp
// KDCombatAttributeSet.cpp:135~141 (종전)
EventData.EventTag = Event_Combat_PerfectParryTriggered;
EventData.Instigator = ...;  EventData.Target = ...;  EventData.EventMagnitude = HitAngle;
// ⚠️ ContextHandle 이 없다 → HitResult 없음 → ImpactPoint 없음
```

바로 아래 적 방어형 패링(`:151`)은 **이미 `ContextHandle` 을 넘기고 있었다.** 나중에 만든 쪽이 제대로 된 것이고, 퍼펙트 패링만 옛 방식으로 남아 있었다.

그래서 `GCN_PerfectParry` 는 위치를 스스로 찾아야 했고 → 무기 컴포넌트를 태그로 뒤져 소켓에 붙이는 우회를 했다.

---

## 3. ★★ 같은 큐를 두 곳에서 쏘고 있었다

```
GA_Parry                  Execute GameplayCue On Owner   GameplayCue.Combat.PerfectParry.SlowMo   Context 비어 있음
GA_PerfectParryReaction   Execute GameplayCue On Owner   GameplayCue.Combat.PerfectParry.SlowMo   Context 비어 있음
```

둘 다 같은 이벤트(`Event.Combat.PerfectParryTriggered`)에 반응한다 — `GA_Parry` 는 `Wait Gameplay Event`, `GA_PerfectParryReaction` 은 트리거 태그. **패링 한 번에 큐가 두 번 떴다.**

`GCN_PerfectParry` 는 Actor 기반이라 큐마다 액터가 스폰된다. **액터 2개 · MPC Timeline 2개 · 슬로모 요청 2개.**

⚠️ 눈에 안 띈 이유 = `UKDSlowMotionSubsystem` 이 요청을 목록으로 관리해 배율이 같았다. 08-30 에 이 서브시스템을 만든 게 증상을 덮고 있었다.

**`GA_Parry` 쪽에 남겼다.** 이유:
```
Break Gameplay Event Data 가 이미 그 그래프에 있다   ContextHandle 을 바로 꺼낸다
좌/우 리액션 몽타주도 거기서 고른다                  연출이 한 자리에 모인다
```

### 곁가지 — 패링 성공 리액션 애니는 있었다

조사 중 `GA_PerfectParryReaction` 만 보고 *"몽타주가 없다"* 고 판단했다가 뒤집었다. **`GA_Parry` 가 처리하고 있었다.**

```
Wait Gameplay Event (Event.Combat.PerfectParryTriggered · OnlyMatchExact true)
  → Break Event Data → float > float (EventMagnitude = HitAngle)
  → Branch
       True  → PlayMontageAndWait  AM_SB_Parry_L
       False → PlayMontageAndWait  AM_SB_Parry_R
```

**맞은 각도로 좌/우 리액션을 고른다.** `HitAngle` 은 `KDCombatAttributeSet.cpp:139` 가 `EventMagnitude` 로 싣는 값이다.

📌 그래서 큐에 조건 분기가 필요 없다 — **이벤트 자체가 조건**이다. `State.Combat.PerfectParryReady` 가 있을 때만 발행되므로 일반 블록은 이 줄기를 안 탄다.

---

## 4. 변경 내역

### 코드 1줄

```cpp
// KDCombatAttributeSet.cpp:140~141
// 접촉점 운반 — GCN 이펙트 위치 = HitResult 의 ImpactPoint
EventData.ContextHandle = Data.EffectSpec.GetContext();
```

### BP

```
GA_Parry                  Break Event Data · Context Handle → Execute GameplayCue · Context
GA_PerfectParryReaction   Execute GameplayCue On Owner 삭제 (중복)
                          Send Gameplay Event · ApplyGameplayEffectToOwner 는 유지 (CounterReady 부여)
GCN_PerfectParry          RunSlowMo 에 CueParams 입력 추가
                          OnExecute · Parameters → RunSlowMo · CueParams
                          Spawn System Attached(Spear_Tip) → Spawn System at Location
                             Location = Get Hit Result → Impact Point
                          Get Components by Tag · Get(사본) 삭제
GCN_ParryClash            BurstCameraShake = LCS_ParryClash + Play In World 켬
LCS_ParryClash            LCS_PlayerHitConfirm(고아) 개명 + SB 값 이식
```

### `LCS_ParryClash` = SB `CS_PC_DefaltParry_01` 이식

```
OscillationDuration 0.35 / BlendIn 0.0 / BlendOut 0.15
LocOscillation  X · Y · Z 전부 Amplitude 3.0 · Frequency 40.0
RotOscillation · FOVOscillation  전부 0
```
`BlendIn 0.0` = 즉발. 튕기는 순간 바로 흔들린다. 고아 에셋을 재활용해 새 에셋을 안 늘렸다.

### ⚠️ `Play In World` 를 켜야 뜬다

```cpp
// GameplayCueNotifyTypes.cpp:652   bPlayInWorld = false 경로
APlayerController* TargetPC = SpawnContext.FindLocalPlayerController(
    EGameplayCueNotify_LocallyControlledSource::TargetActor);   // ← TargetActor 하드코딩
```

`ParryClash` 는 **적** ASC 가 쏘므로 `TargetActor` = 적이고, 적에겐 `PlayerController` 가 없다. `TargetPC` 가 null이라 쉐이크가 안 뜬다.

`bPlayInWorld = true` 로 켜면 월드 위치 기준으로 근처 플레이어를 흔들고, **거리에 따라 세기가 준다**(`CalculateFalloffIntensity`). 이쪽이 더 낫다 — 멀리서 다른 적이 막아도 화면이 안 흔들린다.

⚠️ `WorldInnerRadius` / `OuterRadius` 를 **둘 다 0으로 두면 안 된다.** falloff 가 0을 반환해 걸러진다. 넣은 값(300 / 1500)은 SB 근거가 없는 임의값이라 PIE 튜닝 대상.

---

## 5. ★ 조사 방법 교훈 — BP 는 `functions` 를 먼저 봐야 한다

`GCN_PerfectParry` 를 *"OnExecute 훅이 없다"* 고 두 번 오판할 뻔했다.

```
OnExecute · OnActive 는 BlueprintNativeEvent
   -> EventGraph 가 아니라 함수 그래프로 생긴다
   -> get_graph_details 기본 조회(EventGraph)에는 안 나온다

get_blueprint 로 보면 functions: [UserConstructionScript, OnExecute] 가 뜬다
```

**08-30 의 `GCN_CounterTrail` 진단도 이 이유로 틀렸을 수 있다.** 해당 dev-log에 정정을 달았다. 이미 삭제해서 확인은 불가.

---

## 6. 검증

```
빌드   통과 (승환)
PIE    퍼펙트 패링 시 나이아가라가 접촉점에 정상 출력 (승환)
남은 것 = 이펙트 색 · 크기 튜닝 (나이아가라 값)
```

---

## 남은 것

```
B4  가드 붕괴 / 막기 임팩트 GC   ★쉐이크 없이 Anim + Sound (SB 실측으로 축소)
C1  Vertex Shake (WPO)           ⚠️ "코드 0줄" 은 낙관 — 피격 위치를 머티리얼에 넘길 통로가 없다
C2  히트스톱 무기 IK 스냅
락온 피치 대역                    SB 는 락온 전용 커브로 17도만 (메모리 reference_sb_lockon_camera)
패링 이펙트 색 · 크기             나이아가라 튜닝
GCN_ParryClash 반경               300 / 1500 은 임의값
```

## 관련

- `docs/dev-logs/2026-08-31-camera-lag-burst.md` — 같은 날 랙 버스트 + 워프 잔류
- `docs/dev-logs/2026-08-31-slowmo-timescale-fix.md` — 슬로모 시계 정정
- 메모리 `reference_sb_lockon_camera` (신규) · `reference_silent_gas_defects`
