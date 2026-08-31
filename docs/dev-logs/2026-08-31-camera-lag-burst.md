# 카메라 랙 버스트 + 반격 워프 잔류 수정

**작성** 2026-08-31 / 코드 4파일 · 에셋 4개

같은 날 슬로모 작업(`2026-08-31-slowmo-timescale-fix.md`)의 후속. 핸드오프 B1 *"`GCN_CounterTrail` 배선 — 구현돼 있고 발신자만 없다"* 를 처리하려다 **연출 하나가 아니라 결함 셋**이 나왔다.

---

## 1. SB 실측 — 카메라 랙 제어는 실재하는 기법이었다

`SBShowControlCamLagSpeedKey` **275개**. 덤프에 자체 Show 키 타입으로 존재한다.

```
쓰이는 곳   Evade · JustEvade 반격 · JustParry 반격 · DashAttack · ChargeSlash · ScrewAttack
            = 전부 빠르게 이동하는 동작

필드
  bPlayerCharacterOnly           true
  TargetSpeed                    0.5      랙 속도를 낮춘다 = 카메라가 뒤처진다
  TargetMaxDistance              100.0    최대 뒤처짐 거리
  TargetLockOnCameraBlendScale   0.1
  BlendSpeed                     0.1      값 전환 속도
  Priority                       1.0
  StartTime 0.066 / Duration     0.5
```

**하는 일 = 카메라 랙 속도를 일시적으로 낮춰 카메라를 뒤처지게 만든다.** 캐릭터가 확 이동할 때 카메라가 한 박자 늦게 따라오면 그 이동이 속도로 읽힌다.

### 값 중 셋만 그대로 쓸 수 있다

```
TargetSpeed 0.5 · TargetMaxDistance 100 · Duration 0.5   ✅ 대응 필드가 정확히 있다
BlendSpeed 0.1                                           ❌ SB 보간 공식을 모른다
                                                            UE FInterpTo 에 0.1 = 사실상 안 움직임
                                                            우리 다른 블렌드 속도는 전부 8.f 대
TargetLockOnCameraBlendScale                             ❌ 대응 개념 없음
Priority                                                 ❌ 발신자 1개라 지금은 무의미
```

### ★ 약공과 강공을 SB가 갈라놨다

```
                        RuleMove   CamLagSpeed   CamShake   TimeScale
JustParry_LightAttack1     10           5            5          10
JustParry_StrongAttack1     5           0           10           5
```
세 무기 세트(Fusion · Sword · Tachy) 전부 같은 패턴이다.

```
약공 반격   이동 많다  →  카메라 랙 있다  →  쉐이크 약하게
강공 반격   이동 적다  →  카메라 랙 없다  →  쉐이크 2배
```

**우리 구조가 이미 같았다** — `_L`(약공)만 MotionWarping 노티가 있고 `_R`(강공)엔 없다. 조사 중 이걸 *"워프가 죽어 있다"* 고 결함으로 오판했다가 SB 실측으로 뒤집었다. **결함이 아니라 그 동작의 성격이다.**

---

## 2. 결함 ① `GCN_CounterTrail` 은 껍데기였다

08-30 조사는 *"발신자 C++ 0건 / 몽타주 119개 스캔 0건"* 이라 했는데 **둘 다 틀렸다.**

```
발신   AM_..._L 의 PlayerCue 노티에 GameplayCue.Camera.DashTrail 이 이미 박혀 있었다
       몽타주 스캔이 노티 안의 FGameplayTag 값까지는 안 봤다

수신   GCN_CounterTrail 에 OnExecute / OnActive 훅이 하나도 없다
       로직이 DoDashTrail 이라는 커스텀 이벤트에 매달려 있고 부르는 곳이 없다
```

**안 나온 진짜 이유는 수신 쪽이다.** 태그를 쏴도 받는 데가 없었다.

그 밖에 두 가지가 더 있었다.
```
Cast To KDPlayerCharacter    §1-3 구체 Pawn 캐스팅 금지 위반
Delay → Set CameraLagSpeed 20.0    복귀값 하드코딩. 실제 평상시 값은 19
```

고칠 게 남은 것보다 많아 **버리고 새로 만들었다.**

---

## 3. 소유권을 스프링암으로 — `RequestLagBurst`

복귀값 하드코딩은 08-30 슬로모에서 고친 것과 **같은 뿌리**다.

```
슬로모 종전    Set 0.3 → 0.15초 → Set 1.0        아래 깔린 요청을 밀어버린다
랙 종전        Set 3.0 → 0.5초 → Set 20.0        실제 19 와 1 어긋난다
```

원본값을 아는 건 그 값의 소유자뿐이다. `UKDSpringArmComponent` 는 이미 Tick을 돌며 레일·조준·상승을 관리하니 랙도 여기 소속이다.

```cpp
// KDSpringArmComponent.h
void RequestLagBurst(float TargetLagSpeed, float TargetMaxDistance, float Duration, float BlendSpeed = 4.f);

// BeginPlay 에서 원본 확보 — 에디터 값이 바뀌어도 따라간다
DefaultLagSpeed = CameraLagSpeed;
DefaultLagMaxDistance = CameraLagMaxDistance;
```

역할 분담이 명확하다.
```
RequestLagBurst   목표를 적어두고 스위치를 켠다     한 번 · 즉시. 값은 안 건드린다
UpdateLagBurst    그 목표 쪽으로 조금씩 민다        매 프레임 · Tick
```

**복구를 요청으로 만들지 않았다.** `LagBurstRemaining` 이 0을 지나면 목표가 스스로 `DefaultLagSpeed` 로 갈아탄다. GC는 돌아갈 값을 몰라도 된다.

```cpp
const float GoalSpeed = bHolding ? LagBurstSpeed : DefaultLagSpeed;   // 이 한 줄이 갈림길
```

### 시계 판단 — 여기는 게임시간이 맞다

같은 날 슬로모에서는 실시간이 맞았는데 여기는 반대다.

```
슬로모      월드가 얼마나 느려지나를 밖에서 잰다        실시간
랙 버스트   카메라가 월드 안에서 움직이는 것            게임시간
            엔진 카메라 랙 자체가 DeltaTime 보간이라
            슬로모 중엔 랙도 같이 느려져야 박자가 맞는다
```

`UpdateLagBurst` 가 `DeltaTime` 을 그대로 쓰는 이유다.

### `FInterpTo` 꼬리 자르기

`FInterpTo` 는 남은 거리의 비율씩 좁혀 **목표에 정확히 도달하지 않는다.** 마무리 블록이 없으면 `18.997` 에 눌러앉고 `bLagBurstActive` 가 영원히 true다.

```cpp
if (!bHolding && FMath::IsNearlyEqual(CameraLagSpeed, DefaultLagSpeed, 0.05f))
{
	CameraLagSpeed = DefaultLagSpeed;
	CameraLagMaxDistance = DefaultLagMaxDistance;
	bLagBurstActive = false;
}
```
조건이 `!bHolding` 인 이유 — 요청값이 원본과 비슷하면 유지 구간인데도 꺼진다.

---

## 4. ★★ 결함 ② 워프 타겟이 지워지지 않고 잔류했다

PIE에서 승환이 잡은 증상:

> *원거리 적의 발사체를 패링하고, 다른 곳에서 패링 성공하면 **처음 패링했던 곳으로 강제 워프**된다.*

`AddOrUpdateWarpTargetFromLocationAndRotation` 은 등록만 한다. **한 번 등록된 `CounterTarget` 은 영원히 남는다.** 그런데 `OnActivated` 에 조기 return이 넷 있다.

```cpp
if (!Target) return;                 // 타겟 없음   → 낡은 타겟 그대로
if (Dist > MaxDashRange) return;     // 800 초과    → 낡은 타겟 그대로
if (Dir.IsNearlyZero()) return;      //             → 낡은 타겟 그대로
if (!ensureMsgf(Warp, ...)) return;  //             → 낡은 타겟 그대로
```

`Dist > MaxDashRange` 주석이 *"너무 멀면 대시 생략, 찌르기만"* 인데, **실제로는 대시가 생략되는 게 아니라 옛 좌표로 대시했다.** 월드 좌표로 저장되므로 내가 어디로 옮겨가든 그 지점으로 빨려간다.

원거리 적 시나리오가 정확히 이 경로다 — 발사체를 패링하면 그 적은 멀어서 `:40` 에서 return 한다.

### 수정 — 발동 시작에 먼저 지운다

```cpp
UMotionWarpingComponent* Warp = PC->FindComponentByClass<UMotionWarpingComponent>();
if (!ensureMsgf(Warp, ...)) return;

Warp->RemoveWarpTarget(WarpTargetName);   // 조건 검사보다 위
```

타겟이 없으면 엔진이 알아서 워프를 포기한다.
```cpp
// RootMotionModifier.cpp:319   UE 5.6
if (WarpTargetPtr == nullptr)
{
	SetState(ERootMotionModifierState::Disabled);   // 이 창은 죽고 원본 루트모션대로
	return;
}
```
**주석에 적힌 "대시 생략, 찌르기만" 이 그제서야 실제로 그렇게 동작한다.**

### 규칙이 하나 나온다 — 워프 타겟은 발동 시작에 지운다

`CLAUDE.md §2-5` 의 *"InstancedPerActor GA 멤버 잔류 — 매 활성화 시작부에서 명시 리셋"* 과 같은 성질이다. 워프 타겟도 컴포넌트에 남는 상태다.

전수 확인 결과 **패턴은 이미 확립돼 있었고 둘만 빠져 있었다.**
```
PlayerMelee.cpp:135        RemoveWarpTarget(ApproachWarpName)    이미 있었음
EnemyRushAttack.cpp:94     RemoveWarpTarget(WarpTargetName)       이미 있었음
CounterThrust.cpp:36       ← 오늘 추가
PlayerExecution.cpp:66     ← 오늘 추가 (bHasMeeting 밖으로 빼고 IsValid(Player) 추가)
```

---

## 5. 배선 전체

```
약공 반격(GA_CounterThrust) 발동
  → AM_..._L 재생
  → t=0.000  AN_PlayerCue   CueTag = GameplayCue.Camera.LagBurst
       ASC->ExecuteGameplayCue(...)                    KDAnimNotify_PlayerCue.cpp:22
  → GCN_CameraLagBurst.OnExecute(MyTarget = 플레이어)
  → Get Component By Class(KDSpringArmComponent)       ★ Cast 를 쓰지 않는다
  → RequestLagBurst(0.5, 100, 0.5, 4)
  → 다음 프레임부터 UpdateLagBurst 가 값을 민다
```

같은 t=0.000 에 MotionWarping(dur 0.5)이 캐릭터를 적 앞으로 끌어당긴다. 카메라가 그걸 늦게 따라오는 게 이 연출이다.

📌 **`Static` 을 고른 이유** — `ExecuteGameplayCue` ↔ `GameplayCueNotify_Static` ↔ `OnExecute` 셋이 짝이다. `Actor` 는 큐마다 액터를 스폰하는데 우리는 함수 호출 한 번이 전부다.
```
ExecuteGameplayCue   한 번 터지고 끝    →  OnExecute
AddGameplayCue       켜진 상태로 남음   →  OnActive / OnRemove 짝
```
`OnActive` 방식이면 GC가 **언제 끌지까지 책임**져야 한다. 그게 낡은 BP의 `Delay → Set 20.0` 이었다.

---

## 6. 에셋 변경

```
GCN_CameraLagBurst (신규)    GameplayCueNotify_Static
                             Tag = GameplayCue.Camera.LagBurst
                             OnExecute → GetComponentByClass → RequestLagBurst(0.5/100/0.5/4)
GCN_CounterTrail             삭제
AM_..._L  PlayerCue 태그      DashTrail -> LagBurst
AM_..._R  PlayerCue 트랙      삭제 (빈 태그라 아무 일도 안 하던 잔재)
GA_CounterSlash              HitSlowMoScale 0.2 -> 0.1     SB 배분 = 강공이 가장 강하다
```

### ⚠️ `DashTrail` 태그를 지우며 남긴 것

태그 2줄 삭제 전 Content 전수 스캔 결과 **3개가 물고 있었다.**
```
GCN_CounterTrail.uasset                                       삭제함
Blueprints/Player/Animation/Montage/Spear/AM_Run_Attack_01_Spear.uasset   ⚠️ 손대지 않음
TrickalFanGame/Animation/Combat/RunAttack/AM_Run_Attack_01_Spear.uasset   ⚠️ 손대지 않음
```
뒤의 둘은 길동 창 몽타주 = 08-26 참조 감사에서 **안전 삭제 가능**으로 분류된 폴더다. 태그가 비어도 영향이 없고, 지금 재저장하면 폐기 예정 에셋을 커밋에 끌어들인다.

📌 태그는 **문자열 참조라 지워도 에러가 안 난다**(메모리 `reference_gameplay_tag_asset_string_reference`). 그래서 지우기 전에 목록을 확보했다.

---

## 7. 검증

```
빌드   통과 (승환) — 태그 추가 시 1회 + CounterThrust/Execution 수정 후 1회
PIE    ① 약공 반격에서 카메라가 뒤처졌다 따라온다        ✓
       ② 연타 5~10회 — 랙이 눌러앉거나 누적되지 않는다   ✓ ★복구 검증
       ③ 강공 반격 0.1배 슬로모가 더 묵직하다             ✓
       ④ 슬로모(0.15) + 랙(0.5) 겹침 — 늘어짐 없음        ✓
       ⑤ 원거리 적 패링 → 이동 → 재패링 — 옛 좌표 워프 사라짐  ✓
```

---

## 남은 것

```
C1  Vertex Shake (WPO)       머티리얼만. Bone Shake 의 짝
B2  BP_GCN_ParryClash        BurstCameraShake 빈 칸
                             SB 실측 = CS_PC_DefaltParry_01
                               Dur 0.35 / BlendIn 0.0 / BlendOut 0.15
                               Loc X·Y·Z 전부 Amplitude 3.0 Frequency 40.0 / Rot·FOV 없음
                             ★ SB 는 Result_Parryed(내 공격이 패링당함) 에만 쉐이크를 쓴다
                               Result_Guard · Result_Guard_Break · Result_Parry 는 Anim + Sound 만
B4  가드 붕괴 / 막기 임팩트 GC   위 실측대로 쉐이크 없이 Anim + Sound 로 축소
PP  화면 PP 분리              SB 는 연출별 MI_*PP 를 Weight 커브로 블렌드
                             우리는 상주 M_PP_ScreenEffect + MPC.Intensity 공유
                             색을 갈라야 할 때 이관 (메모리 project_screeneffect_split_2026-06-06)
```

## 관련

- `docs/dev-logs/2026-08-31-slowmo-timescale-fix.md` — 같은 날 슬로모 시계 정정
- `docs/dev-logs/2026-08-30-presentation-audit.md` — `GCN_CounterTrail` 최초 조사(발신자 판정 오류 포함)
- `docs/dev-logs/2026-08-30-counter-slash-notify.md` — 반격(우) 노티 배치
