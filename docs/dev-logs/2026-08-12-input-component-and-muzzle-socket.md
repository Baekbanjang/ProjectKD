# 2026-08-12 — 입력 컴포넌트 분리 + GA 접근자 통일 + 총구 소켓 이전

`AKDPlayerCharacter` 한도 초과 리팩토링 + GA의 구체 Pawn 캐스팅 제거 + 총구 위치를 무기 메시 소켓으로 이전. 결함 2건도 같이 닫혔다.

커밋 = 소스 `d65d21a` `171e786` `ae076f3` `512c3a4`(push 완료) / Content `b1dc4e8`(로컬). 소스 11파일 + Content 5에셋 미커밋.

---

## 1. `UKDPlayerAbilityInputComponent` 신설

`Source/Project_KD/Player/KDPlayerAbilityInputComponent.h`(64줄) `.cpp`(352줄).

`AKDPlayerCharacter`가 `.h` 150 + `.cpp` 276 = **426줄** (리팩토링 전 700줄). CLAUDE.md §1 "Pawn 500줄 초과 금지" 위반 해소.

옮긴 것 = `Try*` 8개(`TryLightAttack`/`TryHeavyAttack`/`TryParry`/`TryParryStop`/`TryDodge`/`TryExecute`/`TryAimStart`/`TryAimStop`) 본문 / 입력버퍼 소비(`ConsumeBufferedInput`) / 제자리 턴(`UpdateTurnInPlace`) / `ExecutionRange`.

캐릭터에 남은 `Try*` 8개는 BP 호환용 한 줄 위임으로만 남았다. `AKDPlayerController`는 **무변경** — 입력 바인딩 경로 그대로.

컴포넌트는 캐릭터 생성자 `CreateDefaultSubobject`로 생성해 BP에 자동 상속. `BeginPlay`에서 형제 컴포넌트 4개(`InputBuffer`/`ComboComp`/`LockOnComp`/`SprintComp`)를 `FindComponentByClass`로 1회 조회해 멤버에 캐싱.

⚠️ `.cpp` 352줄로 CLAUDE.md §1 "Component 300줄 초과 시 분리 검토" 선에 걸린다 — **분리 여부는 미결.**

## 2. GA → Component 접근자 통일

`UGA_ActionBase`에 `GetLockOnComponentFromActorInfo()` / `GetComboComponentFromActorInfo()` 추가 (`GA_ActionBase.h:42,45`).

`Cast<AKDPlayerCharacter>` 4곳(`GA_CounterThrust` / `GA_PlayerAirAttackBase` / `GA_PlayerAttackBase` / `GA_PlayerMeleeAttackBase`) 제거 → **`AbilitySystem/` 폴더에서 `KDPlayerCharacter.h` include 0개.**

근거 = CLAUDE.md §1-3 개정(같은 날 `d65d21a`). 엔진 `FGameplayAbilityActorInfo`가 컴포넌트를 이미 `BlueprintReadOnly`로 넘긴다. 실제 결합은 구체 Pawn 캐스팅 쪽이라 그것만 걷어냈다. 이름은 Epic 규약(`GameplayAbility.h:180` `GetOwningComponentFromActorInfo`) 따름. 동작 변화 없음 — 같은 컴포넌트를 `FindComponentByClass`로 찾을 뿐.

## 3. 총구 위치를 무기 메시 소켓으로

`GetMuzzleLocation(Avatar, Socket)` → `GetMuzzleTransform(Avatar, Socket, WeaponTag = NAME_None)`로 교체. 반환도 `FVector` → `FTransform`(`KDAbilityStatics.h/.cpp`).

폴백 순서 = **무기 메시 소켓 → 캐릭터 메시 소켓 → 액터 트랜스폼.** 무기 선택은 `Avatar->GetComponents<UWeaponComponent>()`를 돌며 `GetWeaponComponentTag() == WeaponTag`인 것을 찾는다. 플레이어는 `WeaponComponent`(Sword) + `GunWeaponComponent`(Gun) 두 개를 갖는 구조라 태그로 구분한다.

`WeaponTag`를 비우면(`NAME_None`) 무기 조회를 생략한다 — `GA_EnemyRangedAttack`은 이 인자를 안 넘겨 종전 동작(캐릭터 메시 소켓) 그대로.

`GA_ShotBlast` / `GA_Shoot` 둘 다 `MuzzleSocket` 기본값을 `"Muzzle"`, `WeaponTag` 기본값을 `"Gun"`으로 노출. 총구 소켓이 **캐릭터 스켈레톤 `Gun_Muzzle` → 총 스태틱메시 `Muzzle`**로 이동했다 — 옛 소켓은 `gun_weapon_l` 본 방향을 그대로 물려받아 X축이 총열과 어긋나 있었다.

```
총 메시    /Game/Gun_and_Sword/Demo/Characters/Mannequins/Meshes/Gun
소켓       Muzzle, 위치 (0, -8.5, -70)
```

이점 = 무기 교체 대응 / 홀스터에 넣으면 총구도 따라감 / 애니가 손 위치를 바꿔도 안 어긋남.

`GA_ShotBlast`의 인라인 소켓 회전 조회(`GetOwningComponentFromActorInfo()->GetSocketRotation()`) 6줄을 삭제하고 `MuzzleXf.GetUnitAxis(EAxis::X)`로 대체.

## 4. `AN_ShotBlast` 노티별 예외 3개

```cpp
bool  bIgnoreHitStop        = false;   // 히트스톱 끄기
bool  bUseMuzzleDirection   = false;   // 총구 방향으로 발사
float ShotHalfAngleOverride = 0.f;     // 각도 예외 (0 = GA 값)
```

`Payload.OptionalObject = this`로 노티가 자기 자신을 실어 보내 GA가 노티별 설정을 읽는다. 발사 방향 우선순위 = **총구 소켓 X축 > 락온 타겟 > 액터 전방.** 용도 = `AM_SB_Combo_05_03`의 윈드밀 회전 5연사.

## 5. 결함 2건 — 둘 다 조용히 아무 일도 안 하던 종류

**(a) 태그 충돌** — `GA_ShotBlast`와 `GA_Shoot`이 둘 다 Ability Tag로 `Ability.Mugong.Shoot`을 갖고 있었다. 조준 사격 입력이 `TryActivateAbilitiesByTag`를 부르면 **둘 다 켜져서 조준 사격 한 발마다 근접 콘 히트스캔 데미지가 공짜로 들어갔다.** → `Ability.Mugong.ShotBlast` 신설(`KDGameplayTags.h:19` / `.cpp:15`), `GA_ShotBlast` BP의 Ability Tag를 이쪽으로 교체(Content, 미커밋).

**(b) 퍼펙트 회피가 성립한 적이 없었다** — `ANS_EnemyAttackWindow` 생성자가 `NotifyColor`만 넣고 `AttackWindowTag`는 안 넣었다(`ANS_EnemyAttackWindow.cpp`). `NotifyBegin`/`NotifyEnd`가 `!AttackWindowTag.IsValid()`면 즉시 return하므로, 태그 칸을 안 채운 노티는 에러도 로그도 없이 no-op. 형제 클래스 `ANS_CancelWindow`는 생성자에서 기본값을 넣는데 이쪽만 빠져 있었다. → 생성자 이니셜라이저 한 줄(`: AttackWindowTag(GameplayTags::State_Combat_EnemyAttackHitWindow)`). **이미 배치된 노티에 소급된다**(UE는 CDO와 같은 값을 직렬화하지 않아, 비워둔 노티는 새 CDO 기본값을 읽는다).

---

## 검증 결과 (PIE)

- 리팩토링 회귀 — 약공·강공 콤보 / 회피·패링·처형·조준 / 제자리 턴·빠른 연타 **전부 이상 없음**
- 총구 소켓 이전 후 — 조준 사격·콤보 총격·적 원거리 **전부 이상 없음**
- 태그 분리 후 — 조준 사격에 콘 안 나옴 **확인**

## 실측으로 밝힌 것

- `DrawDebugCone`은 각도 180°에서 꼭짓점 16개가 **뒤쪽 한 점으로 뭉쳐 바늘로 보인다**(`LineBatchComponent.cpp:515~546`). 판정과 무관한 그리기 문제
- `AM_SB_Combo_05_03`의 `Shot` 노티 5개 중 **1번만 각도 예외 10, 나머지 4개는 179**
- `BP_SBPlayer` 컴포넌트 19개 — `WeaponComponent`(Sword) + `GunWeaponComponent`(Gun) 2개 확인
- `DA_Weapon_Gun`: `WeaponStaticMesh=.../Gun`, `HandSocketName=Gun_Weapon_LSocket`, `SheathSocketName=Gun_Holster_RSocket`

## 남은 것

1. **`AM_SB_Combo_05_03` 첫 `Shot` 노티 각도 10 → 179 여부** — 첫 발만 정면 조준탄으로 둘지 결정 필요
2. **360°가 되면 한 적이 5번 맞는다** — 발당 데미지 분배를 정해야 함. `ShotRange` 500도 링 치고 멀다(250~350 검토)
3. **디버그 구체 그리기 미적용** — 각도 90° 이상이면 `DrawDebugSphere`로 대체하는 코드 조각은 나왔으나 아직 안 넣음
4. **일반 공격 자동 조준(미착수)** — 락온 안 걸었을 때 가장 가까운 적 쪽으로 자동 회전. `ULockOnComponent::FindBestTarget()`이 public이라 재사용 가능하고, `GA_PlayerMeleeAttackBase::OnActivated`의 락온 게이트만 바꾸면 된다. 미결 = 카메라 정면 기준(현재 `FindBestTarget` 동작, ±45°)이냐 스틱 입력 방향 기준이냐. 부수 결정 = 자동 조준 사거리(락온 반경 1000cm는 근접에 멂) / 즉시 스냅이 티 날지
5. `UKDPlayerAbilityInputComponent.cpp` 352줄 — §1 Component 300줄 선 초과. 분리 여부 미결
6. **미확정 관측** — `DA_Sword_Bandit` 등 적 정의 4개의 `PoiseDamageByAttack` 키가 비어 보인다(python 조회 한계일 수 있음). 에디터에서 직접 확인 필요

## 다음 세션 최우선

- `DA_ComboTree`의 `InputWindow` 26칸 값 채우기 — 작성자가 2026-08-12에 "나중에"로 보류 결정. 로직은 이미 있음
- 발사체 리팩토링 3건 (총 작업 종료로 착수 가능)
