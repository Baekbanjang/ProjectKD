# 2026-08-27 — 대상 탐색을 필터 구조로 + 자동 조준 결함 6건 해소

락온과 자동 조준이 한 함수를 쓰던 것을 갈라내고, 탐색 조건을 `FKDTargetFilter` 한 벌로 데이터화했다.
근거 = SB 덤프 실측(`TargetFilterTable` 1866행) + 업계 조사. 메모리 `reference_sb_skilltable_layers` · `reference_melee_autoaim_research`.

## 1. 왜 했나 — 증상 하나에서 결함 6개가 나왔다

승환 관측 = **"갑자기 공격할 때 공격 방향이 엉뚱하게 튄다".**
모션 워핑 회전을 의심했으나 **반증됐다** — 노티가 `RotationType = Facing` 이고, 엔진 실측상 Facing 은
타깃 **위치**로만 방향을 만든다(적의 회전 미참조, `RootMotionModifier.cpp:393-406`).

진짜 원인은 자동 조준 두 개가 겹친 것이었다.

```
① BP CDO 의 AutoAimConeAngle = 360   ->  cos(180도) = -1  ->  "Dot < -1" 이 영원히 거짓
                                     ->  시야콘 필터가 통째로 무효. 반경 800 전방위가 후보
② KDGameplayAbility_PlayerMelee.cpp:135 의 DeltaYaw 가 계산만 되고 미사용
                                     ->  주석은 "뒤쪽 135도 초과 제외" 인데 가드가 없다
```

콘 360 은 의도(후보를 넓게 줍기)였고 각도 제한은 ②가 맡기로 돼 있었는데, **그 가드가 비어 있었다.**
컴파일 경고도 안 뜬다 — `const float` + 함수 호출이라 미사용 경고를 피한다.

## 2. 해결된 것 6건

```
등 뒤 180도 스냅        MaxAutoAimTurnAngle 가드 + 부채꼴 반각 90도로 후보 자체를 제한
콘 360 무효화           반각을 5~135 로 클램프 — 부채꼴로는 전방위를 만들 수 없다
                       전방위가 필요하면 ShapeType 을 Cylinder 로 "골라야" 한다
멀어도 중앙이면 뽑힘     자동 조준 정렬을 Nearest 로. 코앞 적을 친다
기준이 카메라           Basis = CharacterForward. 캐릭터가 보는 쪽
위아래 안 잘림          구 -> 박스 수집 + 가로 거리로 원기둥. 높이 500 / 발밑 -150
락온·자동조준 한 함수    FindBestTarget(락온) / FindTargetByFilter(공용) 분리
```

## 3. 구조

### 신규 `Combat/Data/KDTargetFilter.h` (`.cpp` 없음)

```
EKDTargetShapeType   Arc | Cylinder
EKDTargetSortType    Nearest | SmallestAngle
EKDTargetBasisType   Camera | CharacterForward     <- SB 에 없는 칸. 우리가 겪은 문제라 명시
FKDTargetFilter      위 3개 + Radius · HalfAngle · Height · HeightOffset · bDrawDebug
```

### 조회 3단 (UE 에 원기둥 오버랩이 없어서)

```
1  박스로 줍는다        원기둥을 감싸는 최소 상자. 캡슐은 반지름 > 반높이면 구가 돼 못 쓴다
2  가로 거리로 깎는다    모서리 제거 -> 원기둥
3  반각으로 깎는다       Arc 일 때만 -> 부채꼴
```

`GatherCandidates`(수집·자격·LoS) / `FindTargetByFilter`(정렬·선택) / `GetFilterBasis`(기준 벡터) 로 나눴다.
정렬은 **거리 부호를 반전**해 각도순·거리순이 비교문 하나를 공유한다.

### 값 (에디터)

```
DA_LockOnConfig_Default   Arc · 1000 · 반각 45 · Camera · SmallestAngle
GA_LightCombo · Heavy     Arc ·  800 · 반각 90 · CharacterForward · Nearest
공통                       Height 500 · HeightOffset -150 (SB 실측)
```

⚠️ **반각 표기 주의** — 옛 `ViewConeAngle 90`(전체)이 새 구조에선 `HalfAngle 45`다. 숫자가 절반이 된다.

## 4. SB 와의 대조 (참고)

```
SB 일반 콤보    3DArc · ±90도 · 500cm · 높이 500(발밑 -150) · 각도순 · 55개 스킬이 필터 하나 공유
SB 회피         3DCylinder · 전방위 · 5000cm · 거리순
SB 전체 정렬     Near 1762 (94%) / SmallAngle 50 — 거리순이 기본, 검 콤보만 예외
SB 락온         LockOnOverrideTargetFilterAlias 라는 별도 칸. 쓰는 스킬은 진입기 7개뿐
```
우리는 형태를 SB 콤보(부채꼴 ±90)에서, 정렬을 SB 주류(거리순)에서 가져왔다.

⬜ **안 가져온 것** — 회전 보간(SB 대시어택 `RotationEndTime 0.05`). 우리 몽타주 워프가 이미
`Facing` 회전을 하고 있어 중복이다. 스냅이 거슬리면 그때 넣는다.

## 5. 디버그

필터에 `bDrawDebug` 를 두어 DA·GA 마다 따로 켠다.
```
하늘색 호·선   필터 범위 (지면 높이)
흰 선          기준 벡터 — 카메라인지 캐릭터 정면인지가 눈에 보인다
노란 수직선     실제 판정 세로 범위
초록 구         통과 후보  |  빨강 구  최종 선택
```
⚠️ **원점이 캡슐 중심이라 `HeightOffset -150` 은 지면 아래 62cm 다.** 처음엔 호를 그 높이에 그려
땅에 묻혀 안 보였다. 호는 지면 높이 한 겹만 그리고 세로 범위는 수직선으로 표시하도록 고쳤다.

## 6. 곁가지

```
ShotBlast     시그니처 변경에 딸려 임시 필터를 매번 만든다. 멤버로 올리는 건 다음 건
경고 2건       AbilityTags 지원 중단(CounterThrust:13 · SprintAttack:10) — 이번 변경과 무관
KDLockOnComponent 이 400줄대로 늘었다. §1 Component 300줄 선 초과 — 분리 검토 대상
```

## 검증

```
빌드   Succeeded / 에러 0 / 경고 2(무관)
PIE    승환 확인 — 정상 동작
```
