# 2026-08-25 — C2: 넉백을 거리(cm) 기반 컴포넌트로

리팩토링 이월분 C2. `LaunchCharacter`(속도) 를 `RootMotionSource`(거리) 로 바꾸고, 넉백을 `UKnockbackComponent` 로 분리했다.

**신규** — `Combat/KnockbackComponent.h` / `.cpp`
**직전 작업** = `2026-08-25-refactor-b2-onhitreceived.md` (B2 가 이 분리의 길을 닦았다)

---

## 1. 왜 바꿨나 — 거리를 못 정하는 구조였다

```cpp
LaunchCharacter(Dir * KnockbackStrength, true, false);   // 속도(cm/s) 를 주는 함수
```

`LaunchCharacter` 는 초기 속도만 준다. 얼마나 가서 멈추는지는 `CharacterMovementComponent` 의 마찰·제동이 정한다. **"몇 cm 밀린다"를 값으로 표현할 방법이 없었다.**

실제 비용 — 기본값 `400` 이 계산상 17cm 라 사실상 안 밀리는 값이었고, 08-20 에 1800 으로 올려서야 밀림이 보였다. 그때까지 아무도 몰랐던 이유가 "숫자를 봐도 거리를 알 수 없어서"다.

SB 는 넉백을 cm 단위로 정의한다(볼트 `reference_sb_movement_impact`).

## 2. 바뀐 것

```
전   EnemyDefinition.KnockbackStrength   1800   cm/s (속도)
후   EnemyDefinition.KnockbackDistance     60   cm   (거리)
```

```cpp
// KnockbackComponent.cpp
Source->StartLocation  = Start;
Source->TargetLocation = Start + Dir * Distance;   // Dir 은 길이 1 -> Distance 가 그대로 월드 cm
Source->Duration       = KnockbackDuration;
Source->AccumulateMode = ERootMotionAccumulateMode::Override;
```

### ⚠️ 리다이렉트를 일부러 안 걸었다

`KnockbackStrength` -> `KnockbackDistance` 는 이름만 바뀐 게 아니라 **뜻이 바뀐다.** `+PropertyRedirects` 로 값을 이어주면 1800 이 거리로 넘어와 **에러도 경고도 없이 18미터 날아간다.** 값이 버려지는 편이 낫다.

`DefaultEngine.ini` 에 관련 리다이렉트가 안 생겼는지 확인했다(0건).

## 3. 컴포넌트 경계 — brain 은 Pawn 이 소유

```
UKnockbackComponent      방향 계산 + RootMotionSource 적용 + 디버그 표시
AKDEnemyBaseCharacter    brain 정지 (StopMovement + PauseLogic + 타이머)
                         통신 = OnKnockbackBegin 델리게이트
```

`StaggerComponent.h:18` 의 선례를 따랐다 — *"Pawn 액추에이션(brain/movement)은 델리게이트 위임"*.

**이유는 brain 이 이미 Pawn 소유라서다.** 경직도 `OnStaggerBegin` -> Pawn 이 brain 을 멈춘다. 컴포넌트가 직접 만지면 경직과 넉백이 각자 타이머로 같은 brain 을 재개하게 되고, 재개 순서가 꼬인다.

§1-3 의존성 방향도 지켜진다 — 컴포넌트가 보는 건 `ACharacter` · `UCharacterMovementComponent` 뿐이고, 구체 Pawn 캐스팅은 0개. `EnemyDefinition` 값은 **Pawn 이 인자로 넘긴다**.

```cpp
KnockbackComp->ApplyKnockback(*Payload, EnemyDefinition ? EnemyDefinition->KnockbackDistance : 0.f);
```

## 4. 연타 처리

```cpp
Move->RemoveRootMotionSource(KnockbackSourceName);   // 적용 전 직전 것 제거
```

`InstanceName` 을 붙여두면 이름으로 지울 수 있다. 안 지우면 두 소스가 겹쳐 이동량이 합산된다.

## 5. 디버그를 목표 대비 실제로 바꿨다

```
전   Knock  Light  x1.00   speed 1800   ->  247 cm   (남은속도 310)   0.1초 시점
후   Knock  Light  x1.00   목표 60 cm   ->  실제 60 cm                밀림 종료 직후
```

측정 시점을 `KnockbackDuration + 0.05f` 로 옮겼다. **밀림이 끝난 뒤에 재야** 목표와 실제를 비교할 수 있다.

### 실측 — 계산은 정확하다

`KnockbackBrainPause`(0.15) 가 `KnockbackDuration`(0.2) 보다 짧아 **밀림 종료 전에 AI 가 깨어난다.** 이게 측정값을 깎을 거라 예상했으나, BP 에서 0.35 로 올려 재보니 **값이 동일**했다. 되밀림 가설은 기각.

-> 디버그 숫자는 믿어도 된다. 60 이 약했던 건 되밀림 탓이 아니라 **그냥 60cm 가 작아서**였다.

## 6. 값 감각 (실측 기준선)

```
밴딧 캡슐    지름 68cm · 키 176cm
플레이어     지름 68cm · 키 190cm
바닥 Plane   10000cm (100m) 정사각형 5장, MI_ProcGrid
```

| KnockbackDistance | 캡슐 |
| --- | --- |
| 60 | 0.9개 — 살짝 휘청 |
| 100 | 1.5개 |
| 200 | 3개 |
| 340 | 5개 |

⚠️ `MI_ProcGrid` 의 한 칸이 몇 cm 인지는 **확인 못 했다.** 파이썬이 머티리얼 표현식에 접근이 안 되고, 인스턴스에 격자 크기 파라미터가 노출돼 있지 않다. 자로 쓰려면 캡슐(68cm)을 기준 삼는 편이 확실하다.

## 7. 검증

```
빌드            통과
PIE 밀림        동작 확인 — 더미 200 으로 체감 적정
디버그 재현성    BrainPause 0.15 / 0.35 에서 측정값 동일
BP 오버라이드    적 6종 전부 C++ 기본값 그대로 (조회 확인)
```

**미검증** — 공중에 뜬 적. `AccumulateMode::Override` 가 속도를 통째로 덮으므로 낙하 중 중력이 눌릴 수 있다. 뜨는 적이 생기면 `Additive` 나 `FRootMotionSource_ConstantForce` 검토.

## 8. 남은 것

```
적 5종 KnockbackDistance   더미만 200 으로 잡음. 나머지는 체급 기준으로 나중에
C1 콤보 노드 값            KnockbackMultiplier 포함 3칸이 전부 0
                          -> 지금은 타격마다 넉백 배수가 전부 1.0 (마무리타가 안 셈)
BrainPause / Duration 관계  0.15 < 0.2 역전. 측정엔 영향 없음이 확인됐고 연출 판단만 남음
```

### 줄 수

```
KDEnemyBaseCharacter.cpp   655 -> 605   (B2 로 늘었던 33줄 회수 + 추가 감소)
KnockbackComponent         .h 29 / .cpp 102
```
