# 범위 판정 GA + 스킬 몽타주 연출 배선 (2026-09-02 오후)

09-02 오전에 만든 스킬 4종 위에 **땅 찍기 AoE**를 얹고, 스킬 몽타주 4개의 연출을 콤보 표준에 맞췄다.

---

## 1. 신규 클래스 1개

```
UKDGameplayAbility_AreaBlast : UKDGameplayAbility     범위 판정 GA
```

`GA_ShotBlast`와 **같은 층위의 독립 GA**다. 스킬 GA(`GA_Skill_02`)는 이 일을 모른다 — 두 GA가 같은 몽타주 위에서 각자 돈다.

```
AM_SB_Skill_02 f≈60 (1.008초)
  → AN_SendGameplayEvent → Event.Montage.AreaBlast
  → GA_AreaBlast_GroundSlam → 원기둥 판정 1회 → 즉시 종료
```

**부모를 안 건드린 이유** — `UKDGameplayAbility_PlayerMelee`는 판정을 무기 소켓 스윕(`UKDAbilityTask_MeleeTrace`)으로만 한다. 원기둥 판정을 거기 넣으면 **콤보 GA 20개가 안 쓰는 코드를 물려받는다.** 09-02 오전 차지 스킬이 부모를 0줄 건드린 것과 같은 판단이다.

신규 태그 2개 — `Ability.Player.AreaBlast` · `Event.Montage.AreaBlast`.

---

## 2. ★ 노티를 새로 만들 뻔했다 — 이미 있었다

C++ 노티 `UKDAnimNotify_SendGameplayEvent`를 설계까지 마쳤는데, **범용 BP가 이미 존재했다.**

```
/Game/Blueprints/AnimNotifies/AN_SendGameplayEvent
  변수   EventTag : GameplayTag  (public)
  그래프  Get Owner → Get ASC → Is Valid → Send Gameplay Event to Actor
```

내가 설계한 C++판과 **변수도 동작도 같다.**

🔴 **`KDGameplayTags.h:119` 주석이 그 이름을 이미 부르고 있었다.**

```cpp
// 돌진 재조준 — 런지 윈도우 시작 시 AN_SendGameplayEvent가 발화, ...
UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Rush_Warp)
```

읽고도 못 잡았다. 원인 = **기존 클래스 검색을 `Source/`에만 하고 `Content/`에는 안 했다.** `Event_Rush_Warp` 를 grep 했을 때 "C++ 발신처 0건"이 나왔는데, 그게 곧 "BP가 쏜다"는 신호였다.

📌 §1 "새 클래스 제안 전 같은 역할의 기존 클래스 검색 의무" 는 **에셋까지 포함해야 한다.**

---

## 3. 전용 노티와 범용 노티를 가르는 것 = `OptionalObject`

```cpp
// KDAnimNotify_ShotBlast.cpp:24
Payload.OptionalObject = this;              // 노티가 자기 자신을 넘긴다
// KDGameplayAbility_ShotBlast.cpp:52
Cast<UKDAnimNotify_ShotBlast>(TriggerEventData->OptionalObject);   // GA 가 캐스팅해 값을 읽는다
```

**캐스팅할 타입이 필요해서 전용 클래스가 존재한다.** `ShotBlast`는 노티마다 각도·배수 오버라이드가 7개라 값이 살 집이 필요했다(총격 20발이 몽타주마다 조건이 다름).

`AreaBlast`는 오버라이드가 0개다 — 값이 전부 GA BP에 있다. 그래서 태그만 고르는 범용으로 충분하다.

---

## 4. 판정 — 락온 컴포넌트를 안 건드린 이유

원기둥 수학은 `KDLockOnComponent.cpp:284~306`에서 가져왔다(박스 오버랩 → 2D 반경 → 높이 띠).

```cpp
OverlapMultiByObjectType(..., MakeBox(Radius, Radius, HalfHeight))   // 넓게 줍고
if (DistSquared2D(Origin, TargetLoc) > RadiusSq) continue;           // 모서리를 깎아 원기둥
```

`GatherCandidates`를 직접 부르지 않은 이유 둘:
```
① private 이고 반환이 단일 타겟 (AoE 는 배열이 필요)
② 그 함수가 "락온 받을 자격"(CanBeTargeted)을 검사한다 — 데미지 판정에 섞이면 안 된다
```
게다가 `UKDLockOnComponent`는 552줄로 이미 300선을 넘겨 있다. 공용 API를 하나 더 얹는 건 반대 방향이다.

`ShotBlast::GatherTargets`와 20줄쯤 겹친다. **세 번째 히트스캔 GA가 생길 때 `UKDAbilityStatics`로 뽑는다** — 지금 뽑으면 `ShotBlast`의 미완 구조(`PoiseMultiplier` 1.f 하드코딩)가 굳는다.

### 부채꼴 판정 — 내적으로 각도를 재는 법

```cpp
Dot(Basis, ToTarget) = cos(내 정면과 적 방향 사이 각도)
각도 > HalfAngle   ==   Dot < cos(HalfAngle)      // cos 가 감소함수라 부등호가 뒤집힌다
```
`acos()`를 안 쓰는 이유 = 코사인끼리 비교하면 곱셈 3번이다.

⚠️ **`HalfAngle`은 반각이다** — `90`이면 부채꼴 전체가 **180도**. `FKDTargetFilter.h:47` 클램프가 `5~135`라 최대 270도고, 360도는 `Cylinder`를 쓴다. 도형 enum이 둘로 갈린 이유가 이것.
⚠️ `GetSafeNormal2D()`가 Z를 버리므로 **높이는 각도 판정에 안 낀다.** 머리 위 적도 각도상 "정면"이다.

### LoS 는 안 본다 (승환 결정)

트레이스가 ①벽 판정 ②`ImpactPoint` 획득 두 일을 했는데, ①을 버리니 블록이 통째로 사라졌다. 타격 위치가 적 캡슐 표면 → 적 중심으로 바뀌고, 적당 라인트레이스 1회가 없어진다.

📌 벽에 막히는 범위기가 실제로 필요해지면 그때 `bRequireLineOfSight` 노브를 붙인다.

---

## 5. 방사형 넉백은 코드 0줄

```cpp
// KDKnockbackComponent.cpp:46
Dir = (OwnerChar->GetActorLocation() - Payload.Instigator->GetActorLocation()).GetSafeNormal2D();
```

피격자 위치에서 **공격자 위치를 뺀다.** 공격자가 원기둥 중심이므로 적마다 바깥으로 밀린다. `SendHitEvent`에 배수만 넘기면 끝이다.

경직도 같은 구조 — `KDEnemyBaseCharacter.cpp:527`이 `InstigatorTags.HasTag(Key)`로 맵을 뒤진다. GA의 `AssetTags`가 곧 키다.

⚠️ **적 DA 5개에 `Ability.Player.AreaBlast` 키가 없다 = 경직이 에러 없이 안 걸린다.** `Skill1~4` · `CounterSlash`도 같은 상태. 적 밸런싱 때 한꺼번에.

---

## 6. ★ 빌드 함정 2개

### `TObjectPtr` 를 `TWeakObjectPtr` 에 대입하면 C2679

```cpp
CueParams.SourceObject = HitConfirmProfile;          // 실패
CueParams.SourceObject = HitConfirmProfile.Get();    // 통과
```

`WeakObjectPtrTemplates.h:100~109`:
```cpp
template <typename U UE_REQUIRES(!TLosesQualifiersFromTo_V<U, T>)>
FORCEINLINE TWeakObjectPtr& operator=(U* Object)     // 받는 건 U* — 생 포인터
```

**템플릿 인수 추론은 사용자 정의 변환을 시도하지 않는다.** 추론이 먼저고 변환이 나중이라 `TObjectPtr`에서 `U`를 못 정한다. `TObjectPtr`를 받는 오버로드는 엔진에 없다.

이어서 **C2440**이 났는데 원인이 달랐다 — `.h`를 전방 선언으로 바꾸고 `.cpp`에 include를 안 넣었다. 업캐스팅에는 상속 관계를 알아야 하는데 전방 선언은 이름만 알려준다. **전방 선언(`.h`)과 include(`.cpp`)는 항상 짝이다.**

⚠️ 같은 줄이 `KDGameplayAbility_PlayerMelee.cpp:49` · `KDGameplayAbility_ShotBlast.cpp:240`에도 `.Get()` 없이 있다. **이번 증분 빌드가 그 둘을 재컴파일하지 않아 안 걸렸다** — 그 파일을 건드리면 터질 것으로 본다(예측, 전체 리빌드로만 확정).

### 🔴 `Build.bat` 은 컴파일 에러에도 exit 0 을 준다

```
error C2679 1건 + Result: Failed (OtherCompilationError)  인데  [exited with code 0]
```

**exit code 로 빌드 성공을 판정하면 안 된다.** 판정 기준 = 로그의 `Result: Failed` 문자열 또는 `error` 카운트.

---

## 7. 스킬 몽타주 연출 — 콤보 표준에 맞춤

콤보 20개 전수 조회로 표준 트랙을 확정했다.

```
지상 콤보 20개 공통
MeleeTrace · CancelWindow · MovementCancel · WeaponAttach
Sound_Swing · WeaponTrail · Shot · Sound_Shot · Muzzle · Warp
```

스킬에 넣은 것 / 뺀 것:
```
넣음   WeaponAttach · Sound_Swing · WeaponTrail · Sound_Shot · Muzzle
뺌     CancelWindow · MovementCancel    스킬 확정사양 = 캔슬 없음
       Warp                             ApproachWarpName = None (워프 타겟을 GA 가 안 만든다)
```

간격은 콤보 5개에서 전부 동일하게 나온 값을 그대로 썼다.
```
WeaponTrail   MeleeTrace − 0.05,  dur = melee.dur + 0.1333
Sound_Swing   MeleeTrace − 0.033
Muzzle        Shot − 0.05,  dur 0.10
Sound_Shot    Shot 과 동시
```

| 몽타주 | 노티 | 비고 |
|---|---|---|
| `AM_SB_Skill_01` | 6 → 19 | 총 3발 |
| `AM_SB_Skill_02` | 4 → 11 | 총 없음 · `GroundBlast` 트랙 |
| `AM_SB_Skill_03` | 12 → 37 | 총 6발 |
| `AM_SB_Skill_04` | 1 → 4 | 총 없음 |

### ★ 총격 1발 = 3노티 세트가 깨져 있었다

```
표준        Shot + Sound_Shot + Muzzle       (08-30 dev-log)
작업 전     Skill_01 Shot 3개만 / Skill_03 Shot 6개만
            -> 총성 X · 총구 섬광 X
```
검 쪽도 4개 몽타주 전부 `WeaponTrail`·`Sound_Swing`이 0개였다. **판정만 있고 궤적도 소리도 없었다.**

### 🔴 `Muzzle` 이 `WeaponTrail` 클래스인 이유

```cpp
// GameplayCueNotifyTypes.cpp:337   UE5.6
TargetComponent = (TargetCharacter ? TargetCharacter->GetMesh() : TargetActor->GetRootComponent());
```

**GameplayCue 의 소켓 부착은 캐릭터 메시(없으면 루트)만 본다.** 총은 별도 메시라 그 `Muzzle` 소켓이 GC 의 시야 밖이다. 무기 메시를 컴포넌트 태그로 찾는 `KDAnimNotifyState_WeaponTrail`이 **유일한 경로**다(08-17 결정).

이름만 "Trail"이지 하는 일은 **"무기 소켓에 나이아가라 붙였다 떼기"**다. 검격이 첫 용도라 그 이름이 붙었을 뿐.

⚠️ **그 대가를 이번에 치렀다.** 클래스 기본값이 `Sword`/`Sword_Bottom`이라, `niagara_system`과 `socket_name`만 바꾸고 `weapon_mesh_component_tag`를 빠뜨렸다. 검 메시에서 `Muzzle` 소켓을 찾다 실패 — **에러도 로그도 없다.** Muzzle 9개를 `Gun`으로 고쳤다.

```
Muzzle 노티는 세 값이 한 세트
  niagara_system              NS_SB_Hit_Fire_Once
  socket_name                 Muzzle
  weapon_mesh_component_tag   Gun        ← 기본값이 Sword
```

📌 근본 해결안 = 기본값을 `NAME_None`(캐릭터 몸)으로. 그러면 안 넣었을 때 **조용히 틀리는 대신 안 뜬다.** 다만 기존 검 트레일 27개가 기본값에 기대고 있을 수 있어 전수 확인이 먼저다.

### 🔴 `Shot` 을 옮기면 `Muzzle` 은 안 따라온다

승환이 `Skill_03` 첫 총격을 `0.1731 → 0.2000`으로 옮겼는데 `Shot`·`Sound_Shot`만 따라가고 `Muzzle`이 남았다. **타임라인에 검 궤적과 똑같이 "WeaponTrail"로 뜨니 못 찾는다.**

노티 시각을 바꾸는 API가 없어서 **트랙을 통째로 다시 깔았다.**
```python
AL.remove_animation_notify_events_by_track(m, "Muzzle")
# Shot 시각을 읽어 Muzzle = Shot − 0.05, Sound_Shot = Shot 으로 재생성
```
트랙 단위라 다른 노티를 안 건드리고, **멱등이라 몇 번 돌려도 안전하다.** `Shot` 을 옮길 때마다 재실행하면 된다.

---

## 8. MCP 조회법 (재사용)

```python
AL = unreal.AnimationLibrary
AL.get_animation_notify_track_names(m)          # 트랙 이름 목록
AL.get_animation_notify_events_for_track(m, t)  # 트랙별 노티
AL.add_animation_notify_track(m, name, LinearColor)
AL.add_animation_notify_state_event(m, track, start, duration, cls)   # 반환 = 노티 객체
AL.add_animation_notify_event(m, track, start, cls)
AL.remove_animation_notify_events_by_track(m, track)
```

```
m.export_text()                       AnimMontage 엔 없다 (AttributeError)
m.get_editor_property("notifies")     protected
m.get_editor_property("anim_notify_tracks")   Failed to find property
FAnimNotifyEvent.export_text()        ★된다. LinkValue / Duration / TrackIndex
```
⚠️ `LinkValue` 는 두 번 나온다 — **앞이 끝, 뒤가 시작**(08-20 실측 재확인). `TrackIndex` 는 export 전문에 있다(앞 600자만 보면 안 보인다).

### 🔴 에디터가 한 번 튕겼다

24개를 한 번에 넣고 저장 전에 크래시 — **파일럿(2개)만 살고 나머지 3개는 통째로 날아갔다.** 다시 할 때 **한 몽타주씩 넣고 즉시 저장**했더니 네 번 다 통과했다.

⚠️ **원인은 확정 못 했다** — 배치량인지, 저장을 안 해서인지, MCP 브리지인지 갈리지 않았다. 당분간 **10개 단위 + 즉시 저장**으로 간다. 재현되면 그때 좁힌다.

📌 `EditorAssetLibrary.save_asset(path, False)` 는 이번엔 정상 동작했다(`git status` 로 디스크 확인). 09-01 의 "조용히 안 쓰는 함정"은 재현되지 않았다.

---

## 검증

- [x] 빌드 `Result: Succeeded` 에러 0 · 경고 0
- [x] PIE 통과 (승환) — 땅 찍기 발동 · 범위 판정 · 넉백 · 히트스톱
- [x] `GA_ShotBlast` CDO 대조 — 활성 태그 3종 · Cooldown · Cost 전부 비어 있음 (같은 모양)
- [x] 노티 44개 배치 후 전수 재조회
- [x] Muzzle 9개 `tag=Gun` / `socket=Muzzle` 콤보와 일치 확인
- [ ] `Skill_03` 소리 밀도 — 0.9초에 검격 6 + 총성 6. 귀로 판정 남음
- [ ] `Skill_03` 트레일 겹침 — 궤적 6개가 서로 물린다. 하나로 합칠지 눈으로 판정
- [ ] `bDrawDebug` 끄기 (검증용으로 켜둔 상태)

## 🔴 BP 셋업 함정 2개 (실측 확인)

`GA_Skill_02` CDO 실측 = `ActivationOwnedTags`에 `State.Combat.Attacking`, `CooldownGameplayEffectClass`에 `GE_SkillCooldown`.

```
CooldownGameplayEffectClass    비울 것
   스킬이 이미 Cooldown.Player.Skill 을 부여했다 -> CheckCooldown 실패
ActivationBlockedTags          비울 것
   스킬이 State.Combat.Attacking 을 들고 있다 -> 영원히 차단
```
둘 다 `CommitAbility` 에서 막혀 **에러 없이 조용히 안 나간다.** `GA_ShotBlast`가 정확히 이 이유로 셋 다 비어 있다.

## 커밋

```
코드      bcd5a2e  [GAS] 범위 판정 GA - UKDGameplayAbility_AreaBlast
         6ae6d3a  [doc] PROJECT_OVERVIEW 전면 갱신
Content  037218b  [Anim] 스킬 몽타주 4개에 표준 트랙 + 연출 노티 44개
         68d1b4f  [Anim] Skill_03 첫 총격 f12 이동 + Muzzle 재동기화
         5bd49df  [fix] 스킬 Muzzle 노티 9개의 무기 태그 Sword -> Gun
```

## 남은 것

```
스킬 연출        스킬 전용 HitConfirmProfile · 슬로모
                ⚠️ AreaBlast 는 PlayerMelee 자식이 아니라 슬로모 칸 3개가 없다
우하단 스킬 UI    SB식 슬롯
PoiseDamage      적 DA 5개에 Skill1~4 · CounterSlash · AreaBlast 키
SourceObject     PlayerMelee.cpp:49 · ShotBlast.cpp:240 에 .Get() 미적용
StartupEffects   BP_PlayerState 배열 마지막 항목이 None (빈 칸)
노티 표시명       Muzzle 과 WeaponTrail 이 타임라인에 같은 이름으로 뜬다
```
