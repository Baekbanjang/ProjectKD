# 2026-08-21 — 조준 정확도 / 넉백 실효화 / 공중 클립 카메라

## 0. 요약

무음 결함 3건을 잡았다. **셋 다 "캡슐 기준으로 계산하는데 캡슐이 진실이 아닌" 문제**였다.

| 증상 | 진짜 원인 |
| --- | --- |
| 가까운 적만 총이 빗나감 | 조준 트레이스가 `ECC_Visibility` — 적이 이 채널을 무시해서 조준점이 적 뒤 벽에 찍힘 |
| 넉백 배수를 바꿔도 밀리는 거리가 그대로 | AI 경로추종이 `LaunchCharacter` 속도를 매 틱 덮어씀 |
| 공중 콤보에서 캐릭터가 화면 위로 벗어남 | 루트 Z 가 0 — 애니 안에서 몸만 뜨고 캡슐은 땅에 |

덤으로 `05_04` 총격 콘이 엉뚱한 데로 나가던 것도 같은 뿌리(캡슐 위치)에서 나왔다.

커밋 = `5105741` `b6a5647` `77d19cb` `cce4f2c` / Content `e1893fd`

---

## 1. 조준 사격이 가까운 적을 빗나감

### 관측

먼 벽은 정확히 맞는데 3m 앞 적만 옆으로 스친다. 디버그를 그려보니 **총구 선과 카메라 선이 적 위치에서 벌어져 있고 멀리서 붙었다.**

### 원인

```
BaseEngine.ini:3081   Pawn 프로파일           Visibility = Ignore
BaseEngine.ini:3083   CharacterMesh 프로파일   Visibility = Ignore
```

`GA_Shoot::OnShootEvent` 의 화면 중앙 트레이스가 `ECC_Visibility` 하나만 썼다. **적 캡슐과 메시가 둘 다 이 채널을 무시**하므로 선이 적을 관통해 뒤 벽에 꽂힌다. `AimPoint` = 벽.

총구는 카메라에서 옆으로 떨어져 있다. 출발점이 다르고 도착점(먼 벽)이 같은 두 선은 **중간에서 벌어진다.**

```
벌어짐 = 총구 오프셋 x (1 - 적거리 / 조준점거리)

적 300cm / 벽 3000cm  ->  오프셋의 90% 만큼 옆으로 지나감
벽을 직접 겨눔        ->  0. 그래서 먼 건 정확했다
```

### 해결

월드 트레이스로 벽을 잡고, **그 지점까지만** Pawn 오브젝트 트레이스를 한 번 더 건다.

```cpp
// GA_Shoot.cpp
FHitResult WorldHit;
const bool bWorldHit = World->LineTraceSingleByChannel(WorldHit, ViewLoc, TraceEnd, ECC_Visibility, TraceParams);
FVector AimPoint = bWorldHit ? WorldHit.ImpactPoint : TraceEnd;

FHitResult PawnHit;
const FCollisionObjectQueryParams PawnObjects(ECC_Pawn);
if (World->LineTraceSingleByObjectType(PawnHit, ViewLoc, AimPoint, PawnObjects, TraceParams))
{
	AimPoint = PawnHit.ImpactPoint;
}
```

끝점을 `AimPoint` 로 두면 **벽 뒤 적은 자동 탈락**한다. 거리 비교 코드가 필요 없다.

> 적 캡슐의 Visibility 를 Block 으로 바꾸는 길도 있으나 채택하지 않았다 — AI 시야 판정(`LineOfSightTo`)이 Visibility 를 쓰므로 적들끼리 서로의 시야를 가린다.

### 진단 도구

`GA_Shoot` 에 `bDrawAimDebug` 추가. 초록 구 = 조준점 / 노란 선 = 총구→조준점 / 청록 선 = 카메라→중앙. **초록 구가 적 몸에 붙으면 정상, 적 뒤 벽에 찍히면 트레이스 문제.**

---

## 2. 넉백이 배수를 안 따름

### 관측

```
speed 1800 (x1.0)  ->  99 cm    계산값 99   일치
speed  900 (x0.5)  ->  90 cm    계산값 46   불일치
```

속도를 절반으로 줄였는데 거리가 9% 만 줄었다.

### 계산 기준

```
BP_Bandit / BP_Dummy
  GroundFriction 8 x BrakingFrictionFactor 2 = 마찰 16
  BrakingDecelerationWalking 1000

d = v0/f - (a/f^2) x ln(1 + f x v0/a)
```

### 원인

디버그에 남은 속도를 찍게 하니 **0.1초 시점 300~500** 이 나왔다. 순수 감속이면 131 이어야 한다. 300~500 은 `MaxWalkSpeed 500` 범위다.

```cpp
// KDEnemyBaseCharacter.cpp  (수정 전)
AICon->StopMovement();   // 현재 요청만 취소
```

BT 가 다음 틱에 MoveTo 를 다시 걸면 `PathFollowingComponent` 가 속도를 자기 값으로 다시 쓴다. 밀린 거리 대신 **적이 걸어온 거리**를 재고 있었다.

엔진 확인 — `MaxWalkSpeed` 가 자르는 게 아니다.

```cpp
// CharacterMovementComponent.cpp:3836
const float NewMaxInputSpeed = IsExceedingMaxSpeed(MaxInputSpeed) ? Velocity.Size() : MaxInputSpeed;
```

이미 최대 속도를 넘었으면 클램프 기준을 현재 속도로 바꾼다. 강제로 깎지 않는다. 문제는 `:3845` 의 `RequestedAcceleration` 이 반대 방향으로 계속 가속하는 것.

### 해결

넉백 구간(0.15s)만 `BrainComponent` 를 멈춘다. **경직이 이미 쓰던 방식**(`OnStaggerBegin` 의 `PauseLogic("Staggered")`)을 그대로 쓴다.

```cpp
Brain->PauseLogic(TEXT("Knockback"));
GetWorldTimerManager().SetTimer(KnockbackBrainTimer, this,
	&AKDEnemyBaseCharacter::ResumeBrainFromKnockback, KnockbackBrainPause, false);
```

⚠️ **`ResumeLogic` 은 이유별로 세지 않는다.** 밀리는 도중 경직이 걸리면 넉백 타이머가 경직으로 멈춘 두뇌를 깨운다. 재개 함수 첫 줄에서 막는다.

```cpp
if (bIsDead || (StaggerComp && StaggerComp->IsStaggered())) { return; }
```

### 부산물

디버그 측정 시점을 **0.5초 → 0.1초**로 당겼다. 넉백은 0.1초면 90%가 끝나고, 그 뒤는 AI 이동이 섞인다. 짧게 재야 넉백만 보인다.

> 근본 해법은 `FRootMotionSource_MoveToForce` 로 전환하는 것 — 경로추종보다 우선하고 **cm 단위**로 지정한다(SB 방식). 미착수.

---

## 3. 공중 클립에서 카메라가 안 따라옴

### 원인

```
AS_Combo_Attack_05_04_Seq   enable_root_motion = True

t       root Z    root 이동    pelvis Z
0.00      0.0        0            119
0.40      0.0     +131            285
0.60      0.0     +148            291   <- 발사 프레임
0.80      0.0        -             322   <- 최고점
1.41      0.0     -414             45   <- 착지
```

**루트 뼈 Z 가 내내 0.** 점프가 통째로 골반·다리에 그려져 있고 루트는 땅에 붙어 있다. 엔진 입장에서는 제자리이므로 카메라가 따라갈 세로 이동이 없다.

### 왜 캡슐을 띄우지 않았나

메시가 캡슐에 붙어 있어서 캡슐을 올리면 **높이가 두 번 더해진다**(캡슐 +200 + 애니 골반 +200 = +400). 제대로 하려면 골반의 세로 움직임을 루트 트랙으로 옮기는 애니메이션 재작업이 필요하고, 걷는 중에는 엔진이 세로 루트모션을 버린다.

```cpp
// CharacterMovementComponent.cpp:5438   MoveAlongFloor
const FVector Delta = ProjectToGravityFloor(InVelocity) * DeltaSeconds;   // 세로 성분 제거
```

`MOVE_Flying` 전환 + 착지 재조정까지 딸려온다. 카메라 하나 때문에 하기엔 비싸다.

### 해결 — 스프링암이 골반을 직접 읽는다

```cpp
// KDSpringArmComponent::UpdateElevateOffset
const float PelvisZ = MeshComp->GetSocketTransform(PelvisSocketName, RTS_Component).GetLocation().Z;
Target = FMath::Max(0.f, PelvisZ - PelvisFollowThreshold) * PelvisFollowRatio;
ElevateOffset = FMath::FInterpTo(ElevateOffset, Target, DeltaTime, ElevateBlendSpeed);
```

그 값을 **두 곳에 같이** 더한다.

```cpp
SocketOffset = FVector(0.f, Point.Y, Point.Z + ElevateOffset);              // 카메라 위치
const FVector Aim(0.f, SocketOffset.Y, LookAtHeightOffset + ElevateOffset);  // LookAt 표적
```

둘을 같은 값만큼 올려야 **각도가 유지된 채 평행이동**한다. 하나만 올리면 화면이 기울거나 카메라가 위를 쳐다본다.

### 문턱값 = 실측으로 정함

클립 **993개** 전수 측정. 골반 최고 높이(메시 컴포넌트 기준):

```
322.7  AS_Combo_Attack_05_All_Seq
322.0  AS_Combo_Attack_05_04_Seq
303.9  AS_Target_01_Seq
283.7  AS_Combo_Attack_02_03_Seq
278.7  AS_Combo_Attack_02_04_Seq
277.6  AS_Combo_Attack_02_All_Seq
246.7  AS_Execution_02_Seq
204.0  AS_Skill_04_Seq
------------------------------------  큰 틈
169.4  AS_Dodge_Air_L_90_Seq
163.7  AS_Dodge_Air_R_90_Seq
161.7  AS_Combo_Attack_Air_03_Seq
      ... 나머지 985개는 더 아래
```

**204 와 169 사이가 비어 있다.** 문턱 180 이면 위 8개만 걸린다. 걸리는 8개가 전부 "캡슐은 땅에 있고 몸만 뜨는" 클립이다.

### 진짜 점프에는 왜 안 걸리나

진짜 점프는 캡슐이 같이 올라가므로 **골반의 액터 기준 높이가 그대로**다. 감지 조건이 증상과 정확히 일치한다.

### ⚠️ 좌표계 함정

측정값 322 는 **메시 컴포넌트 기준**이다. 메시는 발이 캡슐 바닥에 닿게 액터보다 88cm 아래에 붙어 있어서, 액터 기준(`RTS_Actor`)으로 재면 같은 자세가 **234** 로 나온다. 문턱 180 과 안 맞는다. → `RTS_Component` 필수.

또 `FindComponentByClass<USkeletalMeshComponent>()` 가 아니라 **`ACharacter::GetMesh()`** 를 써야 한다. 무기가 스켈레탈 메시면 그게 잡힌다.

---

## 4. `05_04` 총격 콘이 엉뚱한 데로

### 원인

```
루트 회전 Y   0.0  0.0  0.0 ...   내내 0     <- 몸은 안 돈다
루트 이동     0 -> +148 -> -414              <- 앞으로 달려나간다
```

접근 워프가 적 앞 140cm 에 붙여놓는데, **발사 프레임까지 148cm 를 더 전진해서 적을 지나친다.** 적이 등 뒤로 빠진다.

```cpp
// GA_ShotBlast.cpp
FVector ShotDir = Avatar->GetActorForwardVector();   // 대상 없으면 여기서 안 바뀜
```

`FindAutoAimTarget(ShotRange, AutoAimConeAngle)` 이 정면 좌우 90도만 뒤지므로 대상을 못 찾고, **조준을 포기하고 몸 정면으로 쏜다.** 조준을 잘못한 게 아니라 안 한 상태.

찾더라도 `BodyAimLimitAngle = 60` 이 요를 몸 정면 ±60도로 자른다.

### 해결

`AN_ShotBlast` 에 예외 칸 2개. **이 프로젝트가 이미 쓰는 "0 = GA 값" 규약**을 따른다.

```cpp
float AutoAimConeAngleOverride = 0.f;    // 조준 탐색 각도
float BodyAimLimitAngleOverride = 0.f;   // 몸 조준 한계각
```

`AM_SB_Combo_05_04` 노티에 **360 / 180**. 다른 총격 타는 GA 값(180 / 60) 그대로라 평상시 콤보에서 등 뒤로 쏘는 일은 없다.

`bUseMuzzleOrigin` 도 같이 추가 — 공중 자세에서 콘 원점이 캡슐(지면)이면 몸과 어긋난다. `05_04` 만 켰다.

---

## 5. 넉백 배수의 소유자를 총알로 이전

`GA_Shoot` 이 스폰 직후 총알 값을 덮어써서 **`BP_Bullet` 의 `Knockback Multiplier` 칸이 죽어 있었다.** 고쳐도 아무 일이 안 났다.

```cpp
// 삭제
if (Bullet) { Bullet->SetKnockbackMultiplier(KnockbackMultiplier); }
```

`GA_Shoot::KnockbackMultiplier` 와 `AKDProjectile::SetKnockbackMultiplier` 를 둘 다 걷어냈다. 이제 `AKDProjectile::KnockbackMultiplier` 가 단일 진실이다.

**총이 늘면 `BP_Bullet` 을 복제해 숫자만 바꾼다.** GA 를 복제하면 몽타주·탄약 코스트·조준 로직까지 같이 복제된다.

```
넉백 배수        총알 BP                        <- 이번에 이전
어떤 총알을 쓰나  GA_Shoot::ProjectileClass       <- 나중에 WeaponDataAsset 으로
기준 세기        적 DA KnockbackStrength 1800    <- 체급. 그대로
```

⚠️ 코드에서 빼면서 `BP_Bullet` 값을 안 옮기면 넉백이 2배가 된다(기본값 1.0).

### 발사체 인벤토리

```
BP_Bullet          knock 0.50  speed 4000  life 3.0
BP_Arrow           knock 1.00  speed 3000  life 5.0
BP_Arrow_Charged   knock 1.00  speed 1200  life 5.0
```

화살 2개의 값은 **현재 아무 데도 안 쓰인다** — `Event.Combat.Hit` 수신자가 `KDEnemyBaseCharacter.cpp:117` 한 곳뿐이라 플레이어가 맞아도 읽는 코드가 없다.

---

## 6. 직전 미기록분 (08-20)

문서를 안 남긴 채 커밋된 것들.

### 넉백 배수 레이어

```
적 DA KnockbackStrength   기준 세기 (체급)      1800 x 6종
FComboNode.KnockbackMultiplier   타별 배수
AN_ShotBlast / GA 기본값         총격 배수
        |
   FGameplayEventData::EventMagnitude 로 전달
        |
KDEnemyBaseCharacter.cpp:470   0 이면 1.0 폴백
```

기준값을 400 → 1800 으로 올렸다. 400 은 계산상 **17cm** 라 사실상 안 밀리는 값이었다.

`FComboNode.KnockbackMultiplier` 는 `EditDefaultsOnly` 가 아니라 **`EditAnywhere`** 로 뒀다 — 파이썬 쓰기가 `EditDefaultsOnly` 에서 차단된다.

### 회피 몽타주 트림 + 노티 재배치

착지가 원본 f56~58(B/L/R) / f44(F_0) 인데 몽타주가 착지 전에 잘려 있었다. 트림 후 `ANS_MovementCancel` 을 착지 지점으로 옮겼다. **아직 부자연스러운 구간이 남아 있다** — 다음 레버는 `Anim End Time` 을 1.0333 너머로 연장.

### 적 상태 바 폭 상한

더미의 `MaxPoise` 가 99999 라 바가 화면 밖으로 늘어났다. 폭 계산이 `RefreshPoise` 가 아니라 **`EventGraph` 의 `SetTarget` 이벤트**에 있었다(`Get MaxPoiseCache x 10 -> Set Width Override`). `Min(Float, 10)` 삽입.

---

## 7. 이번에 쓴 조회법

```python
# 몽타주가 쓰는 원본 시퀀스 찾기 — 몽타주 프로퍼티는 대부분 protected
ar.get_dependencies(pkg, unreal.AssetRegistryDependencyOptions(include_hard_package_references=True))

# 노티 목록 — get_editor_property('notifies') 는 protected 라 막힘
unreal.AnimationLibrary.get_animation_notify_events(m)
unreal.AnimationLibrary.get_anim_notify_event_trigger_time(e)

# 뼈 좌표 — 원본 AS_ 시퀀스에만. 몽타주에 쓰면 에디터가 죽는다
unreal.AnimationLibrary.get_bone_pose_for_time(s, 'pelvis', t, False)
```

⚠️ **bool UPROPERTY 는 파이썬에서 `b` 접두사가 빠진다.** `bUseMuzzleOrigin` → `use_muzzle_origin`. `b_use_muzzle_origin` 으로 찾으면 "없다"고 나와서 **빌드가 반영 안 된 걸로 오판하기 쉽다.**

⚠️ PIE 중에는 `load_asset` 이 **`None` 을 반환**한다. 로그에 `The Editor is currently in a play mode` 만 뜨고 예외는 안 난다.

---

## 8. PIE 단축키 (엔진 소스 확인)

```
DebuggerCommands.cpp:368   재생 <-> 일시정지 토글    Pause 키
DebuggerCommands.cpp:369   빙의 해제 / 재빙의         F8
DebuggerCommands.cpp:365   마우스 커서 꺼내기         Shift+F1
DebuggerCommands.cpp:367   한 프레임 넘기기(Skip)     기본 단축키 없음 — 직접 지정
```

디버그 선은 정지 중에 안 사라진다(시간이 안 흐름). `GA_ShotBlast` 의 디버그 수명은 1초 고정이라 정지가 늦으면 이미 없다 — `slomo 0.2` 를 먼저 친다.

---

## 9. 남은 것

```
확인      Execution_02(골반 246) 가 처형 전용 카메라와 겹치는지
          Target_01(303) 이 뭔지 — 실사용 클립인지 미확인
          Combo_02_03 / 02_04 도 05_04 와 같은 증상일 것
폴리싱    05_04 총격 (아직 완전하지 않음)
          회피 착지 (Anim End Time 연장)
근본      넉백을 RootMotionSource 로 (cm 단위, AI 간섭 X)
          공중 클립의 세로 움직임을 루트 트랙으로 (애니 재작업)
```
