# 2026-08-26 — Poise 2단계: 커스텀 EffectContext 로 타별 계수 통로

타격마다 Poise 차감을 다르게 하려면 GA 에서 적까지 숫자 하나를 더 보내야 하는데 **통로가 막혀 있었다.** `FGameplayEffectContext` 를 서브클래싱해 뚫었다.

**신규** — `AbilitySystem/Context/KDGameplayEffectContext.h/.cpp` · `AbilitySystem/Globals/KDAbilitySystemGlobals.h/.cpp`
**직전 작업** = `2026-08-26-refactor-c3-c4-c5.md`

---

## 1. 문제 — 실을 자리가 없었다

```
FGameplayEventData.EventMagnitude   float 하나 — 넉백 배수가 이미 쓰는 중
```

`Event.Combat.Hit` 이 나르는 숫자는 이거 하나뿐이라 두 번째 값을 넣을 데가 없었다.

### 검토한 대안 — 태그 방식

노드가 `Attack.Poise.Heavy` 같은 **등급 태그**를 실어 보내고, 적 DA 가 그 태그를 자기 값으로 해석하는 안. 코드 변경이 훨씬 적고 적 쪽은 0줄이었다(`PoiseDamageByAttack` 이 이미 태그 -> float 맵이라).

**기각 이유 = 방향 결정.** 승환이 "타격마다 세밀하게" 쪽을 택했다. 등급 태그는 이산값만 보낼 수 있고, 앞으로 실을 후보가 3~4개 더 보인다.

```
지금       PoiseDamage
후보       실드 계수 (현재 AS_Combat.cpp:102 에 0.4 하드코딩. SB 는 스킬마다 0.4~10.4)
           히트스톱 시간 (현재 GA 멤버 고정. 마무리타만 길게 하려면)
           콤보 단계 인덱스 (몇 타째냐로 카메라·이펙트 분기)
```

## 2. 왜 Context 가 맞나 — 두 경로를 동시에 탄다

```
ApplyDamageEffect  ->  GE Spec 에 실림             ->  AS_Combat::PostGameplayEffectExecute
SendHitEvent       ->  HitEvent.ContextHandle       ->  적 Pawn
```

`EventMagnitude` 는 이벤트 경로에만 있는데 **Context 는 데미지 계산 쪽과 반응 쪽이 같은 것을 본다.** 값이 갈릴 일이 없다.

## 3. 구성

```
FKDGameplayEffectContext : FGameplayEffectContext
    float PoiseMultiplier
    Get / GetMutable        캐스팅 지점 단일화 — 타입 검사 후에만 static_cast
    GetScriptStruct         우리 타입 반환. Get 의 판별 기준
    Duplicate               스펙 복사 시 값 유지
    NetSerialize            부모 직렬화 후 우리 필드 이어붙이기

UKDAbilitySystemGlobals : UAbilitySystemGlobals
    AllocGameplayEffectContext  ->  new FKDGameplayEffectContext()

Config/DefaultGame.ini:12
    AbilitySystemGlobalsClassName = /Script/Project_KD.KDAbilitySystemGlobals
```

ini 는 **새 섹션을 만들지 않고 기존 줄을 교체**했다. 되돌리려면 그 줄만 `/Script/GameplayAbilities.AbilitySystemGlobals` 로.

## 4. ★ 작업 중 나온 함정 3개

### 4-1. `Duplicate()` 를 빼면 값이 조용히 증발

GE Spec 이 복사될 때 엔진이 `Duplicate()` 를 부른다. 오버라이드가 없으면 **부모 것이 불려 부모 타입 객체가 생성**되고 `PoiseMultiplier` 가 사라진다. 에러도 로그도 없다.

그리고 `HitResult` 는 부모가 `TSharedPtr` 로 들고 있어(엔진 `GameplayEffectTypes.h:122`) 복사 생성자만으로는 **원본과 같은 것을 가리킨다.** `AddHitResult(*GetHitResult(), true)` 로 별도 인스턴스를 만들어야 한다.

### 4-2. 타입 검사 대상을 부모로 쓰면 항상 nullptr

```cpp
if (Base->GetScriptStruct() == FGameplayEffectContext::StaticStruct())     // 항상 false
if (Base->GetScriptStruct() == FKDGameplayEffectContext::StaticStruct())   // 맞다
```

우리 `GetScriptStruct()` 는 자식 타입을 돌려주므로 부모와 절대 같지 않다. **빌드는 통과하고 값만 안 먹는다.**

### 4-3. `AllocGameplayEffectContext` 에서 `Super::` 를 부르면 교체가 무효

```cpp
return Super::AllocGameplayEffectContext();   // 부모 타입이 나온다
return new FKDGameplayEffectContext();        // 맞다
```

셋 다 **빌드가 통과하는 조용한 실패**다. 위 4-2 · 4-3 은 실제로 작성 중 나왔고 리뷰에서 잡았다.

## 5. 배선 — 기존 계수 둘과 같은 모양

```
ComboTreeDataAsset.h        float PoiseMultiplier = 0.f          0 = 값 없음
GA_MeleeTraceBase.h         float PoiseMultiplier = 1.f          런타임 멤버(UPROPERTY X)
GA_PlayerAttackBase.h       DefaultPoiseMultiplier = 1.f         지상 GA 기본
GA_PlayerAirAttackBase.h    DefaultAirPoiseMultiplier = 1.f      공중 GA 기본
ApplyComboNode              인자 6개로 — 디폴트 복원 후 노드가 0 초과면 덮어씀
KDAbilityStatics            ApplyDamageEffect 에 인자 추가 + Context 적재
GA_ShotBlast                상수 1.f (UGA_ActionBase 자식이라 그 멤버가 없다)
KDEnemyBaseCharacter        ApplyPoiseDamage 가 Get 으로 읽어 곱셈
```

### 최종 계산식

```
Poise 차감 = DA 태그 합산값  x  노드 배수
             (적 체급)          (타격 세기)
```

C2 의 넉백(`DA 거리 x 배수`)과 같은 패턴이다.

## 6. 검증

```
빌드                통과
1단계 PIE           Context 교체만 넣고 확인 — 패링 3종 · 이펙트 위치 · 넉백 · 처형 이상 없음
2단계 빌드          배선 후 통과
```

**노드 값은 안 채웠다.** 26노드 전부 0 이라 배수가 전부 1.0 으로 떨어지고, 동작은 종전과 같다. **통로만 뚫은 작업이다.**

> ⚠️ **미검증 — 통로 관통 확인.** `DA_ComboTree` 노드 하나에 `PoiseMultiplier = 3.0` 을 넣고 그 타의 Poise 칸이 3배로 깎이는지 봐야 한다. 값이 전부 1.0 이라 **먹는지 안 먹는지 지금은 구분이 안 된다.** 값 채우기(C1) 전에 반드시 확인할 것.

## 7. 남은 것

```
C1                    콤보 노드 값 채우기 — InputWindow · DamageMultiplier
                      · KnockbackMultiplier · PoiseMultiplier 4칸이 전부 0
통로 관통 확인          위 §6 경고
Context 추가 필드       실드 계수 · 히트스톱 시간 · 콤보 단계 (필요해질 때)
```

📌 **`DefaultGame.ini:12` 는 문자열 클래스 경로다.** 클래스 개명 작업 시 CoreRedirect 가 안 먹는 자리이므로 손으로 맞춰야 한다.
