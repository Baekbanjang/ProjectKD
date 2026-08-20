# 콤보 진입기 접근 워프 — 반경 안 적에게 붙어서 시작

멀리 있는 적에게 좌클릭하면 첫 타가 허공을 갈랐다. 콤보 진입기에 모션 워핑을 걸어 적 앞까지 붙게 했다.

**소스** — `GA_PlayerMeleeAttackBase.h` / `.cpp`
**에셋** — `AM_SB_Combo_{01~05}_01` 노티 5개 / `GA_LightCombo` · `GA_HeavyCombo` CDO
**앞 문서** — [2026-08-19-combo-damage-scaling](2026-08-19-combo-damage-scaling.md)

---

## 1. 부품 두 개가 다 있어야 동작한다

```
코드   Warp->AddOrUpdateWarpTarget...("ComboTarget", 위치)   "여기로 가라" 좌표 등록
노티   AnimNotifyState_MotionWarping, 이름 ComboTarget       "이 구간에 그 좌표를 써라"
```

전날 코드만 들어가 있었다. **좌표는 등록되는데 읽는 쪽이 없어 아무 일도 안 일어났다.**

노티 구간 = `[0프레임 ~ ANS_MeleeTrace 시작]`. 판정 구간까지 물리면 때리는 도중에 미끄러진다.

## 2. ★ 워프창 이동량 실측 — 배율이 곧 미끄러짐

모션 워핑은 위치를 만들어내지 않는다. **원래 있던 루트모션 이동량을 늘리거나 줄인다.**

```
C01_01 워프창에서 원래 85cm 간다
적이 350cm 앞  ->  요구 이동 = 350 - 200 = 150cm
85 -> 150 = 1.76배.  발은 85cm 속도인데 몸은 150cm 간다 = 그 차이가 미끄러짐
```

20개 클립 전수 측정 (창 = `[0, 판정시작]`, 값 = 그 구간 수평 이동량)

| 루트 | 1타 | 2타 | 3타 | 4타 |
|---|---|---|---|---|
| C01 | 85 | 139 | 52 | 122 |
| C02 | 153 | **0** | 223 | 244 |
| C03 | 67 | 73 | *총격* | 391 |
| C04 | 70 | 95 | 248 | 278 |
| C05 | 178 | 246 | 287 | *총격* |

진입기 판정 시작 = `f11 / f20 / f8 / f13 / f17` (0.183 / 0.333 / 0.133 / 0.217 / 0.283초).

**`C02_02` 는 0cm** — 제자리에서 시작하는 모션이라 워프를 걸면 나누기 0 = 순수 순간이동. 전체 적용 시 이 클립만 제외하거나 창을 옮겨야 한다.
**`C03_03` · `C05_04` 는 `ANS_MeleeTrace` 가 없다** — 총격 타라 `AN_ShotBlast` 로 판정한다. 창 끝을 그 노티 시각으로 잡아야 한다.

### 거리 값은 계산이 아니라 체감으로 정했다

처음엔 배율을 근거로 `MaxApproachRange` 400 / `ApproachStopDistance` 200 으로 잡았다. **PIE 에서 둘 다 부족했다.**

```
StopDistance 200   붙긴 하는데 검이 안 닿는다
MaxRange 400       짧다고 느끼는 거리에서 아예 안 붙는다
```

`ApproachStopDistance` 는 검 사거리 실측으로 내렸다. `AM_SB_Combo_01_01` 판정창에서 검 자루가 골반에서 **최대 94cm**, 검날 129 를 더해도 팁이 223cm 이고 그것도 날이 정면 수평일 때만이다. 200 은 사거리 경계선이었다. → **140**

`MaxApproachRange` 는 그냥 올렸다. 최종 배율(StopDistance 140 기준, 괄호는 cm/s · 풀스프린트 800)

| 거리 | 요구 | C01 | C02 | **C03** | C04 | C05 |
|---|---|---|---|---|---|---|
| 400 | 260 | 3.1배 (1418) | 1.7 (780) | **3.9 (1950)** | 3.7 (1200) | 1.5 (919) |
| **600** | 460 | 5.4배 (2510) | 3.0 (1381) | **6.9 (3451)** | 6.6 (2124) | 2.6 (1626) |

**내가 잡았던 "2~3배면 미끄러져 보인다"는 기준선이 틀렸다.** 6배에서도 체감이 멀쩡했다. 몽타주 자체가 빠른 돌진 모션이라 배율이 눈에 안 띈다. **수치로 상한을 정하려던 게 과했고, 실제 기준은 PIE 체감이다.**

`C03_01` 이 항상 최악이다 — 워프창이 0.133초뿐이라 같은 거리를 제일 빨리 가야 한다. 어디서 깨지든 여기가 먼저 깨진다.

### ⚠️ `AutoAimRange` 가 상한이다

```cpp
AActor* Target = FindAutoAimTarget(AutoAimRange, AutoAimConeAngle);   // 반경 AutoAimRange 구체 오버랩
...
if (Dist > MaxApproachRange || ...) return;
```

**적을 찾는 반경이 먼저다.** `AutoAimRange` 500 인 채로 `MaxApproachRange` 만 600 으로 올리면 500~600 구간은 `Target` 이 `nullptr` 이라 아무 일도 안 난다. **둘을 같이 올려야 한다.**

`C05_01` 은 어느 거리에서도 배율이 낮다 — 대쉬 찌르기가 적을 지나치던 것이 오히려 줄어든다.

## 3. 무음 결함 — 등록한 좌표가 조기 리턴에 살아남았다

증상 두 가지가 같은 뿌리였다.

```
죽은 적에게 접근한다
한 대 때리고 도망친 적을 다시 치면 "처음 때린 자리"로 끌려간다
```

```cpp
// 고치기 전
58:  if (!Target) return;                         // 자동 조준이 죽은 적/시야 밖 적을 걸러 nullptr
75:  Warp->RemoveWarpTarget(ApproachWarpName);    // 지우는 줄이 그 아래 = 도달 못 함
```

`FindAutoAimTarget` 은 죽은 적을 이미 거른다(`KDEnemyBaseCharacter.h:54` `CanBeTargeted_Implementation() { return !bIsDead; }`). 그래서 타겟이 없어져 58줄에서 나가는데, **직전 공격이 등록해둔 좌표는 지워지지 않는다.** 몽타주의 워프 노티는 그 낡은 좌표를 읽는다.

**워프 타겟은 `RemoveWarpTarget` 을 명시적으로 부를 때까지 컴포넌트에 영구히 남는다.** 노티가 끝나도 안 지워진다.

→ `RemoveWarpTarget` 을 함수 맨 위로. **매 공격 시작마다 백지화하고 조건이 맞을 때만 다시 쓴다.**

### 곁다리 — 135도 게이트가 회전만 막고 있었다

```cpp
if (FMath::Abs(DeltaYaw) <= 135.f)
    Attacker->SetActorRotation(...);   // 뒤쪽 적이면 안 돈다
...
Warp->AddOrUpdateWarpTarget...(...);   // 그런데 워프는 그대로 걸었다
```

뒤쪽 적이면 몸은 안 돌린 채 워프가 끌고 갔다. 노티의 `Warp Rotation` 이 이동 중에 몸을 돌려서 뒤로 끌려가며 도는 그림이 됐다. → 일단 조기 리턴으로 바꿔 회전도 접근도 같이 막았다.

**그다음 PIE 에서 이 게이트 자체가 폐기됐다.** 옆·뒤에 선 적에게 좌클릭하면 **몸도 안 돌고 워프도 안 걸렸다.** 두 증상이 같이 죽은 게 단서였다 — 거리 게이트는 회전보다 **아래**에 있으므로 그게 원인이면 몸은 돌았어야 한다. 남는 건 타겟 부재와 이 135도 게이트뿐이었고, 콘을 360 으로 열어둔 상태라 후보로는 들어왔는데 이 줄이 다시 잘라내고 있었다.

**승환 요구 = "내 뒤에 오는 적도 찾아가야 한다"** → 줄 삭제. 이제 몇 도든 즉시 그쪽으로 돈다.

### 각도 필터가 둘이고 기준이 다르다

| | `AutoAimConeAngle` | 135도 게이트 |
|---|---|---|
| 기준 | **카메라** 정면 | **내 몸** 정면 |
| 언제 | 후보를 모을 때 | 후보를 고른 뒤 |
| 어디 | `LockOnComponent.cpp:186` | `GA_PlayerMeleeAttackBase.cpp:74` (삭제됨) |

**카메라와 몸은 따로 논다.** 마우스로 카메라만 돌리면 몸은 그대로라, 카메라 기준 정면인 적이 몸 기준으로는 160도 뒤일 수 있다. 그래서 콘을 열어도 안 먹었다.

`AutoAimConeAngle` 은 **전각**이다(180 = 좌우 90도). 360 을 넣으면 `cos(180°) = -1` 이라 반경 안 전원이 후보가 된다. 정렬은 그대로 각도 최소라 **앞에 적이 있으면 앞이 이기고, 뒤쪽 적은 앞이 비었을 때만 뽑힌다.**
⚠️ 같은 클래스의 `ShotHalfAngle` 은 이름대로 **반각**이다. 섞어 읽으면 틀린다.

## 4. 위치 스냅샷 -> 컴포넌트 추적

`AddOrUpdateWarpTargetFromLocationAndRotation` 은 **활성화 순간의 좌표를 사진 찍는다.** 그 뒤 적이 움직여도 그 자리로 간다.

```cpp
// RootMotionModifier.cpp:72
FTransform FMotionWarpingTarget::GetTargetTrasform() const
{
    if (Component.IsValid() && bFollowComponent)
    {
        Transform = Component->GetComponentTransform();   // 매 프레임 새로 읽음
        RecalculateOffset(Transform);
        return Transform;
    }
    return FTransform(Rotation, Location);                // false = 등록 당시 스냅샷
}
```

`AddOrUpdateWarpTargetFromComponent(..., bFollowComponent = true, ...)` 로 바꿨다.

**멈출 거리도 엔진이 계산한다.**

```cpp
// RootMotionModifier.cpp:104 — VectorFromTargetToOwner
const FVector ContextVector = (AvatarActor->GetActorLocation() - InTransform.GetLocation()).GetSafeNormal();
```

적 -> 나 방향이라 `LocationOffset.X = ApproachStopDistance` 면 적 앞 200cm 지점이 된다.
`LocationOffset` 의 Y·Z 가 0이면 `bCacheForwardOffset = false` 라 **매 프레임 재계산**된다(`:97~100`). 하나라도 0이 아니면 등록 시점 방향으로 캐시되니 주의.
`AvatarActor` 는 컴포넌트가 `GetOwner()` 를 자동으로 넣는다(`MotionWarpingComponent.cpp:831`).

### ⚠️ 회전 기준이 같이 바뀐다

컴포넌트 추적은 회전값도 **적 컴포넌트에서** 읽어온다.

```cpp
// RootMotionModifier.h:274~
Default : Character rotates to match the rotation of the sync point
Facing  : Character rotates to face the sync point
```

`Default` 는 그 회전을 그대로 쓴다 = **적이 등을 보이면 플레이어도 같은 방향을 본다.**
좌표를 직접 넘기던 때는 `ToTarget.Rotation()` 을 같이 줬으므로 `Default` 로 문제없었다. **추적으로 바꿨으면 노티를 `Facing` 으로 같이 바꿔야 한다.**

## 5. 최종 코드

```cpp
void UGA_PlayerMeleeAttackBase::OnActivated()
{
    ACharacter* Attacker = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    if (!Attacker) return;

    UMotionWarpingComponent* Warp = ApproachWarpName.IsNone()
        ? nullptr : Attacker->FindComponentByClass<UMotionWarpingComponent>();

    // 지난 공격 좌표 제거 — 타겟 없는 경로에서 옛 좌표로 끌려감 방지
    if (Warp) Warp->RemoveWarpTarget(ApproachWarpName);

    AActor* Target = FindAutoAimTarget(AutoAimRange, AutoAimConeAngle);
    if (!Target) return;

    const FVector ToTarget = (Target->GetActorLocation() - Attacker->GetActorLocation()).GetSafeNormal2D();
    if (ToTarget.IsNearlyZero()) return;

    Attacker->SetActorRotation(FRotator(0.f, ToTarget.Rotation().Yaw, 0.f));

    if (!Warp) return;
    USceneComponent* TargetRoot = Target->GetRootComponent();
    if (!TargetRoot) return;

    const float Dist = FVector::Dist2D(Target->GetActorLocation(), Attacker->GetActorLocation());

    // 사거리 밖 = 제자리 / 코앞 = 몽타주 원래 이동량 유지
    if (Dist > MaxApproachRange || Dist <= ApproachStopDistance) return;

    Warp->AddOrUpdateWarpTargetFromComponent(
        ApproachWarpName, TargetRoot, NAME_None, true,
        EWarpTargetLocationOffsetDirection::VectorFromTargetToOwner,
        FVector(ApproachStopDistance, 0.f, 0.f), FRotator::ZeroRotator);
}
```

`Dist` 계산 아래에 개발용 온스크린 표시를 넣었다(`#if !UE_BUILD_SHIPPING`). **거리와 어느 게이트에 걸렸는지를 같이 찍는다.**

```
Approach 워프   dist 320   (stop 140 / max 600)
Approach 멂     dist 710   (stop 140 / max 600)
Approach 붙음   dist 100   (stop 140 / max 600)
```

**아무 줄도 안 뜨는 것이 가장 큰 단서다** — 이 지점은 타겟을 찾은 뒤라, 안 뜨면 그 위에서 나간 것이다. 135도 게이트를 잡아낸 게 이 성질이었다.

`ApproachWarpName` 이 스위치다. 비면 전부 꺼진다. `GA_AirLightAttack` · `GA_CounterThrust` · `GA_SprintAttack` 은 `None` 이라 접근이 안 걸린다(`GA_CounterThrust` 는 자기 `CounterTarget` 워프를 따로 쓴다).

`Dist <= ApproachStopDistance` 에서 등록을 안 하는 이유 = 워프가 루트모션을 **덮어쓰므로** 코앞에서 걸면 그 타가 제자리에 못 박힌다.

## 6. ★ 노티 시각을 정규식으로 캘 때 — `LinkValue` 는 두 번 나온다

`AnimNotifyEvent` 구조체는 시각 프로퍼티를 노출하지 않아 `export_text()` 를 파싱했다.

```
(TriggerTimeOffset=..., Duration=0.1666, EndLink=( ... LinkValue=0.3500 ... ), ... , LinkValue=0.1833)
                                                        ^ 앞 = 끝 시각            ^ 뒤 = 시작 시각
```

`re.search` 로 첫 매치를 잡으면 **끝을 시작으로 읽는다.** 이걸 틀려서 워프창을 판정 구간까지 덮게 박았다가 되돌렸다. `re.findall(...)[-1]` 을 쓸 것.

> 애초에 `unreal.AnimationLibrary.get_anim_notify_event_trigger_time(event)` 라는 함수가 있다. 구조체 메서드가 아니라 라이브러리 함수라 `dir(event)` 에 안 보인다.

노티 **시각 수정 API 는 없다.** 고칠 땐 `remove_animation_notify_events_by_track` 으로 트랙을 비우고 재추가한다.

## 7. 최종 값 + 검증

**`GA_LightCombo` · `GA_HeavyCombo` (BP CDO)**

```
AutoAimRange           500 -> 700     찾는 반경. MaxApproachRange 의 상한이다
AutoAimConeAngle       180 -> 360     뒤쪽 적도 후보
ApproachWarpName       ComboTarget    스위치 겸 노티 이름
ApproachStopDistance   200 -> 140     검 사거리 실측 반영
MaxApproachRange       700 -> 600
```

`GA_AirLightAttack` · `GA_CounterThrust` · `GA_SprintAttack` 은 `ApproachWarpName = None` = 꺼짐.
`GA_ShotBlast` 는 **별개 클래스**라 이 값들과 무관하다 — 자기 `AutoAimConeAngle` 180 / `BodyAimLimitAngle` 60 을 그대로 쓴다.

**노티 5개** — 트랙 `Warp`, `[0 ~ 판정시작]`, `ComboTarget`, `Rotation Type = Facing`.

```
빌드            통과
PIE             통과 — 작성자 "나쁘지 않다"
디버그 CVar     a.MotionWarping.Debug 2 / .Debug.Target 2 / .DrawDebugLifeTime 3
                워프창 동안만 구 + 선 + "Warp target name: ComboTarget, is dynamic: True"
```

## 8. 남은 것

```
넉백 거리 단위화        다음 작업. LaunchCharacter 속도 -> cm.
                        그 결과에 따라 위 거리 값들을 다시 손볼 수 있다
나머지 15개 클립 노티    C02_02(창 안 이동량 0cm) 제외 / 총격 2타는 창 끝 별도 측정
노드별 워프 거리         GA 단위라 클립마다 2.7배 차이를 한 값으로 덮는다.
                        C03_01 이 항상 먼저 깨진다 -> FComboNode 에 칸 추가
헤더 기본값              헤더 500/700/200 인데 BP 는 700/600/140. 코드만 보면 헷갈린다
디버그 표시 정리         온스크린 Approach 줄. 튜닝 끝나면 뺀다
```

## 9. SB 는 일반 콤보에서 접근하지 않는다 (반론, 인지하고 진행)

`CharacterMoveTable` 실측 — 이브 라이트 콤보는 `PositionType Self` / `RotationType None`. 타겟으로 가는 건 100여 개 중 15개뿐이고 대시 어택 계열이다.

SB 가 접근 없이 되는 이유 = **넉백을 `Hit_Back_030_100` 처럼 거리로 지정**해 다음 타 사거리를 맞춰 짜기 때문이다. 우리는 `LaunchCharacter` 속도라 몇 cm 밀리는지 모른다.

접근 워프는 그 계산을 안 해도 되게 만드는 우회로다. 넉백을 거리 단위로 바꾸면 다시 볼 자리.
