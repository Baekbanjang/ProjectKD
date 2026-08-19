# 적 근접 판정 부활 — 무기 메시 태그 불일치

적 공격이 데미지를 한 번도 넣은 적이 없었다. 원인은 반경이 아니라 **트레이스 태스크가 생성조차 안 되던 것.**

**소스** — `GA_EnemyWeaponTraceBase.cpp` (1줄)
**에셋** — `BP_Axe_Attack_Basic` / `_Unblockable` / `DA_Sword_Bandit` / `DA_Parry_Bandit`
**앞 문서** — [2026-08-16-melee-radius-and-character-fit](2026-08-16-melee-radius-and-character-fit.md)

---

## 1. 증상과 첫 오진

```
증상   적이 공격 모션을 다 재생하는데 데미지 0
       적 GA 에 bDrawDebug 를 켜도 궤적이 한 개도 안 그려짐
```

**직전까지의 가설 = "판정 반경 3cm 라 프레임 사이로 빠진다".** 08-16 에 플레이어만 3 -> 20 으로 올렸고 적은 그대로였기 때문이다.

**틀렸다.** 반경은 `UAT_MeleeTrace` 안에서 쓰이는 값인데, **그 태스크가 아예 안 만들어지고 있었다.** 디버그 그리기도 그 태스크의 `TickTask` 안에 있어서 같이 죽었다.

> **궤적이 "가늘게" 나오는 것과 "아예 없는" 것은 다른 고장이다.** 없으면 반경을 의심하지 말고 태스크 생성 경로를 봐야 한다.

---

## 2. ★ 진짜 원인 — 무기를 컴포넌트 태그로 찾는다

`GA_MeleeTraceBase.cpp:127~137` 이 판정 시작 때 아바타의 메시 컴포넌트를 훑어 **태그가 일치하는 것 하나**를 고른다.

```cpp
if (Comp->ComponentHasTag(WeaponMeshComponentTag))   // 기본값 "Sword"
        TraceMesh = Cast<UMeshComponent>(Comp);
...
if (!IsValid(TraceMesh)) return;                     // :139
```

| | 무기 메시의 컴포넌트 태그 | GA 가 찾는 태그 | |
|---|---|---|---|
| 플레이어 | `Sword` / `Gun` | `Sword` | 일치 |
| 적 밴딧 | **`Weapon`** | `Sword` | **불일치** |
| 적 도끼 | **`Weapon`** | `Sword` | **불일치** |

### 왜 갈렸나 — 무기를 만드는 주체가 다르다

```
플레이어   UWeaponComponent 가 런타임에 메시를 스폰하며 태그를 붙인다
           WeaponComponent.cpp:184   WeaponMesh->ComponentTags.Add(WeaponComponentTag)
           BP_SBPlayer 인스턴스 값 = WeaponComponent "Sword" / GunWeaponComponent "Gun"

적         BP 에 Weapon 이라는 SkeletalMeshComponent 를 손으로 박아뒀고
           그 컴포넌트 태그가 "Weapon"
```

**플레이어는 무기가 둘이라 태그로 갈라야 한다**(GA 가 검으로 칠지 총으로 칠지 고름). 그래서 C++ 기본값이 `Sword` 다.
**적은 무기가 하나**라 `Weapon` 으로 박았고, 아무도 GA 쪽 값을 맞춰주지 않았다.

---

## 3. 고친 것 — 1줄

`GA_EnemyWeaponTraceBase` 생성자에 기본값을 적 기준으로 덮는다.

```cpp
// GA_EnemyWeaponTraceBase.cpp — 생성자
	// 적 무기 메시 컴포넌트 태그
	WeaponMeshComponentTag = TEXT("Weapon");
```

**BP 7개가 그대로 따라왔다.** 빌드 후 CDO 를 전수 조회해 `Weapon` 으로 읽히는 것을 확인했다 — BP 가 이 값을 명시로 직렬화한 적이 없어서, 에셋을 하나도 안 건드리고 끝났다.

> BP 가 덮고 있었다면 이 방법은 조용히 안 먹는다. **생성자 기본값을 바꿨으면 CDO 재조회가 필수다.**

---

## 4. 배제한 후보들 (같은 증상에서 다시 의심하지 말 것)

```
소켓 trace_base / trace_tip     밴딧 SKM_SsanggeomR · 도끼 SKM_ax_with_wooden_handle 둘 다 존재
bDrawDebug                      적 GA 7개 중 6개 true (Rush 만 false)
AttackMontage                   물려 있음
DamageEffectClass               GE_Damage_Physical 물려 있음
CapsuleRadius                   태스크가 안 생기므로 값이 쓰일 일 자체가 없었다
```

---

## 5. ★ 무음이 진짜 비용이었다

태그 불일치 자체는 1줄인데 몇 세션을 먹었다. **그 게이트가 아무 말도 안 하기 때문**이다.

```
GA_MeleeTraceBase.cpp:118   OwnerBody 소켓 없음      UE_LOG 주석 처리
                     :141   TraceMesh 못 찾음        UE_LOG 주석 처리   ← 이번 사건
                     :149   무기 소켓 없음           UE_LOG 주석 처리
```

세 관문이 전부 `return` 만 하고 끝난다. 화면에는 "공격 모션은 나오는데 판정이 없다"로만 보이고, 로그·에러·경고 어디에도 흔적이 없다.

**되살릴 때 주의** — 이 자리는 판정창마다 불린다. 그대로 켜면 로그가 도배된다. 켠다면 활성화당 1회 형태로.

---

## 6. 곁다리로 정리한 것

```
BP_Axe_Attack_Basic / _Unblockable   CapsuleRadius 3 -> 25
                                     ※ Rush · Sweep 은 원래부터 25 였다 (4개 다 3인 줄 알았음)
DA_Sword_Bandit / DA_Parry_Bandit    PoiseDamageByAttack 키
                                     Ability.Player.Light -> Ability.Player.Parry 복원
```

Poise 키는 패링이 미동작이던 시절 검증용으로 바꿔둔 것이다. 이제 되돌렸으므로 **포이즈를 깎는 경로는 다시 퍼펙트 패링뿐**이다.

---

## 7. ★ 이번에 뚫은 조회 방법 2개

### `GameplayTag` 키 이름을 읽는 법

`TMap<FGameplayTag, float>` 의 키가 파이썬에서 항상 비어 보인다(`dir()` 에 속성이 0개). 그동안 "조회 한계"로 미확정으로 남겨뒀던 것인데 **라이브러리 함수가 있었다.**

```python
unreal.GameplayTagLibrary.get_tag_name(tag)   # -> "Ability.Player.Parry"
```

`tag.tag_name` 은 없다(AttributeError). `unreal.BlueprintGameplayTagLibrary` 도 없다 — 이름은 `GameplayTagLibrary`.

### SCS 컴포넌트의 태그를 읽는 법

`inspect_cdo` 는 컴포넌트를 나열해주지만 **`ComponentTags` 를 안 준다**(`properties: {}`). "태그 없음"으로 오독하기 쉽다.
`Blueprint.simple_construction_script` 도 파이썬에 노출 안 됨. `CDO.get_components_by_class()` 에는 SCS 컴포넌트가 안 잡힌다.

**되는 길 = 템플릿 오브젝트를 직접 로드**한다.

```python
o = unreal.load_object(None, "/Game/.../BP_Bandit.BP_Bandit_C:Weapon_GEN_VARIABLE")
o.get_editor_property("component_tags")   # ['Weapon']
o.does_socket_exist("trace_base")         # True
```

이름 규칙 = `<BP경로>.<클래스명>:<컴포넌트명>_GEN_VARIABLE`

---

## 8. ★ 두 번째 무음 결함 — 퍼펙트 패링이 첫 한 번만 먹혔다

판정을 고치고 나니 포이즈가 **3 -> 2 로 한 번 깎이고 그 뒤로 안 깎였다.**

```
GA_PerfectParryReaction   순수 BP GameplayAbility / InstancedPerActor

ActivateAbilityFromEvent -> Send Gameplay Event to Actor -> Execute GameplayCue On Owner -> (끝)
ActivateAbility(태그)    -> Print String -> End Ability                    <- 이쪽엔 있다
```

**이벤트 경로에 `End Ability` 가 없다.** 인스턴스가 활성인 채로 남고 엔진이 두 번째 활성화를 거부한다 — `InternalTryActivateAbility` 의 `InstancedPerActor && Spec->IsActive() && !bRetriggerInstancedAbility` 갈래가 `return false`. **에러도 경고도 없다.**

### 증상이 헷갈렸던 이유 2개

1. **퍼펙트 패링 연출은 여러 겹이다.** 이 GA 담당은 `GameplayCue.Combat.PerfectParry.SlowMo` 하나뿐이라 두 번째부터는 그것만 빠지고 나머지는 그대로 뜬다 -> "성공했는데 안 깎인다"로 보인다
2. **홀드 중에는 퍼펙트 창이 다시 안 열린다** (`GA_Parry.cpp:48~57`, 진입 시 1회 0.15초). 별개의 정상 사양인데 증상이 겹쳐 보였다. 매번 키를 뗐다 다시 눌러야 한다

### 고친 것 = 노드 하나

`Execute GameplayCue On Owner` 의 `then` -> `End Ability`

`bRetriggerInstancedAbility = true` 로도 증상은 사라지지만 그건 "켜져 있으면 끝내고 다시 켠다"라 **살아남는 상태 자체는 그대로**다. 이 GA 는 이벤트 받고 보내고 큐 쏘면 끝나는 1회성이라 끝내는 게 맞다.

> **BP 로만 만든 GA 는 스스로 끝날 길이 없다.** C++ `UGA_ActionBase` 의 안전망 타이머를 안 타기 때문. 순수 BP GA 마다 확인할 것 = **모든 실행 경로가 `End Ability` 로 끝나는가.**

---

## 9. 검증 (PIE)

```
적 공격 디버그 궤적 표시              통과
적 공격 데미지                        통과   BP_Bandit3 Health 140 / 200 실측
퍼펙트 패링 -> 적 Poise 3 -> 2        통과   MCP 로 어트리뷰트 직접 조회
End Ability 수정 후 반복 발동          작성자 확인
```

**포이즈 체인 전 구간이 실측으로 확인됐다** — `AS_Combat::PostGameplayEffectExecute` -> `Event.Combat.PerfectParryTriggered` -> `GA_PerfectParryReaction` -> `Event.Combat.Hit`(`InstigatorTags = Ability.Player.Parry`) -> `KDEnemyBaseCharacter::OnHitReceived` -> DA 키 매칭 -> 차감.

### PIE 중 어트리뷰트를 직접 읽는 법

```python
w = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
asc = next(iter(actor.get_components_by_class(unreal.AbilitySystemComponent)))
for at in asc.get_all_attributes():
    v, ok = asc.get_gameplay_attribute_value(at)
```
`get_spawned_attributes` 는 없다. 쓸 수 있는 건 `get_all_attributes` / `get_attribute_set` / `get_gameplay_attribute_value` 셋뿐.
**"화면이 안 변한다"와 "값이 안 변한다"를 가르는 유일한 방법**이라 UI 관련 진단마다 쓴다.

---

## 10. ★ 세 번째 무음 결함 — 손에 든 무기가 적을 밀고 있었다

증상 = **플레이어가 적에게 몸을 비비면 뚝뚝거리다 살짝 떠오르고, 더 비비면 멀리 튕겨나감.** 적끼리는 멀쩡했다.

```
WeaponComponent.cpp:166~184   UStaticMeshComponent 를 NewObject 로 생성
                              기본 콜리전 프로파일 = BlockAllDynamic (Pawn 포함 전부 Block)
                              ComponentTags 붙이고 RegisterComponent — 콜리전 끄는 줄이 없다
```

검은 손에 **부착**돼 있어 위치가 강제된다. 밀 수 없으니 맞은 쪽(적)이 밀려나고, 129cm 날이 콤보로 훑으면 멀리 날아간다.

**고침 = 1줄.** `RegisterComponent()` 앞에 `SetCollisionEnabled(ECollisionEnabled::NoCollision)`.

### 왜 안전한가

무기 판정은 무기 콜리전을 **안 쓴다.** `UAT_MeleeTrace` 가 `trace_base`/`trace_tip` 소켓 좌표로 직접 캡슐 스윕을 돈다.
증거 = **적 무기는 BP 에서 이미 `NoCollision` 인데 적 근접 판정이 정상 동작한다.** 플레이어를 같은 조건으로 맞춘 것뿐이다.

### 진단이 두 번 빗나갔다 — 순서 기록

```
1차 가설  적끼리 겹쳐서 depenetration 이 수직으로 민다
          -> 틀림. 작성자 테스트 = "적끼리는 정상"
2차 가설  플레이어 루트모션이 적 캡슐로 파고든다 -> 플레이어 캡슐 Pawn 응답을 Overlap 으로
          -> 틀림. 원인은 캡슐이 아니라 손에 든 무기
확정      작성자가 "검·총 때문인 것 같다" 고 짚었고 코드에 콜리전 끄는 줄이 없는 것으로 확인
```

**CDO 조회로 안 보였던 이유** = 플레이어 무기는 **런타임 스폰**이라 에디터에 대상이 없다. 적 무기는 BP SCS 라 보였고, 그래서 "무기는 `NoCollision` 이네" 하고 **적 값을 보고 플레이어도 그럴 거라 넘겨짚었다.**

> **런타임에 만드는 컴포넌트는 BP 조회에 안 잡힌다.** 코드가 생성하는 컴포넌트는 기본값이 그대로 살아 있는지 `NewObject` 자리를 직접 봐야 한다.

---

## 11. 남은 것

```
스태거 -> 처형 구간                         Poise 0 도달이 처음이라 아직 안 타봤다
BP_Axe_Attack_Rush 의 bDrawDebug 만 false   나머지 6개와 다름. 의도인지 누락인지 미확인
무음 게이트 3곳                             로그 되살릴지 판단 필요
MaxPoise 3 = 퍼펙트 패링 3회                일반 가드는 이벤트 경로가 없어 진전이 0. 밸런스 판단
반경 20 / 25 체감                           숫자만 맞췄고 실플레이 판단 안 함
반경 20 / 25 체감                           숫자만 맞췄고 실플레이 판단 안 함
```
