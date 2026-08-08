# 2026-08-08 — 총 조준 2단계: 사격 GA + 조준선

조준 중 좌클릭 → 발사 모션. 조준 해제 몽타주. 화면 고정 크로스헤어(우리 첫 HUD).
**발사체·탄약은 아직 없다** (승환 스코프 지정 — "총알 실제 발사가 아닌 조준상태·클릭·발사모션까지").

**커밋** — 소스 `25874e4`(AimPitchScale) `f5c9b0c`(사격 GA) `263af39`(인클루드) / Content `103b7a0`
**설계 문서** — 볼트 `notes/코드구조/08_총_조준_스탠스.md` §7

---

## 팩 사격 애니 전수 (2026-08-08 실측)

`02_Attack/09_Normal_Attack` — 조준 스탠스와 같은 6개 묶음

| 클립 | 길이 | 상태 |
|---|---|---|
`AS_Aim_the_Target_Start_Seq` | 0.250s | 진입 |
`AS_Aim_the_Target_Loop_Seq` | 10.000s | `PSD_SB_Idles_Aim` |
`AS_Aim_the_Target_End_Seq` | 0.333s | **해제 (이번에 붙임)** |
`AS_Aim_the_Target_Shoot_Seq` | 0.667s | **제자리 발사 (이번에 붙임)** |
`AS_Aim_the_Target_Walk_Shoot_Seq` | 1.033s | 미사용 |
`AS_Aim_the_Target_Run_Shoot_Seq` | 0.667s | 미사용 |

전부 루트모션 X, 애디티브 X, 루트 뼈 이동 0 = **제자리**. 시퀀스에 손댈 설정 없음.

그 외 사격 계열 (나중 재료) — `06_Combo_Attack_Shoot` 4+All / `08_Combo_Attack_Air_Shoot` 4+All / `11_Dodge_Shoot` 4방향 1.667s / `11_Dodge_Air_Shoot` 4방향 / `AS_Skill_01_Shoot_Seq` 1.5s

**발사 클립이 3개(정지·걷기·달리기)인데 1개만 쓴다.** 몽타주가 `UpperBody` 슬롯이라 걷기/달리기 클립의 다리는 어차피 안 쓰인다. `Walk_Shoot`만 1.033s로 유독 길어 **셋이 대체 가능한 같은 동작이 아닐 가능성**이 크다 → 걸으며 쏴보고 어색하면 그때 DA로 승격(§3 데이터 승격 기준).

---

## 구현

| 손댄 것 | 내용 |
|---|---|
**신규** `GA_Shoot.h/.cpp` | `UGA_ActionBase` 직속, 약 50줄. 몽타주 1회 + 안전 타이머 |
`KDGameplayTags.h/.cpp` | `Ability.Mugong.Shoot` |
`KDPlayerCharacter.cpp:186` | `TryLightAttack` 앞단 사격 분기 |
`GA_AimMode.h/.cpp` | `AimEndMontage` + `OnCleanup` 오버라이드 / 진단 로그 제거 |
신규 에셋 | `AM_SB_Aim_the_Target_Shoot` `AM_SB_Aim_the_Target_End` `GA_Shoot`(BP) `WBP_Crosshair` |

### 왜 `GA_AimMode` 안에 안 넣었나

승환 질문 — "Aim이 돼야 Shoot이 가능하니 GA_Aim 아래에 두는 게 낫지 않나".

**상속은 코드를 물려받는 것이지 런타임 조건을 강제하지 않는다.** `UGA_Shoot : UGA_AimMode`로 만들어도 GAS는 둘을 따로 관리해서 부모가 꺼져 있어도 그냥 켜진다. 얻는 건 안 쓸 멤버뿐.

`GA_AimMode` 안에서 발사까지 처리하는 안도 검토했으나 —
- 팩에 **조준 없이 쏘는 애니**(`Combat_Attack_Shoot` 4개, `Dodge_Shoot` 4방향)가 이미 있다. 가두면 못 쓴다
- GAS 쿨다운·코스트는 GA 하나당 한 세트. 탄약이 붙으면 조준 자체의 쿨다운과 칸을 다툰다
- 발사체·반동이 들어오면 `GA_AimMode`가 §1의 200줄을 넘긴다

### 대신 태그로 강제한다 — 승환 직감이 찌른 구멍

원안은 `TryLightAttack`의 `if`문 하나로만 지키고 있었다. **호출자가 알아서 하겠거니 하는 구조** — `GA_Shoot` 자체는 조준 중이 아니어도 켜졌다.

| 어빌리티 | 칸 | 태그 |
|---|---|---|
`GA_Shoot` | 활성화 **필요** | `State.Combat.Aiming` |
`GA_LightCombo` `GA_HeavyCombo` | 활성화 **차단** | `State.Combat.Aiming` |
`GA_AimMode` | 활성화 **차단** | `State.Combat.Attacking` `State.Combat.Dodging` |
`GA_Dodge` | **Cancel Abilities with Tag** | `Ability.Mugong.Aim` |

`TryLightAttack`의 분기는 남겼다. **분기는 "좌클릭을 어디로 보낼지", 필요 태그는 "이 어빌리티가 켜져도 되는지"** — 앞엣것은 편의, 뒤엣것은 계약.

⚠️ 근접 차단이 `return` 하나로 끝나지 않는 이유 — `Tick`이 입력 버퍼에 남은 `Input.Action.Light`를 꺼내 `Ability.Mugong.Light`를 **직접** 켠다(`KDPlayerCharacter.cpp:463`). 그 경로는 `TryLightAttack`을 안 거치므로 BP 차단 태그가 따로 필요하다.

### 회피는 막지 않고 조준을 끈다

조준 중 회피가 나가면 `GA_AimMode`는 그대로 살아 있어 `State.Combat.Aiming`이 유지되고, **구르는데 상체가 총을 겨눈다.**
회피를 차단하면 조준 중 무방비가 되므로 **`GA_Dodge`가 조준을 취소**하는 쪽으로 갔다. 대가 = 우클릭을 계속 눌러도 회피 후 조준이 안 돌아온다(`IA_Aim`이 `Started`/`Completed` 바인딩). 굴려보고 거슬리면 `Triggered` 바인딩 추가.

### 조준 해제 몽타주 — 태스크로 못 켠다

`OnCleanup`은 어빌리티가 끝나는 중이라 `AbilityTask`가 같이 죽는다. `AnimInstance->Montage_Play`로 직접 재생한다.

엔진 확인 — `GA_ActionBase::EndAbility:22`가 `OnCleanup`을 `Super`보다 **먼저** 부르고, `Super`(`GameplayAbility.cpp:819`)가 태스크에 종료를 통보하면 `AbilityTask_PlayMontageAndWait.cpp:247`이 **ASC가 추적 중인 몽타주만** 정지시킨다. 직접 재생한 건 ASC 추적 대상이 아니라 안 꺼진다.

⚠️ **정상 해제와 피격 강제 취소를 구분할 수 없다.** 조준 해제도 `CancelByTag`라서 `bWasCancelled`가 양쪽 다 `true`. 피격 시 해제 몽타주가 히트 리액션과 겹칠 수 있다 — PIE에서 안 거슬려서 그냥 뒀다.

---

## 크로스헤어 — 우리 첫 화면 고정 HUD

`WBP_Crosshair` = Canvas Panel + Image 5개. **텍스처 0개** (브러시에 리소스를 안 넣으면 단색 사각형).

| 이름 | Position | Size |
|---|---|---|
`Dot` | 0, 0 | 4 × 4 |
`Up` | 0, -14 | 2 × 10 |
`Down` | 0, 14 | 2 × 10 |
`Left` | -14, 0 | 10 × 2 |
`Right` | 14, 0 | 10 × 2 |

앵커·정렬 전부 화면 정중앙(0.5, 0.5). `14`가 중심에서 조각까지 거리 — **나중에 여기에 발사 확산 커브를 곱한다.**

**조각을 밀어내는 방식인 이유** — SB도 같다. 이미지 하나를 스케일하면 선 굵기까지 커져 뭉갠다.

### 켜고 끄기 — 위젯이 스스로 본다

```
Event Tick → Get Player Pawn(0) → Get Ability System Component → Is Valid
           → Has Matching Gameplay Tag (State.Combat.Aiming)
           → Select (False: Collapsed / True: Visible)
           → Set Visibility   Target = Root(Canvas Panel)
```

C++ 0줄. §1-1이 **GA의 UI 직접 호출**을 금지하지, UI가 게임 상태를 읽는 건 정상이다.
ASC는 `PossessedBy`에서 폰에 캐시되므로(`KDPlayerCharacter.cpp:172`) 플레이어 폰으로 바로 얻을 수 있다.

### ★ Target이 `self`면 안 된다 — 하루 중 제일 오래 잡은 것

```
Tick 1회차 → 태그 false → Set Visibility(self, Collapsed)
                              ↓
        접힌 위젯은 Slate 레이아웃에서 빠짐 → Tick 호출 안 됨
                              ↓
        되살릴 코드가 Tick 안에 있는데 Tick이 안 돔 → 영영 못 돌아옴
```

증상 = **시작할 때 잠깐 보였다가 사라지고 조준해도 안 나옴.** 태그·배선은 전부 정상이었다.
→ 캔버스 패널을 변수화해서 **그것을** 껐다 켠다. 유저 위젯 자신은 계속 살아 있다.
(`Set Render Opacity` 0/1도 되지만 레이아웃 계산이 계속 돈다. 정석은 캔버스 쪽.)

트러블슈팅 = 볼트 `UMG-자기자신Collapsed_Tick멈춤`

---

## SB TPS 조준 UI 조사 — 덤프에 통째로 있었다

`Art/UI/Widget/Tps/` 위젯 18개 + `Art/UI/Texture/HUD/Tps/` 텍스처 28개.

**발사 확산 커브** `Default__WB_Tps_Aim_ShootlCurve` (CurveFloat, Cubic)

```
0.00초 → 1.0
0.08초 → 1.3     30% 벌어짐
0.12초 → 1.0     복귀
```

**비대칭이 핵심** — 벌어지는 데 0.08, 돌아오는 데 0.04. 조준선뿐 아니라 **반동·카메라 쉐이크 기준값**으로 쓸 것(승환 지목).

`WB_Tps_Aim`은 조준선이자 탄약 표시다 — 안쪽 5조각(76×76) + 바깥 3겹 링(190~300px, 총알 3발), 위젯 애니 9개가 전부 탄약 전환용. 우리는 탄약이 없어 링은 지금 의미 없다.

Game UI Database(`gameuidatabase.com/gameData.php?id=2039`)는 **403으로 봇 차단**. 스샷은 직접 열어야 한다.

메모리 = `reference_sb_tps_crosshair_dump`

---

## 검증 (PIE 7항목)

조준 진입/해제 ✅ / 조준 중 좌클릭 = 발사 모션 ✅ / 연타 0.667초 간격 ✅ / 비조준 좌클릭 = 검 콤보 정상 ✅ / 콤보 중 조준 차단 ✅ / 회피 시 조준 해제 ✅ / 조준선 반복 표시 ✅

조준 중 피격 애니는 **팩에 조준 전용이 없다.** `08_Hit`에 `Hit` / `Hit_Combat`(전투 자세) 두 계열뿐 — 전투 자세 것을 그대로 쓴다.

---

## 남은 것

- **발사체** — `KDProjectile.cpp:44,47`(델리게이트 바인딩이 `InitProjectile` 안) / `GA_Dodge.cpp:196`(`Proj->GetInstigator() != Avatar` 1줄)
- **탄약** — 링 UI + 소모 + 회복
- **발사 확산** — 커브값 확보됨. 탄약 작업과 같이
- **조준 대상 표시** — 트레이스 생긴 뒤
- 걸으며 사격 클립 / 좌우 조준 6포즈 / `AKDPlayerCharacter` 650줄 분리
