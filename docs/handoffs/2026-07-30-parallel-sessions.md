# 현재 상태 — 2026-08-14

> **세션 시작 시 여기부터.** 진행상황·다음 할 일·보류 목록·설계 미결.
> ⛔ 2세션 병행(A/B 레인)은 **2026-07-31 종료**. §0 참조 — 그 규칙을 따르지 말 것.

---

## ★★ 2026-08-14 — 공중 콤보 캔슬 윈도우 정렬. **PIE 통과·커밋 완료**

dev-log = `docs/dev-logs/2026-08-14-air-combo-cancel-window.md`

08-13이 남긴 공중 잔여를 전부 닫았다.

| 타 | 이전 | 지금 | 근거 |
|---|---|---|---|
| Air_01 | **캔슬창 없음** | f25 (0.4167s) ~ 끝 | 포즈 거리 0.0 + `_All` f0→25 |
| Air_02 | f34 | **f40** (0.6667s) ~ 끝 | 포즈 거리 0.0 + `_All` f25→65 |
| Air_03 | f41 | **f55** (0.9167s) ~ 끝 | 포즈 거리 0.0 + `_All` f65→120 |
| Air_04 | f22 | 그대로 | 마지막 타라 기준 클립 없음 |

**포즈 매칭과 `_All` 교차검증이 세 타 모두 차이 0.** `Air_03`이 14프레임 빨랐던 게 작성자 육안 관측과 일치했다.

### 같이 닫힌 것

- **`Air_02`·`_04` 총·검 재확인** — 본 속도비 0.47 / 0.79 = **둘 다 검 확정.** 기존 배선 그대로 맞았다. `Air_01`이 08-13 기록과 같은 4.75로 재현돼 방법 신뢰도도 확인
- **공중 총격 PIE** — 통과. `GA_ShotBlast`가 공중에서 정상 작동
- **`Air_01` 캔슬창 부재** — 07-30 dev-log의 "`CancelWindow 28~55f` 복원 ✅"는 **에셋에 반영된 적이 없다.** Content 이력으로 확인(`72017a2`부터 0 히트). 계획만 문서에 적힌 건

### 이번에 세운 것 (재사용)

1. **공중은 Idle 복귀 구조가 아니다** — 각 타가 앞 타 끝 자세에서 시작(t=0 포즈끼리 315~658도 차이). 지상용 `anim-cancel-scan` 절차를 그대로 쓰면 안 되고, "다음 타 f0를 앞 타에서 전역 스캔"으로 간다
2. **`fps = (nf-1)/L` 은 틀렸다** — 85f/1.41667s는 60.0이지 59.29가 아니다. 프레임↔시각은 **`AL.get_time_at_frame()`**. 이 오차로 1.3프레임 늦게 박았다가 되돌렸다
3. **`GameplayTag`는 파이썬 repr이 항상 `{}`로 보인다** — `tag_name`까지 꺼내야 안다. 08-12의 진짜 빈 태그 결함과 착시를 구분할 것
4. **노티 시각 수정 API는 없다** — 트랙을 비우고(`remove_animation_notify_events_by_track`) 다시 추가(`add_animation_notify_state_event`). `UANS_CancelWindow`는 생성자가 태그를 넣으므로 재생성해도 딸려온다

### 남은 것

- **`Air_04` 캔슬 기준 미확정** — 마지막 타라 다음 클립이 없다. 지상 연결(`Attack_Air_to_Floor` 마무리안, §4-②)을 붙일 때 그 클립 f0로 같은 측정을 하면 나온다
- 08-13 잔여 그대로 — 급정지 발소리 미배선 / `SC_Shotgun_Shot` 큐(5연사가 같은 소리) / 총성 소재 조달 실패

---

## ★★ 2026-08-13 (2) — 사운드 전면 배선. **PIE 통과·커밋 완료**

dev-log = `docs/dev-logs/2026-08-13-auto-aim-and-gun-sound.md` §4~8

```
총성        13개 몽타주 21발 (지상 콤보) + 조준 사격 + 공중 1·3타
발소리      클립 199개 / 노티 488개 (Unarmed·Combat·Aim 전부)
점프·착지   Jump_Start/End ×2 씩
단발 공격   Run_Attack_01 / Parry_Counter_Attack_L
```

**Content 커밋 5개**(로컬) `595c78b` `d8976c9` `57668ad` `8cf9172` `de3aadc` `4a51fa8` `197285d` `1143e35`

### 이번에 세운 방법 3개 (재사용)

1. **발소리 접지 검출** — `foot_l/r`의 컴포넌트 공간 Z 최저 구간 시작. 규칙 4개(`TOL 1.0` / `MIN_GAP 6f` / 루프 클립만 프레임0 인정 / `MIN_LIFT 2cm`)가 다 필요했다. 하나씩 빠뜨렸을 때의 오검출 사례는 dev-log §5
2. **총 공격 판별** — `gun_weapon_l` vs `sword_weapon_r` 골반 상대 속도비. **1 넘으면 총.** `Air_01` 4.75로 확정. ⚠️ 총구 방향으로는 검증 불가(본 X축이 총열이 아님)
3. **사운드 겹침** — `SoundConcurrency` 의 `RetriggerTime`. ⚠️ `Resolution Rule` 을 `Prevent New` 로 하면 **소리 길이 > 간격**일 때 발소리가 대량으로 죽는다. `Stop Oldest` 가 맞다

### 남은 것

- **급정지 발소리**(`_Scuff` / `_ScuffPivot`) 미배선
- ~~**`Air_02` / `_04`가 정말 검인지** 재확인~~ **✅ 닫힘 (08-14)** — 속도비 0.47 / 0.79 = 둘 다 검. 기존 배선 맞았음
- ~~**공중 총격 PIE 미검증**~~ **✅ 닫힘 (08-14)** — 통과
- **총성 소재** — GDC 2026 번들에 총기 0개. `The Free Firearm Sound Library`(CC0 194MB, OpenGameArt)가 답인데 **승환이 받기 실패**. 현재 총성은 땜빵

---

## ★★ 2026-08-13 (1) — 일반 공격 자동 조준. **✅ 빌드·PIE 9항목 통과·커밋 완료**

> 아래 "🔴 정리 안 된 것" / "PIE 검증 항목"은 **전부 해소됐다.** 백업 파일·고아 include·주석 정리 끝났고 디버그 구체 분기도 들어갔다(`GA_ShotBlast.cpp:109`). 이력용으로만 읽어라.

08-12의 "다음 = 일반 공격 자동 조준"을 착수했다. **락온을 안 걸어도 공격마다 가장 알맞은 적 쪽으로 몸이 돈다.**

### 결정 (SB 덤프 실측 근거)

| 항목 | 값 | 근거 |
|---|---|---|
| 고르는 규칙 | **각도 최소** (거리 최소에서 바꿈) | Eve 검 콤보 17개가 전부 `SortType = SmallAngle` |
| 부채꼴 | **±90°** (`AutoAimConeAngle 180`) | `P_Eve_Sword_0500_180_500_Target` |
| 사거리 | **500cm** | 같은 필터 `FarDistance` |
| 방향 기준 | **카메라 forward** (현행 유지) | SB는 기준 벡터가 표에 없음 = C++. 추측 배제 |
| 총격 판정 원점 | **캡슐 중심** (총구에서 이동) | 총구가 앞서 있어 몸에 붙은 적이 콘 밖으로 빠지던 문제 |

### 바꾼 것 (소스 8파일, 미커밋)

```
GA_ActionBase.h/.cpp          FindAutoAimTarget(Range, ConeAngle) 신설
                              = 락온 중이면 GetLockedTarget / 아니면 FindBestTarget
LockOnComponent.h/.cpp        FindBestTarget 에 인자 2개(반경·각도, 음수면 Config)
                              정렬을 최단거리 → 콘 중심선 각도 최소로
GA_PlayerMeleeAttackBase.h/.cpp   락온 태그 게이트 삭제 + AutoAimRange/AutoAimConeAngle 칸
GA_ShotBlast.h/.cpp           락온 전용 조회 → FindAutoAimTarget / 판정 원점을 캡슐로
                              + AutoAimConeAngle 칸
```

`GA_PlayerAirAttackBase`가 `Super::OnActivated()`를 부르므로 **공중 콤보에도 자동으로 붙는다.**

### 🔴 정리 안 된 것 (빌드 전에)

1. **`Source/.../GA_ShotBlast.cpp~` 백업 파일 untracked** — `.gitignore`에 `*~` 패턴이 없어 커밋에 딸려 들어간다. 삭제할 것
2. **고아 include** — `GA_ShotBlast.cpp:12` `Combat/LockOnComponent.h`. 락온 블록 삭제로 사용 0건
3. **주석 4줄** — `GA_ShotBlast.cpp:31,53,99`(총구 → 원점) / `GA_PlayerMeleeAttackBase.cpp:59`(락온 전용 아님) / `LockOnComponent.cpp:142`(**뜻이 반대로 적혀 있다** — "있으면 Config"가 아니라 "없으면 Config")

### ⚠️ PIE에서 같이 볼 회귀 — 락온 토글도 바뀌었다

`ToggleLockOn`(`LockOnComponent.cpp:126`)과 타겟 자동 전환(`:66`)이 같은 `FindBestTarget`을 쓴다.
→ **락온 키가 이제 "가장 가까운 적"이 아니라 "화면 한가운데 적"을 잡는다.** 의도한 변경(SB 락온 필터도 `SmallAngle`)이지만 "락온이 이상해졌다"로 오해하기 쉽다.

### PIE 검증 항목

```
1  락온 X, 적 옆에서 공격        몸이 도는가
2  적 둘(정면 8m / 측면 2m)      정면 쪽을 치는가        ← 각도순 확인
3  5m 밖 적                      안 돌아야 한다
4  등 뒤 적                      안 돌아야 한다           ← 135도 예외
5  락온 X 로 콤보 총격           콘이 적 쪽으로 도는가    ← 신규
6  몸에 붙은 적                  맞는가                   ← 캡슐 원점 효과
7  AM_SB_Combo_05_03 윈드밀      5발이 사방으로 도는가    ← bUseMuzzleDirection 회귀
8  락온 걸고 공격                종전과 같은가
9  락온 토글                     화면 중앙 적을 잡는다(변경됨, 정상)
```

6번에서 앞쪽 사거리가 짧아진 게 느껴지면 `ShotRange`를 올린다 — 원점이 총구에서 몸으로 왔으니 총구가 앞서 있던 만큼(미실측, 눈대중 50~80cm) 앞 도달이 준다.

### 이 세션의 조사 산출물

**옵시디언 `notes/Reference/StellarBlade_스킬테이블_3층구조.md` (신규)** — SB 스킬 테이블 3층 구조 + `TargetFilterTable` 전수. `JSON덤프_전체구조분석` §2와 `이브_콤보트리_전체복원` 상단에 포인터 추가.

밝힌 것 셋:
- **SB는 `_Target`(누구를 향하나)과 `_HitArea`(누가 맞나)를 별도 필터로 나눈다.** 타격 판정은 `ActiveCollision` 1552 / `TargetFilter` 437 — **대부분 무기 콜리전.** 우리 `ANS_MeleeTrace`와 같은 구조
- **`RotateInputDirection`** — 전체 1414개 중 true 177개, 그 중 **175개가 Eve.** 이름대로면 "입력 방향 회전". SB가 입력 방향을 쓴다는 근거는 이 칸뿐이고 **정확한 동작은 C++이라 확인 불가**
- **스킬마다 필터 칸이 있지만 Eve 검 17개는 값이 전부 같다.** 노드마다 사거리 칸을 파는 건 근거 없음

### 🟡 폴리싱으로 미룸 (승환 결정, 2026-08-13)

**`FComboNode.DamageMultiplier` + `InputWindow` 26칸 값** — SB 표를 보면서 같이 하기로. 설계는 다 나왔다:

```
① ComboTreeDataAsset.h  FComboNode 에 float DamageMultiplier = 0.f  (0 = GA 기본값)
② GA_MeleeTraceBase.h   DefaultDamageMultiplier(EditDefaultsOnly) + DamageMultiplier(런타임)
③ GA_MeleeTraceBase.cpp:210  AttackPower * DamageMultiplier    ← 곱하는 자리는 여기 하나
④ GA_PlayerAttackBase.cpp    :21 옆에 리셋 / :34 아래에 노드 값 덮어쓰기
```

**이미 있는 패턴의 확장이다** — `DamageEffectClass`가 정확히 같은 방식으로 돈다(GA 기본값 → 노드가 있으면 덮어씀). 새 클래스 0개.
SB 계수 = 라이트 `0.7/1.0/1.0/1.8/0.9/3.0/2.2/1.7` · 스트롱 `0.9/1.9/1.8/…/4.0` · 가드 데미지는 항상 정확히 절반. 입력창 `0.7/0.8/0.8`.

⚠️ **`GA_ShotBlast`는 `UGA_ActionBase` 직속이라 이 경로를 안 탄다.** 총격 계수는 별도.

---

## ★★ 2026-08-12 갱신 — 콤보 속 총격 히트스캔 완료 + 입력 리팩토링

**확정 순서 ①카메라 → ②히트스톱 → ③총 발사** 는 08-03/08-04/08-05~08-11에 걸쳐 전부 끝났다. 오늘은 그 위에서 나온 후속 작업.

`GA_ShotBlast`(콤보 중 총격 = 근접 콘 히트스캔, 08-12 오전 `93cb7fe`로 신설) **완료·PIE 통과.** 총구를 캐릭터 소켓에서 무기 메시 소켓(`Gun_Muzzle`→`Muzzle`)으로 옮기고, 노티별 예외(총구 방향 발사/히트스톱 끄기/각도 오버라이드) 배선까지 끝냈다. 결함 2건도 같이 닫혔다 — 어빌리티 태그 충돌(`GA_ShotBlast`/`GA_Shoot`이 같은 태그를 써서 조준 사격마다 콘 히트스캔이 공짜로 붙던 문제), `ANS_EnemyAttackWindow` 태그 미초기화(퍼펙트 회피가 한 번도 성립한 적 없던 문제, §3 참조).

같은 세션에 `AKDPlayerCharacter` 700→426줄(입력 판단을 `UKDPlayerAbilityInputComponent`로 분리, §1 500줄 한도 해소) + GA의 구체 Pawn 캐스팅 4곳 제거(`UGA_ActionBase` 접근자로 통일)도 끝났다.

**dev-log = `docs/dev-logs/2026-08-12-input-component-and-muzzle-socket.md`**

커밋 — 소스 `d65d21a` `171e786` `ae076f3` `512c3a4`(push 완료) / Content `b1dc4e8`(로컬). **미커밋 = 소스 11파일 + Content 5에셋.**

**→ 다음 = 발사체 리팩토링 3건(착수 가능) 또는 `DA_ComboTree.InputWindow` 값 채우기(작성자가 "나중에"로 보류).** 아래 §3 참조.

---

## ★ 2026-08-03 갱신 — 카메라 완결

**확정 순서 ①카메라 → ②히트스톱 → ③총 발사 중 ①이 끝났다.**

`UKDSpringArmComponent::UpdateLookRotation`을 **랙·충돌 전 이상 위치 기준**으로 교체(8단계→5단계) + 마우스 감도 노출. 빌드 통과 + **PIE 6항목 전수 통과**.

> ⚠️ **아래 §2-0 / §2-0-B의 "승인 대기" 표기는 전부 해소됐다.** 그 절들은 설계 근거·수치 참조용으로만 읽어라. 현재 구현 상태는 dev-log가 진실이다.
> **dev-log = `docs/dev-logs/2026-08-03-camera-rail-look-rotation.md`**

**미커밋 8개** — `KDSpringArmComponent.h/.cpp`(신규) · `KDPlayerCharacter.h/.cpp` · `KDPlayerController.h/.cpp` · `docs/INDEX.md` · dev-log. master가 origin보다 1 앞섬

**남은 확인 2개(급하지 않음)**
1. `IMC_Default`의 `IA_Look` Modifiers에 `Scalar`가 있는지 — 최종감도 = `IMC Scalar × 코드값`. 감도 실사용 튜닝은 승환이 BP로 진행
2. 벽 충돌 시 조준 보정 부재 — 현재 안 거슬림. 필요 시 `IdealCam.X`를 압축 거리로 대체(랙은 계속 제외)

**`AKDPlayerCharacter` 601줄** — 500 한도 초과지만 **분리하지 않기로 결정.** 카메라 셋업은 생성자 20줄뿐이라 빼도 581. 실제 무게는 `Try*` 6개(171줄)·`Tick`(101줄)이고, 그것들도 지금 쪼갤 이유가 없다

**→ 다음 = ②히트스톱 재진입 가드** (§2-1 / 아래 "그다음 할 일" 1번)

---

## 0. ⛔ 2세션 병행 종료 — 단일 세션으로 복귀 (2026-07-31)

**A/B 레인 규칙은 폐기됐다.** 아래 규칙을 따르지 말 것. 세션은 하나로 운영한다.

**왜 접었나** (2026-07-30 시작 → 07-31 종료, 이틀)

| 문제 | 실제로 벌어진 일 |
|---|---|
문서 비용이 작업 비용을 넘었다 | 대화에 없는 독자용 보고서를 쓰느라 세션 절반이 갔다. 07-31 실작업은 트레일 배정 1건, 나머지는 조사·보고 |
레인 규칙이 지켜지지 않았다 | 핸드오프는 B가 써야 굴러가는데 `docs/`는 A 영역이었다 → B가 계속 위반 |
커밋 충돌 | `docs/INDEX.md`(A 미커밋분)가 B 커밋 `74ae694`에 쓸려 들어갔다 |
정보 시차 | A가 낡은 핸드오프를 근거로 판단 → 상호 정정에 왕복이 더 들었다 |

**살릴 것** — A레인 산출물 3개는 유효하다. 폐기하지 않는다.
- 옵시디언 `notes/Project_New/로드맵_GunSword_v1.md` (M1~M6, 24주)
- 옵시디언 `notes/Project_New/GunSword_전투수치_v1.md` (제안·미반영)
- `docs/INDEX.md` 포인터 2줄

**이 문서의 새 성격** — 레인 핸드오프가 아니라 **현재 상태 요약**이다. 세션 시작 시 §1 / §1-B / §3 / §4만 보면 된다.

---

## 1. 지금 어디까지 왔나 (2026-07-30 종료 시점)

### 프로젝트 상태
- **Gun & Sword 피벗 라인** (2026-07-21). SB(스텔라 블레이드) 스타일 지상 액션
- 캐릭터 = 마네퀸(`SK_Mannequin_GunSword`), 이동 = 모션매칭 + Chooser, 전투 = 몽타주
- 코드 규모: `.h` 86개 ≈ 4,000줄 / `.cpp` 83개 ≈ 6,900줄
- **`docs/archive/` 는 읽지 않는다** — 버터/길동 폐기 라인

### 오늘 완료된 것

| 항목 | 결과 |
|---|---|
**`State.Combat.InAction` 우산 태그** | 코드 + BP 7개 + 빌드 + PIE 이상 없음 ✅ |
**회피 캔슬 목록 확대** | Light만 → 공격 4개. 세 곳의 서로 다른 목록을 헬퍼 하나로 통합 ✅ |
**ABP_SB 슬롯 재배치** | `Slot 'DefaultSlot'`을 Locomotion SM 뒤로 + 중복 노드 제거 → 공중 몽타주 출력됨 ✅ |
**`AM_SB_Combo_Air_01` 노티 복원** | `WeaponAttach@0f` + `CancelWindow 28~55f`. PIE 로그로 4타 연결 확인 ✅ |
**퍼펙트 패링 큐 발동 노드** | `GA_PerfectParryReaction`에 `Execute GameplayCue On Owner` 추가 ✅ |
**코드 설명서 8문서** | 옵시디언 볼트 `notes/코드구조/` 2,776줄 신설 ✅ |

### 커밋

```
소스 repo (GitHub 푸시됨)
  6cc9340 [doc]    INDEX에 코드 설명서 포인터
  474bd52 [Combat] 회피 캔슬 목록을 공격 4개로 확대
  4a44cfd [GAS]    State.Combat.InAction 우산 태그

Content repo (로컬 전용, 푸시 금지)
  72017a2 [Anim] AM_SB_Combo_Air_01 노티 + 스프린트/반격 GA에 Attacking
  33b9957 [BP]   InAction 태그 7개 + 퍼펙트패링 큐 + ABP DefaultSlot 이동
```

---

## 1-B. 2026-07-31 — 조사 중심 세션

코드 변경은 거의 없고 **실측·조사·문서 정정**이 대부분이다.

### ✅ 완료

| 항목 | 결과 |
|---|---|
**무기 트레일 NS 배정** | `NS_SlashTrail_Distortion_Only_Loop` → 몽타주 25개 / 슬롯 27개. Content `4f1ac60` ✅ |
**SB 카메라 실측 재검증** | 원본 JSON 재파싱. **볼트 문서 정정 3건 + FOV 문제 해결** (아래) |
**SB 조작 키맵 확정** | 승환 캡처로 마우스 버튼 세부까지 확정. 메모리 `reference_control_scheme` 전면 갱신 |
**사격 로직 사전조사** | 발사체 채택 결정 + 팩 애니 인벤토리 + 배선안 (아래) |

트레일은 SB식 **무색 왜곡** 궤적으로 골랐다(색 없음, 배경을 일그러뜨리는 방식).
⚠️ 미확인 하나 — 우리 `ANS_WeaponTrail`이 미는 `SwordLength`/`TrailWidth` User Parameter를 이 NS가 받는지는 **PIE로 봐야 안다.**

---

### ★ SB 카메라 — 볼트 문서에 틀린 게 3개 있었다

전문은 옵시디언 `notes/Reference/StellarBlade_Player_Analysis.md §4-6` (2026-07-31 정정 반영 완료).

| # | 이전 기술 | **실측** |
|---|---|---|
1 | 스플라인 회전 `+81.6 / -33.7 / -36.6` | **`-81.6 / +33.8 / +36.6`** — 부호 전부 반대. BalanceBeam·SlidingSlip도 |
2 | "SocketOffset 덤프 미출력 = 비교 불가" | **스플라인 t=1에 `Y: 40`이 있다** — 어깨 오프셋. 우리도 40이라 이미 일치 |
3 | SlidingSlip만 2점 | **`SlopeMoving2`도 2점** |

**새로 밝혀진 것 2개**

- **FOV 3개 공존 문제가 거의 풀렸다.** `Eve_CameraFov`가 `0→75, 1→75`로 **평평하다 = 무효 슬롯**. 남는 건 컴포넌트 `55`(초기값 성격)와 `FovCurveByCamDist` 40~75 → **거리연동이 이길 가능성 높음**(C++ 없어 추론)
- **`Eve_CameraPitch`(-89~+45)는 FOV 커브가 아니라 돌리 위치 입력이었다.** 시선 피치가 카메라를 곡선 위로 끌고 다닌다. 이게 `ViewPitchMin`/`Max`의 근거

**★가장 중요한 정정 — "SB 암 길이 400"은 성립 안 하는 말이다.**
`TargetArmLength`·`SocketOffset`·`TargetOffset` 셋 다 JSON에 **없다**(엔진 기본값). SB는 스프링암 길이를 안 쓰고 돌리가 위치를 통째로 정한다. 402는 스플라인 t=1의 **총 거리**다.

### 카메라 1단계 — 값 표 완성 (실행 대기)

고정 스프링암으로 SB **수평 시점**을 근사한다. BP 6개 + 코드 1줄.

| 항목 | 현재 | SB 환산 | 위치 |
|---|---|---|---|
`TargetArmLength` | 500 | **382** | BP CameraBoom ※코드 350은 BP가 덮음 |
`SocketOffset.Z` | 70 | **111~118** | BP |
`SocketOffset.Y` | 40 | 40 ✅ 일치 | — |
`bEnableCameraLag` | **false** | **true** | BP |
`CameraLagSpeed` | 10 | **19** | BP |
`CameraLagMaxDistance` | 0 | **57** | BP |
회전 랙 | false | false ✅ SB도 안 씀 | — |
`ProbeSize` | 12 | **10** | BP |
`ViewPitchMin` | -80 | **-89** | `KDPlayerCameraManager.cpp:12` |
`ViewPitchMax` / FOV | 45 / 75 | 45 / 75 ✅ | — |

> **`SocketOffset.Z` 48cm 차이가 핵심.** 거리만 줄이고 높이를 안 올리면 시점이 안 맞는다.
> 캡슐 보정 — SB 95/40, 우리 88/34라 Z는 **111** 근처가 실제로 맞다.

### 2단계 스플라인 돌리 — 실현 가능 확인됨

엔진 소스 확인: `SpringArmComponent.h:171 UpdateDesiredArmLocation()`, `:177 BlendLocations()` **둘 다 `protected virtual`**. 시프트업이 `SBSpringArmComponent`로 한 게 정확히 이 override다.
분량 = 클래스 1개(150~200줄) + 스플라인 1개. **우리는 레일 13개가 필요 없다** — SB는 수영·비행·외줄타기가 있어서 그렇고, 지상 액션은 기본 1개면 된다.
⚠️ `AKDPlayerCameraManager::UpdateViewTarget`이 이미 POV를 손댄다 — 어느 쪽이 최종인지 정하고 시작할 것.

**순서**: 1단계 값(30분) → 기준선 확보 → 돌리. 돌리를 먼저 만들면 비교 기준이 없다.

---

### ★ 사격 로직 — 사전조사 완료 (착수 대기)

**① 우리한테 이미 원거리 뼈대가 있다**
`Combat/KDProjectile.h` + `Abilities/Enemy/GA_EnemyRangedAttack.h`. 적 투척용으로 만든 것이고 **플레이어 총이 그대로 재사용 가능**하다(`AKDProjectile`이 `Abstract` = BP child로 비주얼·속도 지정).

**② 히트스캔이 아니라 발사체로 간다** (결정)
- `AKDProjectile` 재사용 = 0줄
- 퍼펙트 회피가 `bPerfectDodgeable`로 **발사체**를 인식한다 — 히트스캔이면 이 경로가 죽는다
- SB 드론건도 탄이 보인다

**③ 3인칭 함정** — 크로스헤어는 화면 중앙, 총구는 어깨 옆. `카메라에서 트레이스 → 조준점 확정 → 총구에서 재조준`이 필요하다. 근거리에서만 티가 나서 나중에 발견하게 되는 종류.

**④ GAS 타게팅은 대부분 불필요** — `WaitTargetData`/`TargetActor`/예측 스폰/Ability Batching은 전부 **멀티 예측 때문에** 있는 복잡도다. 싱글이라 GA 안에서 트레이스하고 스폰하면 된다.

**⑤ SB 방식 (키맵 캡처로 확정)**

```
Ranged Mode  = Ctrl        (패드 L2 홀드. PC판 토글/홀드 여부는 미확정)
Ranged Fire  = 좌클릭
Select Ammo  = 우클릭
```
설계 의도 = **총은 딜링이 아니라 근접의 준비 도구**. 실드 깎기 / Repulse 후 약점 노출 / 거리 벌리기 / 락온 해제.

**⑥ 팩 애니 인벤토리 — 예상보다 훨씬 많다** (이전 "6클립"은 틀린 기억)

```
01_Idle/05_Offset_Aim/        AimOffset 9포즈 그리드 (중앙+상하좌우+대각4)  ← 교과서 그대로
02_Attack/09_Normal_Attack/   Aim_the_Target_{Start, Shoot, Walk_Shoot, Run_Shoot, End}   5
02_Attack/06_Combo_Attack_Shoot/      01~04 + _All      5   ← 총 콤보 4타
02_Attack/08_Combo_Attack_Air_Shoot/  01~04 + _All      5
02_Attack/11_Dodge_Shoot/     Dodge_Shoot ×4 + Dodge_Air_Shoot ×4          8
03_Walk/04_Walk_Aim/          8방향 × (Start/Stop_L/Stop_R) × RM·IP
04_Run/03_Run_Aim/            8방향 × 동일
09_Turn/04_Turn_Aim/          90_L, 90_R, 180_L, 180_R                     4
```

**⑦ 배선안 — 홀드 조준(SB식) 권장**

```
IA_Aim 홀드 시작 → GA_AimMode → State.Combat.Aiming 부여
                   Aim_the_Target_Start (상체) / 이동속도 감소
                   MM Chooser에 Aiming 컬럼 추가 → Walk_Aim / Run_Aim DB로 전환
IA_Aim 해제      → 태그 제거 + Aim_the_Target_End → 검 스탠스 복귀
IA_Fire (Aiming 중) → GA_Shoot → Shoot / Walk_Shoot / Run_Shoot → 노티에서 발사체 스폰
```

**이 방식을 권하는 이유**: MM DB 선택이 이미 Chooser 테이블이라 조건 컬럼 하나로 조준 로코모션이 **C++ 0줄로** 들어온다. 검 콤보 트리도 안 건드린다.
`Combo_Attack_Shoot` 4타는 **스탠스 전환식**(총 스탠스 독립)으로 갈 때 쓴다 → `DA_GunComboTree`가 하나 더 필요해지므로 **나중**.

**⑧ 애니 함정** — 몽타주와 AimOffset이 싸운다. 몽타주 Additive를 mesh space로 두면 몽타주는 나오는데 AimOffset이 깨지고, 끄면 반대가 된다. 발사 몽타주를 AimOffset 위에 얹을 때 여기서 한 번 막힌다.

---

### 발사체 리팩토링 3건 (발견만, 착수는 총 작업과 함께 — 승환 지시)

| # | 문제 | 왜 위험한가 |
|---|---|---|
1 | **델리게이트 바인딩이 `InitProjectile` 안에 있다** | 안 부르면 발사체가 벽도 적도 통과하고 5초 뒤 조용히 소멸. **에러도 로그도 없다.** 두 번 부르면 GE 2중 적용. 생성자/`BeginPlay`로 옮길 것 |
2 | **`GA_Dodge`가 발사체 발사자를 안 본다** | `IsPerfectDodgeable()`만 검사 → **플레이어 총 도입 시 내가 쏜 총알로 퍼펙트 회피가 뜬다.** 지금은 발사체가 전부 적 것이라 안 터졌다 |
3 | faction 게이트가 Enemy 한쪽만 | "적→적 통과"만 있고 플레이어 대칭 없음. 소환수·동료 생기면 터진다. 급하지 않음 |

곁가지 — `InitProjectile`에 **방향을 정하는 줄이 없다**. `ProjectileMovement`가 스폰 회전을 그대로 쓰기 때문에 스폰 시 회전을 맞춰야 한다. 적은 몽타주가 적을 향해서 우연히 맞았다. **플레이어 총은 여기가 확장 지점.**

---

### SB에 있고 우리에 없는 것 (Eve BP 컴포넌트 전수 대조)

> ⚠️ **비교의 함정** — Eve BP 함수 43개에 **전투 로직이 하나도 없다**(발소리·머리카락·물·유체·얼굴뿐). SB 전투는 통째로 C++이고 덤프에 코드가 없다. 그러니 컴포넌트 목록 비교로 "우리가 뒤졌다"고 읽으면 안 된다 — 우리가 컴포넌트로 노출한 것(`ComboComponent` `LockOnComponent` `InputBuffer` 등)을 SB는 C++에 감춘 것뿐이다. **전투 시스템의 공백은 이 덤프로 알 수 없다.**

| 항목 | 내용 | 판단 |
|---|---|---|
**카메라 스플라인 돌리 13종** | 위 참조 | **1순위** |
**콜리전 4겹 분리** | `CollisionCylinder`(이동, `ECC_GameTraceChannel11` + Pawn **Ignore**) / `BodyCollision`(피격) / `WeaponPhysicsCollision`(Box) / `CameraOverlap`(Box) / `SBCollisionGroupComponent`. 우리는 캡슐 1개 | 3순위 |
**발소리** | `FootStepL/R` + `_Reset` + `CheckPhyMat`(물리재질). **우리는 사운드가 전무하다** | **2순위** |
메시 7분할 | Face/Hair/Ponytail×2/AccSlot×2/Body. 의상·헤어 교체 | 스코프 밖 |
햅틱 2채널 | `RainHaptic` / `TachyModeVBComp`. 비 맞는 진동이 따로 있다 | 스코프 밖 |
환경 상호작용 | 물 깊이 / FluidFlux ×3 / 바람 / `AddBlood` | 스코프 밖 |
기타 | 씬캡처(포토모드) / `AssistanceSlot`(드론 슬롯) / JALI 페이셜 / `getThighAngles` | 스코프 밖 |

**콜리전 분리는 우리 구조상 쉽다** — `AT_MeleeTrace.cpp:86`이 `ECC_Pawn` **ObjectType** 기반이라, 피격 캡슐을 Pawn 타입으로 새로 달고 이동 캡슐을 커스텀 채널로 옮기면 **트레이스 코드는 그대로**다. 단 `GA_Dodge` 반경 오버랩·`LockOnComponent`·적 캐릭터를 같이 확인해야 한다.

### 2026-07-31 커밋

```
Content repo (로컬)
  4f1ac60 [VFX]    무기 트레일 NS 배정 - Distortion_Only_Loop  (25 파일)
  e542cb3 [Camera] SB 카메라 1단계 값 이식 - 붐 6개 + FOV 커브 재조정
```

문서·메모리 갱신(커밋 대상 아님): 옵시디언 `StellarBlade_Player_Analysis.md §4-6` 정정 5군데 / 메모리 `reference_sb_camera_data_2026-05-27` · `reference_control_scheme` 갱신

---

## 2-0. 🟡 진행 중 — 카메라 (1단계 ✅완료 / 2단계 승인 대기)

> **새 세션은 여기부터 이어라.**

### 확정된 작업 순서 (승환)
```
1. 플레이어 카메라   ← 1단계 끝, 2단계 코드 승인 대기
2. 히트스톱
3. 총 발사 로직
```

### 카메라 1단계 — ✅ 전부 완료 (2026-07-31)

| | 항목 | 결과 |
|---|---|---|
✅ | `KDPlayerCameraManager.cpp:12` `ViewPitchMin` -89 | 완료 + 빌드 통과 |
✅ | BP_SBPlayer → CameraBoom 값 6개 | **CDO 조회 결과 이미 반영돼 있었다** (382 / Z111 / Lag 19·57 / Probe 10) |
✅ | `CF_FovByCamDist` 키 | **문제 있었고 고쳤다** (아래) |
✅ | PIE 검증 | 승환 "큰 이상 없음". 랙 마커 정상 동작 확인 |

**커밋** — Content `e542cb3` `[Camera] SB 카메라 1단계 값 이식`

#### FOV 커브가 실제로 문제였다

| | 키 | 거리 400에서 |
|---|---|---|
| 이전 | 100→40 / 200→50 / 350→65 / **500→75** | **67.6** ← 답답 |
| 현재 | 113→40 / 140→40 / **300→75** | **75.0** ✅ |

암 길이를 382로 줄이자 실제 카메라 거리가 `√(382²+40²+111²) ≈ 400`이 됐는데, 커브 마지막 키가 500이라 400에서 FOV 75에 도달하지 못했다. **SB 실측 3키로 교체해서 해결.**
⚠️ `float_curve`는 Python에 노출 안 됨 → **커브 편집은 에디터에서만 가능**(MCP 불가).
곁가지 — 113~140 구간에 0.75도 언더슛(39.25)이 있다. 무시 가능 수준이라 그대로 둠. 없애려면 140 키를 Smart Auto 또는 User+탄젠트 0으로.

#### 남은 것 하나 — 발이 안 보인다

**원인 확정.** 두 변경이 같은 방향으로 겹쳤다.

```
카메라 높이 = 88(캡슐) + 111(SocketOffset.Z) = 199cm
수평 거리   = √(382² + 40²) = 384cm
발까지 각도 = atan(199/384) = 27.4° 아래
화면 하단   = 23.35° 아래     ← FOV 75(가로) → 세로 46.7°의 절반
              27.4 > 23.35  →  지면 위 33cm부터 잘림
```

이전(암 500 / Z 70)엔 발 아래 58cm 여유가 있었다. 총 91cm어치가 움직였다.

**진짜 원인 = SB의 "위치"만 베끼고 "시선"을 안 베꼈다.** SB는 같은 자리에서 **17.1° 아래**를 본다(`atan(118/383)`). 스프링암은 피치 0이면 정직하게 수평을 본다.

→ **2단계 레일에서 자동 해결된다**(레일 = 위치 + LookAt, LookAt이 곧 기울기). 승환 판단으로 임시 처방 없이 2단계로 넘어간다.
임시로 필요하면 `FollowCamera` 회전 Y에 **-8** 하나면 된다 (`bUsePawnControlRotation = false` 실측 확인).

**BP 값 6개** (`Content/SB_Style_GameProject/Player/BP_SBPlayer` → CameraBoom):
```
📁 Camera
   Target Arm Length              500 → 382
   Socket Offset  Z                70 → 111      ★ 이게 핵심. 거리만 줄이면 시점 안 맞음
   Socket Offset  Y                40    유지

📁 Camera Collision
   Probe Size                      12 → 10

📁 Lag                                            ★ 체감 제일 큼
   ① Enable Camera Lag             ☐ → ☑         먼저 체크해야 아래 활성화
   ② Camera Lag Speed              10 → 19
   ③ Camera Lag Max Distance        0 → 57
   Enable Camera Rotation Lag      ☐ 유지        SB도 안 씀. 켜지 말 것
```

### ★카메라 랙 검증법 (2026-07-31 확립 — 재사용할 것)

랙은 지수 감쇠라 **공식으로 예측하고 화면과 대조**한다. 시정수 = `1 / LagSpeed` = 0.053초.

| 공식 | 계산 |
|---|---|
| 등속 뒤처짐 | **속도 ÷ LagSpeed** |
| 정지 후 수렴 | **ln(오차 ÷ 목표) ÷ LagSpeed** |
| 클램프 발동 속도 | **MaxDistance × LagSpeed** |

우리 속도(`SprintComponent.h:32-41` Walk 250 / Jog 500 / Sprint 700 / FullSprint 800) 대입:

```
FullSprint 800 → 뒤처짐 42.1cm
급정지        → 5cm까지 0.112초 / 1cm까지 0.197초
클램프 57     → 57 × 19 = 1083 cm/s 에서만 발동 → 일반 이동으로는 영영 안 걸림
```

⚠️ **`CameraLagMaxDistance 57`은 지금 사실상 죽은 값이다.** 대시·회피(루트모션 순간속도)에서만 빨강선이 뜬다. 승환이 빨강을 못 본 게 정상.

⚠️ **SB의 19/57은 최고속 500 기준 튜닝값이다.** 우리는 800이라 60% 더 뒤처진다(26.3 → 42.1). SB와 같은 체감을 원하면 `LagSpeed ≈ 30`. **지금은 그대로 두고 체감으로 판단하기로 함.**

**디버그 마커 정체** (엔진 `SpringArmComponent.cpp:169-176`):
🟢 초록 = `ArmOrigin`(랙 없는 진짜 기준점) / 🟡 노랑 = `DesiredLoc`(랙 걸린 것) / 화살표 클램프 시 빨강

**PIE에서 볼 것 3가지**

| | 동작 | 왜 |
|---|---|---|
| 1 | **옆걸음(스트레이프)** ★ | 랙이 화면에서 보이는 **유일한** 동작. 화면 가로 절반 294cm 기준 42cm = **14% 밀림** |
| 2 | 급정지 | 오버슛(되튕김) 검사 |
| 3 | 대시·회피 | 클램프 실동작 |

⚠️ **앞뒤 달리기로는 판단 불가** — 카메라가 42cm 밀려도 거리만 384→426으로 변하고 캐릭터는 화면 중앙 그대로다.
⚠️ **회전으로도 판단 불가** — 랙은 `ArmOrigin` 위치에만 걸린다.
✅ **프레임레이트 검증은 이미 통과** — `bUseCameraLagSubstepping = true` / `MaxTimeStep 0.0167` 실측 확인.

---

## 2-0-B. 🔴 2단계 = 스플라인 돌리 — **설계 확정, `.h`/`.cpp` draft 승인 대기**

> **여기가 재개점.** 코드는 아직 파일로 안 만들었다. draft는 이 문서 아래 + 세션 로그에.

### ⚠️ 걱정하던 충돌은 없었다

`AKDPlayerCameraManager::UpdateViewTarget`이 POV를 손대는 건 맞지만 **FOV만** 건드린다(`:55`, `:70` 둘 다 `ViewTarget.POV.FOV`). 위치·회전은 안 만진다.
→ **역할이 이미 갈려 있다.** 붐 = 위치·회전 / 카메라매니저 = FOV. 그대로 둔다.

### 핵심 설계 — 부모 함수 복사 0줄

`UpdateDesiredArmLocation`을 오버라이드하면 부모 90줄을 복사해야 한다(바꿀 건 위치 계산 2줄인데 함수 한가운데 있음). **복사를 피하는 길을 찾았다.**

```
bInheritPitch = false   →  DesiredRot의 피치가 0  →  DesiredRot.Vector()가 항상 수평
                        →  TargetArmLength = 순수 수평거리
                           SocketOffset.Z  = 순수 높이
                        →  레일 점이 그대로 이 두 값으로 표현됨
```

매 틱 두 값만 갈아끼우고 `Super::TickComponent()`를 부르면 **랙·클램프·벽 충돌은 부모가 처리**한다.
시선(LookAt)은 `GetSocketTransform()`이 `virtual`이라(헤더 `:153`) 회전만 가로챈다.

```cpp
class UKDSpringArmComponent : public USpringArmComponent
{
    TObjectPtr<USplineComponent> DollySpline;   // 캐릭터가 주입

    TickComponent() override
    {
        ApplyRailPosition();       // 피치 → 진행도 → 레일 위치 → 붐 값
        Super::TickComponent();    // 랙 | 충돌 | 소켓 갱신
        UpdateLookRotation();      // 새 위치 기준 LookAt
        UpdateChildTransforms();   // 카메라에 반영
    }
    GetSocketTransform() const override;   // 위치는 부모 / 회전만 교체
};
```

**약 90줄.**

### 파일 목록 (승인 대상)

| | 경로 | 클래스 |
|---|---|---|
| 신규 | `Source/Project_KD/Player/KDSpringArmComponent.h` | `UKDSpringArmComponent : USpringArmComponent` |
| 신규 | `Source/Project_KD/Player/KDSpringArmComponent.cpp` | — |
| 수정 | `KDPlayerCharacter.h:18` | 전방선언 2개 교체·추가 |
| 수정 | `KDPlayerCharacter.h:64-66` | 붐 타입 + `CameraDollySpline` 멤버 |
| 수정 | `KDPlayerCharacter.cpp:5` | include |
| 수정 | `KDPlayerCharacter.cpp:48-53` | 붐 클래스 교체 + 스플라인 생성/주입/시드 |

**궤도 점 시드** (엔진 기본 스플라인은 점 2개라 3개로 교체):
```cpp
CameraDollySpline->ClearSplinePoints(false);
CameraDollySpline->AddSplinePoint(FVector(  -1.f,  0.f, 514.f), ESplineCoordinateSpace::Local, false);
CameraDollySpline->AddSplinePoint(FVector(-382.f, 40.f, 111.f), ESplineCoordinateSpace::Local, false);
CameraDollySpline->AddSplinePoint(FVector( -53.f,  0.f, -83.f), ESplineCoordinateSpace::Local, false);
CameraDollySpline->UpdateSpline();
```

### 결정 사항

- **스플라인 = 엔진 기본 `USplineComponent`.** 커스텀 안 만든다 — **SB조차 커스텀을 안 만들었다**(`CameraDollySpline` 클래스가 `SplineComponent`). 로직 붙는 쪽(붐)만 상속
- **캐릭터 직속으로 단다.** 붐 안에 숨기면 뷰포트에서 레일을 눈으로 보고 점을 못 끈다
- **`CameraBoom` 이름 유지.** 클래스만 교체 (이름 바꾸면 BP 디테일 빈 화면 함정)
- **`TargetArmLength 350` / `SocketOffset 70` 두 줄 삭제** — 매 틱 궤도가 덮어써서 죽은 값
- **궤도는 1개만.** SB 13종은 수영·비행·외줄타기 때문
- **`DollySpline`이 비면 전 구간 폴백** → 일반 스프링암 동작. 주입 전에도 안 깨짐

### 착수 시 확인할 실패 지점 2개

1. **`GetSocketTransform` 가로채기가 안 먹을 가능성.** `UpdateChildTransforms()`가 소켓 트랜스폼을 거친다는 전제인데, 엔진이 캐시 경로로 우회하면 카메라 회전이 안 바뀐다 → 대안 = `FollowCamera`에 상대 회전 직접 세팅
2. **`TargetArmLength`가 0이면 벽 충돌 검사가 통째로 꺼진다**(`:191 if (bDoTrace && TargetArmLength != 0)`). 레일 t=0의 X가 -1.2라 1.2로 살아 있지만, 점을 튜닝하다 0을 만들면 카메라가 벽을 뚫는다

### 나중 (지금 하지 말 것)

- `ZOffsetArmLengthCurve` (암길이 20→Z+12 / 60→+30 / 300→0) — 벽에 껴서 당겨질 때 위로 띄워 몸통 관통 방지. 레일 안정화 후
- **나머지 커브는 만들지 마라** — Yaw자동회전·락온 3종은 읽을 코드가 없다. `SlopeControlCurve`가 이미 "슬롯만 있고 에셋 없는" 반면교사

### 우리 커브 현황 (3개)
`CF_FovByCamDist`(거리→FOV) · `CF_FovPunchCurve`(시간→FOV델타) · `CF_TurnSpeedByVelocity`(속도→회전속도)
**셋 다 카메라 "위치"와 무관하다.** 레일은 위치를 정하는 별개 축 — 레일이 생기면 이미 있는 FOV 커브가 자동으로 살아난다.

---

## 2-1. 다음 작업 = 히트스톱 (카메라 끝난 뒤)

**문제**: 다단히트 노드에서 히트스톱이 덜컥거린다.

```cpp
// GA_PlayerMeleeAttackBase.cpp:29-37
FTimerHandle TH;                    // ← 지역 변수. 핸들을 안 들고 있다
Char->GetWorldTimerManager().SetTimer(TH, ...Resume..., Duration, false);
```
호출마다 독립 타이머가 쌓여 `Pause→Resume→Pause→Resume` 스터터. **공격자 몽타주도 멈춘다**(`:66`)라 스윙 애니가 망가진다.
주석에 이미 흔적 있음 — "SetPlayRate(0) 대신 Pause — 겹친 두 번째 타격이 rate=0을 물어 영구 정지하는 것 방지". **한 번 데인 자리다.**

**원인**: `GA_MeleeTraceBase.cpp:88-89`가 **판정창마다 `AlreadyHitActors.Reset()`**. 주석에 의도가 적혀 있다 — "한 몽타주의 2연타가 같은 적에게 각각 들어가게". **버그가 아니라 설계.**

**SB 실측 답** (`SkillActiveStepTable.json` 6,958행 전수):
- 히트스톱 관련 필드 = **`bIgnoreHitStop` 불리언 하나뿐.** 수치 필드 없음 → 지속시간은 코드에서 일괄
- **true 2,284 (33%) / false 4,674**
- Eve 검: true 53 / false 636. **true인 것들이 전부 `_Hit1`/`_Hit2` 쌍 = 다단히트 스텝**
- → **SB는 한 공격에 판정이 2개 이상이면 히트스톱을 끈다**
- 화면 슬로우(TimeScale 0.1 × 0.5초)는 **저스트 패링 전용.** 일반 콤보 히트엔 안 걸림
- 피격 경직 **0.3초 고정**(1~4타 동일), 넉백만 피니셔 2배(100→200) — **시간이 아니라 공간으로 콤보 제어**
- 곁가지: `CH_P_EVE_08_HitStop_CtrlRig` 존재 → SB 히트스톱은 몽타주 정지 + **컨트롤 리그 변형**까지 물려 있음

**해법**: **(b) 재진입 가드 권장** — 타이머 핸들을 멤버로 들고, 진행 중이면 새 요청 무시. (a) `bIgnoreHitStop` 플래그 이식은 나중에 연출 요구 생기면.
**데미지 계수는 이거 고친 뒤에.** 순서가 그렇다.

---

## 2. 즉시 다음 ★ 최우선

### ✅ ① GameplayCue 스캔 경로 — 적용 완료 (소스 `a54a6a0`, PIE "잘됨")

아래는 원인 기록. 같은 종류가 또 나오면 `GameplayCue.PrintGameplayCueNotifyMap`부터.

**이펙트가 안 나오는 진짜 원인.** 에셋은 다 있는데 큐 매니저가 그 폴더를 안 뒤진다.

```
Config/DefaultGame.ini:17-19  현재
  /Game/Blueprints/AbilitySystem/GameplayCueNotify   ✅ 존재 (여기 3개만 살아 있다)
  /Game/Blueprints/Player/GC                          ❌ 폴더 없음 (길동 잔재)
  /Game/TrickalFanGame/GAS/GC                         ❌ 폴더 없음 (버터 잔재)

우리 큐 6개 실제 위치 = /Game/SB_Style_GameProject/GAS/GC/   ← 목록에 없다
```

`GameplayCue.PrintGameplayCueNotifyMap` 실측 결과 — **연결 3개 / 미연결 9개**:

| 상태 | 태그 |
|---|---|
✅ 연결 | `Enemy.Telegraph`(+3리프), `Combat.ParryClash`, `Combat.HitImpact.Light` |
❌ unmapped | `Combat.PerfectParry.SlowMo`, `Combat.PerfectDodge`, `Combat.PlayerHitConfirm`, `Combat.CounterThrust`, `Combat.HitReact`, `Combat.Staggered`, `Combat.Execution`, `Camera.Execution`, `Camera.DashTrail` |

**고칠 것**: `DefaultGame.ini`에 `+GameplayCueNotifyPaths=/Game/SB_Style_GameProject/GAS/GC` 추가, 없는 폴더 2줄 삭제. 옛 폴더 줄은 살아 있는 큐 3개가 아직 거기 있으니 **남긴다**.

**에디터 재시작 필요** (이 목록은 `AbilitySystemGlobals` 초기화 때 1회만 읽는다).
검증 = 재시작 후 같은 콘솔 명령 → `unmapped`가 숫자로 바뀌는지.

> 미확정: 각 GCN의 `GameplayCueTag`를 조회하니 6개 다 비어 보였는데 **잘 되는 옛 3개도 똑같이 비어 보였다** → 조회 방식이 부정확. 태그 문제로 단정하지 말 것. 경로만 확정 원인이고, 재시작 후에도 unmapped면 그때 BP 디테일에서 태그를 직접 박는다.

### ✅ ② 입력 버퍼 창 확대 — 적용 완료 (2026-07-30)

`BP_SBPlayer` → `InputBuffer` 컴포넌트 디테일에서 `BufferTimeWindow` 0.2 → **0.5**, `MaxBufferSize` 4 → **2**. 코드 수정 0줄(둘 다 `EditAnywhere`, `ClampMax`가 정확히 0.5).

지상 콤보 15개 중 8개에서 입력이 증발하던 문제 → **12개 해결.**
**남은 3개**: `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74)는 `ANS_CancelWindow`가 너무 늦게 열려 0.5초로도 못 덮는다 → **노티를 앞으로 당길지 별도 판단.**

---

## 3. 보류 목록 (우선순위 순)

| # | 항목 | 내용 |
|---|---|---|
1 | **`DA_ComboTree` 값 채우기** ★ | **두 값의 상태가 다르다 — 헷갈리지 말 것** (2026-07-31 A레인 지적으로 정정)<br>· **`InputWindow`** = **칸 있음 / 값 전부 0** → 아직 `ComboResetTime 1.5f` 공용값으로 돈다<br>· **`DamageMultiplier`** = **칸 자체가 없다.** `.h` 실측 확인 — DA를 열어도 그 칸은 안 보인다. `FComboNode`에 추가부터 해야 함(`InputWindow` 바로 아랫줄, 같은 형식)<br>SB 입력창 실측: 1~2타 0.7~0.8 / 3~4타 0.9~1.2 / 마무리 1.4~2.0 / 회피 0.8 / 저스트회피 1.5<br>⚠️ **DA는 2개다** — `DA_ComboTree` + `DA_AirComboTree`(같은 `FComboNode` 구조)<br>⚠️ `FComboNode`에 **`DamageEffectClass`(노드별 GE)가 이미 있다** — 계수를 float으로 넣을지 노드별 GE로 갈지 먼저 정할 것. 26노드 × 개별 GE = 에셋 26개라 **float 계수가 가볍다** |
2 | **캔슬 윈도우 늦은 몽타주 3개** | `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74). 버퍼 0.5초로도 못 덮는다. `ANS_CancelWindow`를 앞으로 당기는 게 유일한 해법 — 단 안무 자체가 후딜이 긴 동작일 수 있어 포즈 재확인 필요 |
4 | ~~발사체 리팩토링 3건~~ **2/3 이미 닫힘** (2026-08-13 실측) | ① 델리게이트 바인딩 → `KDProjectile.cpp:53~54`에서 **`BeginPlay`로 이미 이동됨**(08-10 `b36c1c2`) ✅<br>② `GA_Dodge`가 발사자를 안 봄 → `GA_Dodge.cpp:198`에 **`&& Proj->GetInstigator() != Avatar` 이미 있음** ✅<br>③ faction 게이트 비대칭 → `KDProjectile.cpp:72~77`이 "적→적 통과"만 검사. **살아 있으나 소환수·동료가 생겨야 터진다. 급하지 않음**<br>곁가지 = `InitProjectile`에 방향을 정하는 줄이 없다(스폰 회전을 그대로 씀). 버그가 아니라 현재 설계 |
5 | `EnterNode`가 `Context`를 안 받는다 | 트리를 지상→공중 순차 조회로 우회 중. 노드 ID가 안 겹쳐서 지금은 확실하지만, 겹치는 ID가 생기면 깨진다 |
6 | `OnInActionTagChanged` 재호출 | GA가 겹치면 `NewCount` 1→2로 재호출. 같은 소켓 재부착이라 결과 동일. **제약**: `AttachWeaponToHand()`에 1회성 작업(사운드·이펙트) 넣지 말 것 |
7 | **트레일 NS 변수 검증** | `SwordLength`/`TrailWidth`가 새 NS에 먹는지 PIE 확인. 안 먹으면 NS User Parameter 이름을 맞춰야 함 |

### 2026-08-12 신규 보류 (dev-log `2026-08-12-input-component-and-muzzle-socket.md` 참조)

| # | 항목 | 내용 |
|---|---|---|
8 | ~~`AM_SB_Combo_05_03` 첫 `Shot` 노티 각도~~ **닫힘** (2026-08-13 MCP 실측) | **5개 전부 `muzzleDir=True` / `halfAngle=180` / `ignoreHitStop=True`로 이미 통일돼 있다.** "1번만 10, 나머지 179"는 지나간 기록<br>**총격 노티 전수 = 13개 몽타주 20발.** 05_03(5발)만 위 설정이고 **나머지 12개는 전부 기본값**(`muzzleDir=False` / `halfAngle=0`→GA 값 20도 / `ignoreHitStop=False`)<br>→ 08-13에 붙인 총격 자동 조준은 **12개 몽타주 15발에 먹는다.** 05_03은 각도 180(=전방위)이라 방향이 판정에 영향 없음 |
9 | ~~360° 콘이면 한 적이 5번 맞는다~~ **의도로 확정** (2026-08-13 승환) | 발당 데미지 분배 **안 한다.** 5연타 전방위가 의도. `ignoreHitStop=True`도 그 의도와 맞물림(SB도 다단히트 스텝은 히트스톱을 끈다). `ShotRange` 500도 유지 |
10 | **디버그 구체 그리기 미적용** ★ | 각도 90° 이상이면 `DrawDebugSphere`로 대체. **05_03이 정확히 그 경우라 지금 그 판정 범위를 눈으로 볼 방법이 없다**(`DrawDebugCone`이 180°에서 뒤쪽 한 점으로 뭉쳐 바늘로 보임 — `LineBatchComponent.cpp:515~546`). 판정은 정상, 그리기만 문제<br>코드 = `GA_ShotBlast.cpp:107`의 `ConeRad` 선언을 `else` 안으로 옮기고 `if (HalfAngle >= 90.f) DrawDebugSphere(World, Origin, ShotRange, 24, ...)` 분기. include 불필요(`:163`에서 이미 사용) |
11 | **일반 공격 자동 조준(미착수)** | 락온 안 걸었을 때 가장 가까운 적 쪽으로 자동 회전. `ULockOnComponent::FindBestTarget()`이 public이라 재사용 가능 — `GA_PlayerMeleeAttackBase::OnActivated`의 락온 게이트만 바꾸면 됨. 미결 = 카메라 정면 기준(현재 동작, ±45°)이냐 스틱 입력 방향 기준이냐 |
12 | **`UKDPlayerAbilityInputComponent.cpp` 352줄** | §1 Component 300줄 선 초과. 분리 여부 미결 |
13 | **`DA_Sword_Bandit` 등 적 정의 4개 `PoiseDamageByAttack` 키 미확정 관측** | python 조회 결과 비어 보이나 조회 한계일 수 있음. 에디터에서 직접 확인 필요 |

### 닫힌 항목 (2026-08-12)

- ✅ **`ANS_EnemyAttackWindow`의 `AttackWindowTag`** — 생성자 이니셜라이저 한 줄 추가(`: AttackWindowTag(GameplayTags::State_Combat_EnemyAttackHitWindow)`). 이미 배치된 노티에도 소급 적용(CDO 기본값 직렬화 미포함). 코드 1줄로 끝났다 — 아래는 원인 기록
  - 증상은 맞았다 — 비어 있으면 퍼펙트 회피가 영원히 안 뜬다. 형제 클래스 `ANS_CancelWindow`가 이미 생성자에서 기본값을 넣는 패턴인데(`CancelTag(GameplayTags::State_Combat_CanCancel)`) 이쪽만 빠져 있었다

### 닫힌 항목 (2026-07-30~31)

- ✅ **`GA_Dodge` 레벨 전체 순회 → 반경 물리 조회** (`84b57af`). 채널 2개(Pawn + `ECC_GameTraceChannel1`) 필수 — 발사체는 Pawn이 아니다. `EnvQueryContext_AllyEnemies`는 **고칠 필요 없다**: 타입 지정 순회 + "아레나 3~12마리라 가볍다"고 파일 주석에 판단 근거가 있다
- ✅ **`ANS_WeaponTrail` 액터별 분리** (`84b57af`). 노티 객체는 몽타주 에셋 소속 1개라 모든 액터가 공유 → 멤버 대신 `TMap<MeshComp, Trail>`
- ✅ **`PerfectDodgeWindowSec` 삭제** (`84b57af`)
- ✅ **`FComboNode.InputWindow` 칸 + 배선** (`c0ccc9e`). `ProcessInput` 타이머를 노드 결정 후로 이동, `EnterNode` 우선순위 3단계
- ✅ **입력 버퍼 0.5 / 2** (Content `60698c5`)
- ✅ **`AirComboResetTime` 폐기** (2026-07-31) — `DA_AirComboTree`도 같은 `FComboNode`라 **`Air_1~4` 각각이 이미 `InputWindow` 칸을 가진다.** 컴포넌트 레벨 노브보다 정밀하고 코드 0줄. 항목 자체가 사라졌다
- ✅ **트레일 NS 27개 배정** (Content `4f1ac60`)

### 닫힌 항목

- ✅ **`PerfectDodgeWindowSec` 삭제** (2026-07-30) — 참조 0건인 죽은 값. 실제 퍼펙트 판정은 시간이 아니라 **상태**(적 ASC의 `State.Combat.EnemyAttackHitWindow` 태그). ⚠️ `PerfectDodgeCheckRadius`(500)는 살아 있다
- ✅ **`Block_End` = A안(연결 안 함)으로 확정** (2026-07-30 승환) — C++·BP 그래프 전수 확인 결과 재생 경로가 없고, 지금 자연스럽게 보이는 건 **ABP 블렌드 아웃(0.25초)**이다. 연결하면 가드를 놓고 0.6초간 묶여 "가드 풀었는데 못 움직인다"는 새 문제가 생긴다. SB도 블렌드로 처리
  - 뒤집을 때 방법 3개: ① 짧은 전용 GA ② `GA_Parry` 종료를 몽타주 끝까지 지연 ③ GA 없이 `AnimInstance`에 직접 재생(가장 단순, 순수 연출이라 판정 없음 — 단 §1-3 의존성 방향 확인)

---

## 4. 설계 미결 — 판단 대기 ★

코드가 아니라 **판단이 필요한** 것들. 착수 전에 승환이 정해야 한다.

### ① 스탠스 체계

> **★2026-07-31 — 실목록과 수치를 찾았다.** `Content/Local/Data/CharacterStanceTable.json`. 스탠스가 곧 이동 속도 세트다.

| 스탠스 | Walk | Jogging | **Run** | LockOn Run |
|---|---|---|---|---|
| **`P_Eve_Default`** (평시 = 검) | 150 | 300 | **500** | **280** |
| `P_Eve_BlockSword` / `Fusion` / `Tutorial` / `AirDead` / `Freeze` | 150 | 300 | 500 | 280 |
| **`P_Eve_Tachy`** (각성) / `Fusion2` | 150 | 450 | **600** | 400 |
| **`P_Eve_Gun`** / `Gun_Gorgon` | **100** | **100** | **100** | 100 |
| `P_Eve_GunNikke` / `GunBlockSword` / `GunTutorial` | 120 | 120 | 120 | 120 |
| `P_Eve_Fishing` | 0 | 0 | 0 | 0 |

**여기서 나오는 것 3가지**
- **총 스탠스 = 100. 걷기(150)보다 느리다.** "총은 딜링이 아니라 근접의 준비 도구"라는 §1-B 해석이 수치로 확증됐다. **사격 착수 시 이 값을 그대로 쓴다**
- **락온 전용 속도가 따로 있다** (500 → 280, 44% 감속). 우리는 이 개념이 없다
- **스프린트가 없다.** SB는 150/300/500 3단계. 우리는 250/500/700/800 4단계로 **전반적으로 빠르다** — 우리 Jog(500)가 SB 최고속과 같다

SB는 `Default(=Sword) / Tachy / Fusion / Gun계열 / Fishing / 특수(사망·동결·튜토리얼)`로 나뉜다. **평시 스탠스가 없다** — Eve는 항상 무장 상태다.
우리 초안(승환): `Default(평시 전투 = Sword&Gun) / Gun(진짜 사격만) / 사망`.
→ **사격 로직 착수 전까지 보류 결정됨**(YAGNI). 사격을 시작할 때 이 표를 확정해야 한다.

> **2026-07-31 갱신 — 이제 확정할 재료가 다 모였다.** §1-B의 사격 조사 참조.
> 팩 구성이 **두 설계를 다 지원한다**: (a) SB식 홀드 조준(임시 모드, 놓으면 검 복귀) / (b) 스탠스 전환식(검·총 대등, `Combo_Attack_Shoot` 4타 사용).
> **권장은 (a)** — 검 콤보 구조를 안 건드리고 MM Chooser 컬럼 하나로 조준 로코모션이 들어온다. (b)는 `DA_GunComboTree`가 하나 더 필요하고, 그건 아직 값도 안 채운 콤보 트리를 하나 더 이고 가는 것.
> **사격 착수 전에 확정할 것.**

### ② 공중 콤보 재설계
`Air_01`이 **총 클립**이라 1타에 검 판정이 없다(`MeleeTrace` 없음이 의도).
방향: **07 유지 + 14~16 `Attack_Air_to_Floor`를 마무리로 붙여 지상 콤보로 연결**(공중 공격 → 지상 찍기 → 자연스럽게 지상 콤보).
`02_Attack` 폴더는 **전부 루트모션** — InPlace 클립이 없다. "제자리 공중 공격"은 RM을 끄는 게 아니라 **이동량 0인 클립**으로 얻어야 한다.

### ③ 스태미나 폐기
점프·달리기 자원 폐기 결정(스킬 코스트는 유지). SB 644행 실측으로 검증됨(SB의 Stamina = 적 격파 게이지). **미착수.** 소모처 3곳 값 0 + BP Cost GE 함정 주의.

### ④ 락온 애니
락온 상태 전용 애니가 보류 상태. 검 콤보 → 총 순서로 미뤄뒀다.

### ⑤ 미착수 폴리싱
~~트레일 NS 27개 미배정~~ **✅ 완료 (2026-07-31, Content `4f1ac60`)** / **사운드 노티 없음** / 데미지 GE 26노드 비어 있음 / LoP식 방사형 회피 이펙트.

> **사운드가 이제 제일 큰 구멍이다.** 트레일 27개가 붙어서 **볼 건 생겼는데 들을 게 없다.**
> 노티 자리는 이미 잡혀 있다(`ANS_MeleeTrace` 위치 그대로) — 사운드 노티를 얹는 작업이다. SB도 `FootStepL/R` + `CheckPhyMat`(물리재질 연동)으로 발소리를 따로 관리한다.

> **★ 2026-08-13 MCP 전수 실측으로 그림이 바뀌었다 — 상세 = 메모리 `reference_project_sound_inventory`**
>
> | | 실제 |
> |---|---|
> **발소리** | ❌"전부 없다" → ✅ **`/Game/MotionMatchingAnimation/Audio` 에 286개 이미 있다.** 걷기30·달리기29·스트레이프29·착지20·점프17·구르기9 + `MSS_FoleySound_*` **동작별 래퍼 13개**. **없는 건 재료가 아니라 배선.** Sonniss 없이 지금 붙일 수 있다 |
> **검 소리** | 배선 완료(08-11). `SC_Sword_Hit`→`Metal_Hit_Flesh_1~20` / `SC_Sword_Swing`→`Metal_Light_Whoosh_1~12` / `_Heavy`→`Metal_Heavy_Whoosh_1~10`. **랜덤 풀은 넉넉하다** |
> **"빈 철봉" 정체** | `Metal_Light_Whoosh` = **공기 가르는 소리만 든 팩.** 금속 울림 성분이 없다. → `SlashTrailElemental/Resource/SW_Basic_Slash`·`SW_Distortion_Slash` 를 큐 안에서 Mixer 로 겹치면 붙는다(⚠️ 미시험) |
> **총성** | **0개.** Sonniss GDC 7.47GB 필요. `Gun_and_Sword` 팩엔 사운드가 하나도 없다 |
> **그 밖 없는 것** | 피격 보이스·신음 / UI / 발검·납검 금속음 = 전부 0. 앰비언트는 MM 샘플 딸림 4개뿐 |
>
> 우리가 만든 사운드 에셋은 `/Game/SB_Style_GameProject/Audio/Combat` **큐 3개가 전부**다. `/Game/Assets/Sfx` 2개는 길동 잔재.

### ⑥ 카메라 (2026-07-31 신규)
1단계 값 표는 §1-B에 **완성돼 있다**(BP 6 + 코드 1줄, 30분). 2단계 스플라인 돌리도 실현 가능 확인됨.
**판단 필요**: 카메라를 지금 하나, 사격 뒤로 미루나. 승환은 "시점이 중요하다"고 했고, 값 이식은 30분이라 사격 전에 끼워넣을 수 있다.

---

## 5. 세션 시작 시 읽을 것

프로젝트 `CLAUDE.md §0 세션 시작 프로토콜`이 단일 진실이다. 요약:

1. `docs/PROJECT_OVERVIEW.md` — 부트 문서
2. `docs/INDEX.md` — 전체 카탈로그
3. **이 핸드오프** + 이번 작업 영역의 최신 dev-log
4. 코드 작업 세션이면 `.h` 전수 훑기(4,000줄이라 감당 가능), `.cpp`는 닿는 것만
5. **코드 설명서 = 옵시디언 볼트 `ProjectKD/notes/코드구조/`** (8문서 2,776줄) — 클래스별 용도·함수 기능·흐름·핵심 코드 발췌. `00_코드구조_MOC`부터

**에셋 값은 MCP로 조회해 확인할 것.** 코드 기본값과 다를 수 있고, 폐기된 라인의 값이 남아 있던 사례가 실제로 있었다 — 2026-07-28 공격 GA 5개가 길동 창 소켓 `Spear_Tip`을 물고 있어 근접 판정이 죽어 있었다.

---

## 6. 오늘 배운 함정 3개 (재발 방지)

1. **`.h` 선언과 `.cpp` 정의는 짝이다.** `InAction`이 `.cpp`만 있어서 빌드가 깨졌다. 태그 추가 시 양쪽 확인
2. **구독과 해제는 같은 태그로.** `RegisterGameplayTagEvent(A).Add()` 하고 `RegisterGameplayTagEvent(B).Remove()` 하면 조용히 실패한다 — 핸들 리셋은 그대로 돌아서 코드가 깨끗해 보인다
3. **`search_assets`는 패턴을 무시하고 100개를 통째로 반환한다.** 목록·집계는 `execute_python`으로. 실측: 821개 중 100개가 돌아와 1만 토큰 낭비

### 2026-07-31 추가 4개

4. **`CurveFloat`의 `float_curve`는 Python에 노출 안 된다.** `get_editor_property('float_curve')` → `Failed to find property`. **커브 키 편집은 에디터에서만.** 읽기는 `get_float_value(x)` / `get_time_range()` / `get_value_range()`로 가능하니, 값 샘플링으로 키를 역추출한 뒤 사람이 편집하는 방식이 현실적
5. **UE의 `FieldOfView`는 가로 FOV다.** 16:9에서 75 → 세로는 46.7도(아래로 23.35도뿐). "왜 발이 안 보이지"의 계산 근거가 여기
6. **핸드오프의 "미완"을 믿지 말고 실측할 것.** BP 값 6개가 이미 다 들어가 있었다. `inspect_cdo`로 30초면 확인된다
7. **스플라인 점의 회전이 시선인지 판별하는 법** — 점들의 높이가 크게 다른데 회전값이 같거나 무관하면 **그건 시선이 아니다.** 머리 위 5m와 발치 아래에서 같은 각도로 같은 대상을 볼 수 없기 때문
