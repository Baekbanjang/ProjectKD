# 2026-08-10 — 총 3단계: 발사체

조준(1단계) → 사격 모션·크로스헤어(2단계)에 이어 **실제로 총알이 나가는 단계**. 작업을 A-1~A-4로 쪼개 진행했다.

| | 내용 | 커밋 |
|---|---|---|
A-1 | 발사체 결함 2건 | `b36c1c2` |
A-2 | 발사 노티 + 총알 스폰 코드 | `edd468a` |
A-3 | 에셋 (`BP_Bullet` + `GA_Shoot` BP 값) | Content |
A-4 | 스폰 경로 중복 제거 → `UKDAbilityStatics` | — |

---

## A-1 — 발사체 결함 2건

**① 충돌 델리게이트 바인딩이 `InitProjectile` 안에 있었다**

`InitProjectile`을 안 부르면 총알이 **벽도 적도 통과하고 5초 뒤 조용히 소멸**한다. 에러도 로그도 없다.
→ `BeginPlay`로 이동. 스폰 경로가 무엇이든 충돌은 항상 연결된다.

**② `GA_Dodge`가 발사체 발사자를 안 봤다**

`IsPerfectDodgeable()`만 검사해서 **내가 쏜 총알로 내 퍼펙트 회피가 떴다**. 발사체가 전부 적 것이던 시절엔 안 터지던 버그.
→ `&& Proj->GetInstigator() != Avatar` 한 줄.

## A-2 — 발사 배선

**`AN_Shoot` + `Event.Montage.Shoot` 신설.** `Event.Montage.TraceBegin` 재사용은 버렸다 — 그건 `UANS_MeleeTrace`(근접 판정 **구간** 노티)가 보내고 `OptionalObject`에 트레이스 설정까지 싣는다. 사격은 한 프레임 사건이라 구간이 필요 없다.

신규 `UGA_Shoot`(`UGA_ActionBase` 직속). 몽타주 재생 + 노티 프레임에 총알 스폰.

## A-3 — 에셋

```
Gun_Muzzle 소켓        SK_Mannequin_GunSword, hand_r 계열
AN_Shoot               AM_SB_Aim_the_Target_Shoot, t=0.0001s (몽타주 길이 0.667s)
BP_Bullet              /Game/SB_Style_GameProject/Combat/, 부모 = KDProjectile
GA_Shoot BP 값 3칸     ProjectileClass / DamageEffectClass / MuzzleSocket
```

**`BP_Bullet` 값** — Sphere 메시 Scale 0.15 + **NoCollision**(안 끄면 콜리전이 두 겹) / `ProjectileMovement` Initial·Max Speed **4000** / `InitialLifeSpan` **3.0** / `Perfect Dodgeable` ☑ 유지.

### ★ 무성 실패 지점 — `Muzzle Socket`

코드 기본값이 `"Muzzle"`인데 스켈레톤에 실재하는 소켓은 **`Gun_Muzzle`** 하나뿐이다(MCP 실측). 안 고치면 `GA_Shoot.cpp`가 `GetActorLocation()`으로 폴백해 **캐릭터 배꼽에서 총알이 나간다. 에러도 로그도 없다.**

### 곁가지 — 데미지 GE 칸을 잘못 찾고 있었다

계획서에 "`GA_LightCombo`에 지정된 GE와 같은 것"이라 적어뒀는데, 그 BP의 `DamageEffectClass`는 **비어 있었다.** 실제 BP 노출 칸은 `DefaultDamageEffectClass`이고 값은 `GE_Damage_Physical`(C++ 클래스 직결).

`GA_PlayerAttackBase.cpp:23`이 `DamageEffectClass = DefaultDamageEffectClass`로 **런타임에 복사**하는 구조라, CDO만 조회하면 빈 걸로 읽힌다. 콤보 노드가 자기 GE를 가지면 그걸로 다시 덮는다(`:34`).
→ **CDO 조회 결과가 비었다고 "미설정"으로 단정하면 안 되는 사례.** 런타임에 채워지는 칸이 따로 있다.

## A-4 — 스폰 경로를 하나로

`GA_Shoot::OnShootEvent`와 `GA_EnemyRangedAttack::OnReleaseProjectile`이 같은 절차를 두 벌 갖고 있었다(약 40줄). **진짜 다른 건 조준 방향 산출 한 군데뿐**이다 — 플레이어는 카메라 트레이스, 적은 플레이어 위치 직선.

```cpp
// KDAbilityStatics.h  (기존엔 함수 0개의 빈 껍데기였다)
static FVector GetMuzzleLocation(const AActor* Avatar, FName MuzzleSocket);
static AKDProjectile* SpawnDamageProjectile(
    UAbilitySystemComponent* InstigatorASC, AActor* Avatar,
    TSubclassOf<AKDProjectile> ProjectileClass, TSubclassOf<UGameplayEffect> DamageEffectClass,
    const FVector& SpawnLoc, const FRotator& SpawnRot, const FGameplayTagContainer& AbilityTags);
```

**분리 근거는 줄 수가 아니라 A-1의 ①이다.** 총알 만들기는 네 단계(총구 위치 → 데미지 Spec → 스폰 → `InitProjectile`)인데 **마지막을 빠뜨리면 무성 통과**한다. 스폰과 `InitProjectile`을 같은 함수에 묶으면 그 실수를 할 자리가 없어진다. 세 번째 발사체가 생겨도 마찬가지.

`UFUNCTION`은 안 붙였다 — BP에서 부를 일이 없고, 붙이면 `TSubclassOf` 핀이 BP에 노출돼 오용 경로가 생긴다.

**통합하며 미세 차이 2개를 한쪽으로 맞췄다** (동작 동일 확인)
- 소켓 조회: `GA_Shoot`는 `Cast<ACharacter>` 없이 `GetMesh()`였음 → 캐스팅 있는 쪽으로
- `Params.Instigator`: `GA_Shoot`는 암묵 업캐스트 → `Cast<APawn>`으로. 여기가 깨지면 `KDProjectile::BeginPlay`의 자기 충돌 무시와 A-1 ②의 발사자 비교가 **둘 다** 죽는다

---

## 설계 결정 4개 (재론 불필요)

**① 발사 GA를 `GA_AimMode` 자식으로 안 만들었다.** 상속은 코드를 물려받을 뿐 런타임 조건을 강제하지 않는다 — 부모가 꺼져 있어도 자식은 켜진다. `GA_AimMode` 안에 넣는 안도 버렸다: 팩에 **조준 없이 쏘는 애니**(`Combo_Attack_Shoot` 4개, `Dodge_Shoot` 4방향)가 있어 가두면 못 쓰고, GAS 쿨다운·코스트가 GA당 한 세트라 탄약이 붙으면 조준과 칸을 다툰다.
→ 강제는 **활성화 필요 태그 `State.Combat.Aiming`**으로.

**② 총구는 캐릭터 메시 소켓** (총 메시 아님). 총 메시 소켓을 쓰려면 `FindComponentByClass<UWeaponComponent>()`가 필요 → CLAUDE.md §1-3 "GA → Component 직접 참조 금지" 위반. 적 원거리도 `Char->GetMesh()`로 이 벽을 피한다. 총이 손에 고정이라 상대 위치가 안 변해 정확도 손해도 없다.
→ 총구 이펙트도 같은 소켓에 붙인다. `GameplayCueNotifyTypes.cpp:337`이 `TargetCharacter->GetMesh()`를 쓰므로 GC의 `SocketName`에 `Gun_Muzzle`을 넣으면 총알과 같은 자리.

**③ 조준 방향은 `GetPlayerViewPoint`.** ⚠️ **`ControlRotation`을 쓰면 안 된다** — 우리 피치는 시선 각도가 아니라 **레일 눈금**이다(08-03 카메라 개편). 08-05 락온이 정확히 이 함정에서 죽었다.

**④ 3인칭 정렬** — 크로스헤어는 화면 중앙, 총구는 어깨 옆. `카메라에서 트레이스 → 조준점 확정 → 총구에서 재조준` 순서. SB는 상체 AimOffset이 총구를 화면 중앙에 맞춰주는 걸 신뢰하지만, 우리는 좌우 AimOffset이 사실상 안 쓰여서(캐릭터가 카메라로 정렬돼 `TurnYawOffset`→0) 트레이스 방식을 골랐다.

## SB 총알 실측 (`ProjectileTable` 1357행 중 `P_Eve_Gun_ShootSlug1_Projectile1`)

```
Speed / Min / Max   20000  (셋 다 같음 = 등속)
LifeTime            0.5초  → 사거리 10000
PhysicsType         None   (중력 무시 직선)
안전장치            bCheckShotPosition + Bip001-Head (벽 뚫고 쏘기 방지)
```

우리는 **4000 / 3초**를 골랐다 — 20000은 눈에 거의 안 보여 프로토타입 검증에 불리하다.

---

## 검증

**PIE — 플레이어 발사 정상 동작 확인.** A-4 리팩토링 후 재확인도 통과(로직 이동뿐이라 동일 동작).

⚠️ **적 원거리는 실행 검증 못 했다** — `KDProjectile` 자식 BP가 `BP_Bullet` 하나뿐이라(전수 조회) 적 원거리 GA에 물릴 발사체 에셋이 아직 없다. 이번 확인 범위는 **컴파일 통과까지**. 적 작업 때 함께 볼 것.

## 남은 것

| | 상태 |
|---|---|
`GA_EnemyRangedAttack.cpp:10`의 `GameFramework/Character.h` | 고아 include(빌드엔 무해). A-4가 만든 것 |
탄약·탄창 | 재장전 없음 확정(팩에 클립 0개). 회복은 아이템인데 인벤토리 체계가 없어 보류 |
발사 확산 | SB 커브 확보됨 `0.00→1.0 / 0.08→1.3 / 0.12→1.0` 비대칭 |
반동 | ⚠️ `ControlRotation.Pitch`를 밀면 `RailAlpha`가 올라가 **카메라가 스플라인을 미끄러진다**. 상체까지 들려 발사 몽타주 반동과 이중. 카메라 컴포넌트에만 얹는 대안 검토 필요 |
`AN_Shoot` 위치 | t=0.0001s(사실상 t=0). 팔이 올라오기 전에 나가 보이면 조정 지점. 몽타주에서만 고치면 됨 |
