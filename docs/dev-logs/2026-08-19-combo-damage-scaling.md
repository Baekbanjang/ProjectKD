# 콤보 타별 데미지 계수 + 총격 조준 제한 + 타격감 더미

모든 타격이 `AttackPower 20` 고정이던 것을 타마다 갈랐다. 총격이 몸과 따로 놀던 것도 같이 잡았다.

**소스** — `ComboTreeDataAsset.h` / `GA_MeleeTraceBase.h·cpp` / `GA_PlayerAttackBase.h·cpp` / `GA_PlayerAirAttackBase.h·cpp` / `GA_ShotBlast.h·cpp` / `AN_ShotBlast.h` / `AS_Combat.cpp`
**에셋** — `DA_ComboTree` · `DA_AirComboTree` 계수 24칸 / `BP_Dummy` + `DA_Dummy_Immortal` · `DA_Dummy_Stagger`
**앞 문서** — [2026-08-19-enemy-melee-trace-tag](2026-08-19-enemy-melee-trace-tag.md)

---

## 1. 데미지 계수 — 칸 하나 + 곱셈 하나

`FComboNode` 에 `DamageMultiplier` 를 추가했다. 곱하는 자리는 코드 전체에서 하나다.

```cpp
// GA_MeleeTraceBase.cpp:210
const float AttackPower = AttackerASC->GetNumericAttribute(UAS_Combat::GetAttackPowerAttribute()) * DamageMultiplier;
```

흐름은 `DamageEffectClass` 가 이미 쓰던 것과 같다.

```
입력 -> ComboComponent.ProcessInput -> 노드 결정
     -> GA_PlayerAttackBase.ActivateAbility 가 노드 값을 GA 변수로 복사
        DamageMultiplier = DefaultDamageMultiplier          매 시작 복원
        if (Node->DamageMultiplier > 0) 노드 값으로 덮어씀
     -> 몽타주 -> ANS_MeleeTrace -> AT_MeleeTrace -> 명중
     -> OnWeaponHit 에서 AttackPower x DamageMultiplier
```

**복원 줄이 핵심이다.** GA 가 `InstancedPerActor` 라 인스턴스가 활성화 사이에 살아남는다. 없으면 마무리(1.8)를 쓴 뒤 1타가 1.8로 나간다.

### 공중은 별도 경로다

`UGA_PlayerAirAttackBase` 는 `UGA_PlayerAttackBase` 의 자식이 아니다. 형제라서 **같은 쌍을 두 번** 넣어야 했다(`DefaultAirDamageMultiplier`).

### 런타임 변수엔 `UPROPERTY` 를 안 붙였다

`DamageMultiplier`(런타임)는 매 활성화마다 덮인다. `UPROPERTY` 를 붙이면 BP 디테일에 칸이 뜨는데 그 값은 아무 의미가 없다 — 08-18 에 `DamageEffectClass` 로 "CDO 가 비어 있다"고 오진했던 자리다.

## 2. 값 — A안(5루트 동일 곡선)

SB 실측 `0.7 / 1.0 / 1.0 / 1.8` 을 5루트에 그대로 반복했다.

```
DA_ComboTree      C01~C05  각  0.7 / 1.0 / 1.0 / 1.8      (20칸)
                  Evade / JustEvade  0.0                   몽타주 없는 경유 노드
DA_AirComboTree   Air_1 0.9  Air_2 1.0  Air_3 1.2  Air_4 1.8
```

`AttackPower 20` 기준 **14 / 20 / 20 / 36.** 총합은 지금(80)과 비슷한 90인데 모양이 다르다 — 앞이 잘게 깎이다 마지막에 크게 뜯긴다.

**1타를 0.7로 낮춘 게 핵심이다.** 마무리만 올리면 전체가 세지기만 하고 대비가 안 생긴다.

우리 5루트가 SB 의 어느 계열에 대응하는지는 **검증된 적이 없다.** 루트별 성격 부여는 근거가 없어 보류했다.

## 3. ★ 라이브 코딩 직후 파이썬으로 구조체를 쓰면 데이터가 통째로 날아간다

`DA_ComboTree` 의 노드 22개가 **전 필드 초기화**됐다. NodeId `None`, 몽타주 없음, 링크 0. `Entries` 2개만 살았다.

```
원인   USTRUCT(FComboNode)에 칸을 추가하고 라이브 코딩으로 빌드한 직후
       에디터 재시작 없이 파이썬으로 그 구조체 배열을 되썼다
       -> 파이썬이 바뀌기 전 레이아웃으로 써서 필드가 밀림. 에러 X
```

복구 = Content git 에 정상본이 커밋돼 있어 `git checkout` 으로 되돌렸다(노드 22 / 링크 33 / Entries 2 전부 무손실).

### 안전 절차 (재사용)

```
1  USTRUCT 을 바꿨으면 에디터를 재시작한 뒤에 파이썬을 돌린다
2  배열 안 구조체는 버릴 복제본에서 먼저 증명한다
     duplicate_asset -> 쓰기 -> save -> reload_packages -> 필드 생존 확인
3  본 에셋 쓰기 전에 Content 커밋 (해당 폴더를 포함해서)
```

재시작 후 같은 코드가 정상 동작했다. 저장하고 디스크에서 다시 읽어도 노드 22 / 링크 33 이 온전했다.

> ⚠️ 이번에 세이브포인트를 `Blueprints/Enemy/Dummy` 만 add 해서 `ComboData` 가 빠져 있었다. 이전 커밋에 정상본이 있어 살았지 운이 좋았다. **커밋 범위에 대상 폴더가 들어갔는지 확인할 것.**

### 파이썬이 못 쓰는 프로퍼티

`EditDefaultsOnly` 는 파이썬 쓰기가 막힌다(`PropertyAccessChangeNotifyMode` 를 바꿔도 동일). 에디터에서는 편집된다.
`FComboNode` 는 `UComboTreeDataAsset` 안에서만 쓰여 인스턴스 개념이 없으므로, `InputWindow` 와 `DamageMultiplier` 두 float 만 `EditAnywhere` 로 열었다. 에셋 참조 칸(`Montage` · `NextLinks` · `DamageEffectClass`)은 그대로 뒀다.

## 4. 총격이 몸과 따로 놀던 것

증상 = 앞 적을 검으로 죽이고 총을 쏘면 **몸은 앞을 보는데 뒤에 있는 적이 맞는다.**

```cpp
// GA_ShotBlast.cpp — 판정 콘 방향만 타겟으로 돌린다. 몸은 안 돈다
const FVector ToTarget = (Target->GetActorLocation() - ConeOrigin).GetSafeNormal();
if (!ToTarget.IsNearlyZero()) ShotDir = ToTarget;
```

`GA_ShotBlast` 는 `UGA_ActionBase` 직속이라 `GA_PlayerMeleeAttackBase::OnActivated` 의 회전 코드를 안 탄다. 그리고 기준이 둘로 갈려 있었다.

```
타겟 선별   AutoAimConeAngle 180 = 카메라 정면 ±90도
몽타주      몸 기준. 타가 시작할 때 정한 방향에 고정
-> 둘이 벌어진 만큼 콘이 몸을 벗어난다
```

### 고침 = 클램프

몸 정면에서 벗어날 수 있는 각도에 상한을 뒀다. 몸을 돌리는 방법도 있으나 **몽타주 재생 중 `SetActorRotation` 은 홱 튄다.**

```cpp
const float BodyYaw = Avatar->GetActorRotation().Yaw;
const float DeltaYaw = FMath::FindDeltaAngleDegrees(BodyYaw, ToTarget.Rotation().Yaw);
const float ClampedYaw = FMath::Clamp(DeltaYaw, -BodyAimLimitAngle, BodyAimLimitAngle);

FRotator AimRot = ToTarget.Rotation();   // 피치는 타겟 그대로
AimRot.Yaw = BodyYaw + ClampedYaw;       // 요만 제한
ShotDir = AimRot.Vector();
```

`BodyAimLimitAngle` 기본 60. **0 이면 항상 정면, 180 이면 제한 없음**이라 값 하나로 양 극단을 오간다.

피치를 타겟 것으로 유지하는 이유 = 수평 벡터로 만들면 **공중 콤보에서 지상 적을 못 맞춘다.**

`AM_SB_Combo_05_03` 의 5발은 `bUseMuzzleDirection` 갈래라 영향 없다. 전방위 유지.

## 5. 총격 데미지 배수

`GA_ShotBlast` 는 콤보 노드를 모른다. `DamageMultiplier` 경로를 안 타서 20 고정이었다.

```
GA_ShotBlast.DefaultShotDamageMultiplier      0.8      전체 기본
AN_ShotBlast.ShotDamageMultiplierOverride     0 = GA 값 사용
```

`ShotHalfAngle` / `ShotHalfAngleOverride` 와 같은 패턴이다. **PIE 에서 16 확인.**

노드 배수를 그대로 쓰면 안 되는 이유 = 같은 타 안에서 **1발(`Combo_01_01`)이거나 5발(`05_03`)** 이다. 5발에 1.8을 곱하면 검 마무리의 다섯 배가 된다.

## 6. 온스크린 데미지 표시 (개발용)

```cpp
// AS_Combat.cpp — Health 차감 직전
#if !UE_BUILD_SHIPPING
    GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Yellow,
        FString::Printf(TEXT("%s  -%.0f"), *GetNameSafe(ASC->GetAvatarActor()), ToHealth));
#endif
```

**`ToHealth` 를 찍는다** — 실드 흡수와 방어 경감이 끝나고 체력에 실제로 들어간 최종값이다.
`#if !UE_BUILD_SHIPPING` 이라 지우는 걸 잊어도 출시 빌드엔 안 들어간다.

SB 조사 결과 데미지 숫자 위젯(`WB_MainHUD_DamageSlot`)이 실재하지만 **켜는 토글을 못 찾았다**(치트 7139행 · 옵션 · 트레이닝룸 19행 전부 없음). 상시 표시는 아니라는 것만 확인됐고, 우리는 위젯 대신 디버그 표시로 갔다.

## 7. 타격감 검증용 더미

```
/Game/Blueprints/Enemy/Dummy/
   BP_Dummy             부모 BP_Bandit / BehaviorTreeAsset = None
   DA_Dummy_Immortal    HP 99999 · Poise 99999 · 공격력 0 · 공격셋 0 · 넉백 400
   DA_Dummy_Stagger     Poise 3 + PoiseDamageByAttack{Ability.Player.Parry:1}
```

`EnemyDefinition` 만 갈아끼우면 한 BP 로 둘 다 쓴다. 새 클래스 0개.

### ★ `AIControllerClass = None` 으로 하면 안 된다

처음에 "컨트롤러를 없애면 제자리에 선다"로 만들었더니 **때려도 아무 반응이 없었다.**

```
AKDEnemyBaseCharacter::PossessedBy 가
   ASC InitAbilityActorInfo + StartupAbilities 부여 + DA 스탯 적용
을 전부 담당한다. 컨트롤러가 없으면 PossessedBy 가 아예 안 불린다
```

→ **컨트롤러는 두고 `BehaviorTreeAsset` 만 비운다.** `KDEnemyAIController::OnPossess:81` 이 BT 를 null 가드로 감싸므로 초기화는 다 되고 행동만 안 한다.

회전도 안 한다 — 컨트롤러 yaw 는 `SetFocus` 가 있어야 갱신되는데 그걸 설정하는 게 `BTService_FindPlayer` 다.

## 8. 검증

```
총격 데미지 16                     통과
총격 조준 클램프                    통과 (작성자 "어느 정도 제한해두는 게 맞는 듯")
데미지 계수 14 / 20 / 20 / 36       미검증
배수 잔류 (1타 -> 4타 -> 1타)       미검증
공중 18 / 20 / 24 / 36              미검증
적 공격 20 회귀                     미검증
```

## 9. 남은 것

```
넉백이 거리 단위가 아니다      LaunchCharacter 는 속도(400 cm/s)를 던지고 마찰이 멈춘다
                              실제 몇 cm 밀리는지 모른다 -> 다음 타가 닿는지 계산 불가
                              SB 는 Hit_Back_030_100 처럼 거리를 직접 적는다
넉백 차등 미구현               FComboNode.KnockbackMultiplier + EventMagnitude 로 실어 보내는 안까지 설계 완료
모션워핑                      콤보 루트모션 6~15m 문제(07-27 기록)와 넉백 문제를 한 번에 푼다. 미착수
HitConfirm 1종                DA_HitLightAttack 하나를 전 타격이 공유
GCN_CounterThrust / CounterTrail   이펙트 참조 0 (빈 껍데기)
```
