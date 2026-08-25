# 2026-08-25 — B2: `OnHitReceived` 4분해

리팩토링 이월분 B2. 적 피격 처리 135줄 한 함수를 SB `ResultTable` 축(인지/연출/수치/이동)으로 나눴다.

**대상** — `AKDEnemyBaseCharacter` (`Enemy/KDEnemyBaseCharacter.h` / `.cpp`)
**직전 작업** = `2026-08-25-refactor-dedup-deadcode.md` (A·E·B1)

---

## 1. 왜 나눴나

`OnHitReceived`(구 `:425~554`) 안에 네 가지가 섞여 있었다.

```
인지   UAISense_Damage::ReportDamageEvent      AI 가 등 뒤 공격을 인지
연출   TriggerBoneShake + ExecuteGameplayCue   흔들림 + 타격 큐
수치   PoiseDamageByAttack 순회 -> Poise 차감   경직 게이지
이동   StopMovement + PauseLogic + LaunchCharacter
```

네 갈래가 각자 다른 조건을 보는데 그 조건들이 `if` 중첩과 `return` 두 개로 얽혀 있었다. 하나를 고치면 나머지 셋이 흔들리는 구조.

## 2. 나눈 결과

```
:425  OnHitReceived            33줄   네 갈래로 분배만
:459  ReportHitToPerception    12줄   인지
:472  PlayHitFeedback          13줄   연출
:486  ApplyPoiseDamage         30줄   수치 — bool 반환
:517  ApplyKnockback           71줄   이동
```

`ApplyKnockback` 은 바깥 `if` 한 겹만 벗기고 본문은 글자 그대로 옮겼다. 디버그 블록·람다·폴백 포함.

## 3. ★ 핵심 — 숨은 연결을 반환값으로 올렸다

구 코드의 넉백 직전에 `IsStaggered()` 재조회가 있었다. 위쪽 `:458` 에서 이미 경직이면 `return` 하는데 왜 또 보는지, 그 줄만 봐선 알 수 없었다.

이유는 **그 사이에 값이 바뀌기 때문**이다.

```
SetNumericAttributeBase(Poise, 0)
  -> UStaggerComponent::OnPoiseChanged   (StaggerComponent.cpp:39)
  -> BeginStagger()                       다음 줄 전에 이미 경직 상태
```

어트리뷰트에 값을 쓰면 델리게이트가 **그 줄에서 동기로** 터진다. Poise 를 깎다가 0 이 되면 그 자리에서 경직에 들어간다.

→ `ApplyPoiseDamage` 가 `bool` 을 반환하게 해서 호출부에 이유를 적었다.

```cpp
// Poise 차감이 경직을 유발하면 넉백 X
if (ApplyPoiseDamage(Payload))
{
    return;
}
```

> ⚠️ 설계 중 이 줄을 "항상 false 인 죽은 조건"으로 한 번 오판했다. `:458` 에서 return 하니 아래는 안전하다고 봤는데, 중간의 `SetNumericAttributeBase` 가 델리게이트를 터뜨린다는 걸 놓쳤다. **어트리뷰트 쓰기는 그 자리에서 남의 코드를 부른다.**

## 4. 겸사 — `IsStaggered()` 로 통일

구 코드는 `StaggerComp && StaggerComp->IsStaggered()` 를 두 곳에서 각자 썼다. 헤더 `:51` 에 같은 식의 함수가 이미 있어 그쪽으로 합쳤다.

```cpp
bool AKDEnemyBaseCharacter::IsStaggered() const { return StaggerComp && StaggerComp->IsStaggered(); }
```

## 5. 줄 수는 늘었다

```
622줄  ->  655줄   (+33)
```

함수 시그니처 4개 + `// 기능 :` 주석만큼 늘었다. §1 의 500줄 한도로 보면 나빠졌다.

의도한 거래 — 파일 총량 대신 **함수 하나당 크기**를 줄였고, `ApplyKnockback` 71줄이 **덩어리째 떼어낼 수 있는 모양**이 됐다. C2 에서 `UKnockbackComponent` 로 나가면 655 -> 약 585. **지금은 손해고 C2 까지 가야 회수된다.**

## 6. 검증 (PIE)

동작이 안 바뀌는 게 정상. 전부 통과.

```
평타로 때리기            밀림 + 흔들림
연타로 Poise 0           경직 진입하는 그 타는 밀리지 X   <- ApplyPoiseDamage 반환값
경직된 적 계속 때리기     흔들림·이펙트만
등 뒤에서 때리기         적이 돌아본다                    <- ReportHitToPerception
```

두 번째가 이번 분해의 핵심 검증이다.

## 7. 남은 것

```
C2   ApplyKnockback -> UKnockbackComponent 추출 + LaunchCharacter 를 RootMotionSource 로
     LaunchCharacter 는 속도를 주는 함수라 "몇 cm 밀린다"를 못 정한다.
     기본값 400 이 계산상 17cm 였던 것도 그래서 늦게 발견됐다(08-20 에 1800 으로 상향)
```

**미해결** — 적이 화살을 막을 때 뼈 흔들림이 뜨는지 여부. `PlayHitFeedback` 이 이제 C++ 유일 호출처라, 여기 로그 한 줄이면 C++ 경로인지 BP 경로인지 갈린다. `TriggerBoneShake` / `TriggerBoneShakeParams` 둘 다 `BlueprintCallable` 이고 `HitFeedbackComponent.h:8` 주석은 "GameplayCue 가 호출"이라고 적고 있다.
