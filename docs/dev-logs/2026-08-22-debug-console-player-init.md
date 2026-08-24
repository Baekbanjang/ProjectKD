# 2026-08-22 — 디버그 콘솔화 / 플레이어 초기값 GE / 마무리 워프 회전

## 0. 요약

포폴 촬영 준비 중에 나온 4건. 전부 "켜고 끄는 자리가 코드 안에 박혀 있던" 문제다.

| 항목 | 전 | 후 |
| --- | --- | --- |
| 온스크린 디버그 4종 | `if (GEngine)` — 항상 켜짐, 끄려면 재컴파일 | 콘솔 변수 `KD.Show*` |
| 플레이어 어트리뷰트 초기값 | C++ 생성자 | `GE_InitPlayerStats` (BP) |
| 마무리 5타 착지 방향 | 적 등 뒤에 착지 | 워프 회전 해제 |
| 입력 버퍼 상한 | 0.5 | 0.8 |

커밋 = Content `6b7003b` `c0a7575` `e7b6ecc` `feb5461` `5be9ee2` `5068971`

---

## 1. 온스크린 디버그 → 콘솔 변수

### 문제

데미지 숫자 · 회피 판정 · 접근 거리 · 넉백 계측이 `#if !UE_BUILD_SHIPPING` 안에서 `if (GEngine)` 하나로만 걸려 있었다. **개발 빌드면 무조건 켜진다.** 끄려면 코드를 고치고 다시 빌드해야 했다.

### 해결

```cpp
#if !UE_BUILD_SHIPPING
// 개발용 데미지 표시 스위치 — 콘솔 KD.ShowDamage 1
static TAutoConsoleVariable<int32> CVarShowDamage(
	TEXT("KD.ShowDamage"), 0,
	TEXT("피격 데미지 온스크린 표시 유무"), ECVF_Cheat);
#endif
```

호출부는 `if (GEngine && CVarShowDamage.GetValueOnGameThread() > 0)` 로 게이트 하나만 덧댄다.

| 콘솔 | 파일 | 표시 내용 |
| --- | --- | --- |
| `KD.ShowDamage` | `AS_Combat.cpp` | 실드·방어 경감 후 체력에 들어간 최종량 |
| `KD.ShowDodge` | `GA_Dodge.cpp` | `PERFECT DODGE` / `Normal Dodge` |
| `KD.ShowApproach` | `GA_PlayerMeleeAttackBase.cpp` | 접근 거리 + 어느 게이트에 걸렸는지 |
| `KD.ShowKnock` | `KDEnemyBaseCharacter.cpp` | 배수 / 속도 / 0.1초 뒤 이동 거리 / 남은 속도 |

`ECVF_Cheat` = 출시 빌드에서 등록 자체가 빠진다.

### 왜 `UPROPERTY` 체크박스가 아닌가

데미지와 넉백은 **GA 인스턴스가 아니라 어트리뷰트셋과 Pawn 에서 찍는다.** 체크박스를 두려면 "어느 에셋에 둘지"부터 정해야 하고, 켜고 끌 때마다 그 에셋을 열어야 한다. 콘솔은 PIE 도중에 바로 켜고 끈다.

⚠️ **무기 궤적(초록·빨강 캡슐)과 조준선은 여전히 BP 체크박스다** (`GA_MeleeTraceBase::bDrawDebug` / `GA_Shoot::bDrawAimDebug` / `GA_ShotBlast::bDrawDebug`). GA 별로 따로 켜는 값이라 콘솔로 안 뺐다.

---

## 2. 플레이어 초기값을 GE 로

### 문제

플레이어 어트리뷰트 초기값이 **C++ 생성자에만** 있었다. 탄약을 25 → 5 로 바꾸려고 해도 재컴파일이 필요하다.

적은 `EnemyDefinitionDataAsset` 로 이미 데이터화돼 있었는데 플레이어만 코드에 남아 있었다.

### 해결

`GE_InitPlayerStats` (Instant) 신설 → `BP_PlayerState` 의 `StartupEffects` 배열에 등록.

적용 지점은 새로 만들지 않았다. 이미 있던 루프를 쓴다.

```cpp
// KDPlayerState.cpp:40
for (const TSubclassOf<UGameplayEffect>& EffectClass : StartupEffects)
```

이제 플레이어 스탯은 **BP 에서 고친다.** 생성자 값은 폴백으로 남는다.

---

## 3. 마무리 5타 착지 방향

### 증상

`05_04` 로 콤보를 끝내면 캐릭터가 **적과 같은 방향(등 뒤)** 을 보고 착지한다.

### 원인

접근 워프 노티의 `RotationType = Facing` + 동기점이 캐릭터 뒤쪽. `05_04` 는 캐릭터가 적을 **지나쳐 달려나가는** 클립이라, "적을 보게" 돌리면 진행 방향과 반대로 돈다.

### 해결

마무리 5타(`AM_SB_Combo_01_04` ~ `05_04`) 전부 노티의 `warp_rotation` 해제. 접근 워프는 **이동만** 담당하고, 회전은 `UGA_PlayerMeleeAttackBase::OnActivated` 의 `SetActorRotation` 이 이미 처리한다.

> ⚠️ 워프 노티 19개 중 **`AM_SB_Parry_Counter_Attack_L` 하나만 `RotationType = DEFAULT`** 로 남아 있다(타겟명 `CounterTarget`). 나머지는 전부 `Facing` 이었다. 반격이 엉뚱한 데로 도는지 확인 필요 — 미확인.

### 진단 순서

가설(`RotationType` 이 원인)을 **워프 노티 19개 전수 측정으로 먼저 반증**했다. 전부 `Facing` 이라 "이 값이 특이해서"가 아니었다. 그다음 A/B(회전만 끄고 재생)로 확정.

---

## 4. 입력 버퍼 상한

```cpp
// InputBufferComponent.h
UPROPERTY(EditAnywhere, Category="Input Buffer", meta=(ClampMin="0.05", ClampMax="0.8"))
float BufferTimeWindow = 0.2f;
```

상한만 0.5 → 0.8 로 열었다. **기본값 0.2 는 그대로.** 근거 = SB 입력 접수창 실측 0.7~0.8.

지상 콤보 15개 중 8개에서 입력이 증발하던 건이 있어 튜닝 여지를 열어둔 것이고, 실제 값 조정은 미착수.

---

## 5. 촬영용 임시값 (원복 완료)

촬영 편의로 잠깐 바꿨다가 되돌린 것들. **커밋에 남아 있으니 현재 값과 헷갈리지 말 것.**

| 대상 | 촬영값 | 현재 |
| --- | --- | --- |
| `DA_Sword_Bandit` MaxPoise | 1 | 3 |
| `GA_Parry` 퍼펙트 창 | 0.5 | 0.2 |
| GA 디버그 표시 (근접·총격) | 끔 | — |
| `LV0_Test` 조명·노출 | 노출 고정 / 모션블러 끔 / 그림자 거리 축소 | — |

⚠️ 노출은 `AEM_MANUAL` 로 바꾸면 **화면이 통째로 검어진다.** Manual 은 씬 밝기를 무시하고 실사 카메라 값(주광 기준)으로 계산한다. 고정하려면 Histogram 유지 + `min = max` 로 잠근다.

---

## 6. 남은 것

```
확인      AM_SB_Parry_Counter_Attack_L 의 RotationType = DEFAULT
폴리싱    입력 버퍼 실제 값 조정 (상한만 열어둔 상태)
근본      넉백을 RootMotionSource 로 (2026-08-21 §2 에서 이월)
```
