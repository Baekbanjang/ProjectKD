# 2026-07-30 — 길동 몽타주 전면 탈출 (단발·방어·회피·피격·공중)

GAS 어빌리티가 아직 물고 있던 **길동(TrickalFanGame) 시절 몽타주 18개**를 Gun&Sword 팩 기반으로 교체했다.
지상 콤보 20개는 이전 세션에 끝났고, 이번엔 나머지 전부.

## 결과

| GA | 이전 (길동) | 이후 |
|---|---|---|
| `GA_SprintAttack` | `AM_Run_Attack_02_Spear` | `AM_SB_Run_Attack_01` (100f) |
| `GA_CounterThrust` | `AM_Run_Attack_01_Spear` | `AM_SB_Parry_Counter_Attack_L` (95f) |
| `GA_Parry` | `AM_Block_Start/Loop/Hit/End` + `AM_Parry_L/R` | `AM_SB_Block_*` 4개 + `AM_SB_Parry_L/R` |
| `GA_Dodge` | `AM_Dodge_F/B/L/R` | `AM_SB_Dodge_Combat_F_0/B_180/L_90/R_90` |
| `GA_HitReact` | `AM_Hit_Combat_*_Butter_Montage` 4개 | `AM_SB_Hit_Combat_F/B/L/R` |
| `GA_AirLightAttack` | `AM_Combo_Attack_Air_0X_Spear` 4개 | `DA_AirComboTree` 경유 (`AM_SB_Combo_Air_01~04`) |

GAS 폴더 전체 의존성 검사에서 길동 몽타주 참조 **0건** (유령 참조 1건 제외).

## 1. 애니 선택은 본 궤적 실측으로

눈으로 고르지 않고 프레임마다 본 트랜스폼을 뽑아 판단했다.

```python
APE = unreal.AnimPoseExtensions
pose = APE.get_anim_pose_at_frame(seq, f, unreal.AnimPoseEvaluationOptions())
t = APE.get_bone_pose(pose, 'Sword_Weapon_R', unreal.AnimPoseSpaces.WORLD).translation
```

`SK_Mannequin_GunSword`에 **무기 전용 본**이 있어 검·총 궤적을 직접 잴 수 있다: `Sword_Weapon_R` / `Gun_Weapon_L` (+ 홀스터 `Sword_Holder_L` / `Gun_Holder_R`).
검·총 속도는 **골반 상대 위치의 프레임 차분**으로 계산 — 이동 성분이 빠져 휘두름만 남는다.

### 단발 후보 4종 실측

| | 동작 | 길이 | 루트모션 | 검 피크 | 채택 |
|---|---|---|---|---|---|
| `AS_Run_Attack_01_Seq` | 검 찌르기 | 100f | 앞 +521cm | 1366@f18 | ✅ SprintAttack |
| `AS_Run_Attack_02_Seq` | 슬라이딩 총공격 | 100f | 앞 +628cm | **522** | ❌ 검 안 휘두름 |
| `AS_Parry_Counter_Attack_L_Seq` | 회전 검휘두르기 | 95f | 앞 +224cm | 2949@f24 | ✅ CounterThrust |
| `AS_Parry_Counter_Attack_R_Seq` | 제자리턴 검+총발사 | 95f | **뒤 −146cm** | 3006@f7 | ❌ 워프와 충돌 |

**`Counter_R`을 버린 이유**가 숫자로 나왔다. `GA_CounterThrust`는 MotionWarping으로 **적 앞으로 당겨가는데**(`GA_CounterThrust.cpp:51`) 이 애니는 **뒤로 146cm 밀려난다.** 워프와 애니가 서로 싸운다.
`Counter_L`은 앞으로 224cm 가서 워프가 그 거리를 늘리거나 줄이면 된다.

### 자동 분류기의 한계 (검증 결과)

96개 클립에 특징 추출을 돌려봤는데, 정답 4개로 검증하니 **2개가 틀렸다.**

- 믿을 수 있음: 길이 / 루트모션 방향·거리 / 공중 여부 / 골반 높이(슬라이딩·도약) / 회전량 / 타격 구간 위치
- **못 믿음**: **검·총 판별** — 총이 왼손이라 몸이 튀어나갈 때 왼팔이 같이 흔들려 팔 흔들림과 구분이 안 된다. `RunAttack_01`은 검으로 찌르는데도 총 본 속도(2261)가 검(1366)을 이겼다
- **못 믿음**: 찌르기/베기 — 2프레임 샘플링에선 경계에서 뒤집힌다

## 2. 회피 소스에 앞 14프레임 죽은 구간이 있었다

```
AS_Dodge_Combat_F_0_Seq  100f  총 635cm
   f 0~14  ░░░░ 완전 정지        ← 0.23초 반응 지연
   f15~48  ████████████ 이동 90%
   f49~72  ██████ 감속
   f73~99  ░░░░░ 완전 정지
```

4개 전부 같았다. 안 자르면 회피 버튼 누르고 **0.23초 뒤에** 움직인다.
길동 회피가 45~64f였던 건 리타겟 시점에 이 앞뒤를 잘라냈기 때문.

**AS를 자르지 않고 AM 세그먼트의 `Start/End Time`으로 잘랐다.** 팩 원본이 안 망가지고, 숫자만 고치면 되돌려진다. 지상 콤보 22개도 전부 팩 원본을 무수정 참조하는 방식이라 일관성도 맞다.

| | Start Time | End Time | 결과 |
|---|---|---|---|
| `AM_SB_Dodge_Combat_F_0` | 0.217 | 1.250 | 62f |
| `_B_180` / `_L_90` / `_R_90` | 0.217 | 1.000 | 47f |

블렌드는 회피만 **0.10** (나머지 12개는 0.25). 길동 회피가 쓰던 값 그대로 — 회피는 블렌드가 느리면 입력이 씹힌 느낌이 난다.

## 3. 루트모션을 켜는 기준은 "이동량"이 아니다

블록·패리는 루트 이동이 **0cm**인데도 켠다.

> 루트모션 **ON + 이동량 0** = 애니가 이동을 담당하는데 애니가 안 움직임 = **그 자리에 완전히 고정**
> 루트모션 **OFF** = CMC(입력 이동)가 살아 있어서 **가드하면서 걸어다닌다.** 전신 슬롯이라 발이 미끄러진다

그래서 가드·패리는 ON, 피격은 OFF(외부 힘에 밀려야 함). 길동 쪽 설정도 정확히 그랬다.

| | Enable Root Motion |
|---|---|
| Block Start / Loop / End | ☑ |
| Block Hit | ☐ |
| Parry L / R | ☑ |
| Dodge 4개 | ☑ |
| Hit F/B/L/R | ☐ |
| Block_Hit_Break(뒤 113cm) / Hit_Death(88cm) | ☑ |

## 4. 가드 속도는 세그먼트 PlayRate로

`GA_Parry.MontagePlayRate`는 **Start/Loop/Hit/End 4개에 한꺼번에** 걸린다(`GA_Parry.cpp:69, 131, 152`). 이걸 올리면 피격 리액션까지 빨라진다.
**몽타주 안 애님 세그먼트의 `Play Rate`**로 개별 처리했다.

```
Block_Start   팩 40f  vs  길동 25f  →  PlayRate 1.6   (40f → 25f)
Block_End     팩 50f  vs  길동 35f  →  PlayRate 1.4   (50f → 36f)
Block_Hit     팩 50f  =   길동 50f  →  1.0
Block_Loop    팩 600f =   길동 600f →  1.0
```

Start/End만 짧아져 있었다 — **가드는 빨리 올리고 빨리 내린다**는 판단이 이미 한 번 내려진 셈.

## 5. 공중 콤보 — 노티가 0개라 느렸다

증상은 "공중 공격이 느리다"였는데, 원인은 `ComboResetTime`이 아니었다.

```cpp
// KDPlayerCharacter.cpp:396
if ((!bAttacking && !bDodging) || bCanCancel) { ... 다음 타 ... }
                                   ↑ ANS_CancelWindow가 붙이는 State.Combat.CanCancel
```

**공중 몽타주 4개에 노티가 하나도 없어서** `bCanCancel`이 영원히 false. `!bAttacking`만 남아 **몽타주를 100% 재생하고 GA가 끝나야** 다음 타가 나갔다.

| | 지상 `Combo_01_01` | 공중 `Combo_Air_01` |
|---|---|---|
| 길이 | 100f | 55f |
| CancelWindow | f51부터 | **없음** |
| 다음 타까지 | 0.85초 (51%) | 0.92초 (**100%**) |

4타 합계 **4.25초 → 약 2.1초**. 공중 몽타주가 지상보다 짧은데도 느린 이유가 이거였다.

그리고 `ANS_MeleeTrace`도 없어서 **때려도 판정이 없었다.**

### 배치 (C안 = 임시)

```
Air_01 55f  WeaponAttach@0 , CancelWindow 28~55          ← 검 피크 571 = 총 클립, MeleeTrace 없음
Air_02 65f  WeaponTrail 10~36 , MeleeTrace 13~31 , CancelWindow 34~65
Air_03 85f  WeaponTrail 23~43 , MeleeTrace 26~38 , CancelWindow 41~85
Air_04 50f  WeaponTrail 11~24 , MeleeTrace 14~19 , CancelWindow 22~50
```

`ANS_MovementCancel`은 **일부러 안 깔았다** — `KDPlayerController.cpp:97~100`의 캔슬 목록에 `Ability.Mugong.AirCombo`가 없어 태그만 붙고 아무것도 안 끊긴다.

`DA_AirComboTree` = Nodes 4 / Links 3 / Entries 1 (4타 일직선).

## 6. SB 공중 콤보 실측 (설계 근거)

`SkillCommandTable.json` 복원 문서에서 검 스탠스 공중 커맨드를 뽑았다.

```
□ Air_LightAttack1_1          입력창 없음(진입)
└─ □ Air_LightAttack2_1       0.00~0.50s
   └─ □ Air_LightAttack3_1    0.00~0.50s
      ├─ □ Air_LightAttack4_1        홀드→릴리즈 0.50s
      └─ □ Air_RollingAttack1_1_Ex   홀드 0.80s [스킬트리]
△ Air_DropAttack1_1 / 2_1 / GuardBreakAttack1_1 (+_Ex)   ← 콤보가 아닌 단발
```

**약공 4타 일직선 + 홀드 분기 하나.** `Tachy`/`Fusion` 스탠스는 3타뿐.

| | 지상 | 공중 |
|---|---|---|
| 입력창 | 0.7 ~ 2.0초 (깊이에 따라 우상향) | **0.5초 고정** |
| 분기 | 약↔강 갈아타기 | **없음** |
| 그래프 | 회피·패링이 합류 | **지상과 노드를 하나도 공유 안 함** |

> `bJumping=True`는 16개 행, 전부 `*_Air_*` — 공중 파생은 **완전히 별도 네임스페이스**로 분리돼 있고 지상 콤보 풀과 연결되지 않는다.

우리 `EComboContext::Air` + 별도 `DA_AirComboTree` 구조가 이것과 일치한다.

**단 SB 입력창은 우리 CancelWindow와 같은 물건이 아니다.** SB는 전부 `0.00~X`로 **노드 시작부터 재는 접수 창**이고, 우리 CancelWindow는 **몽타주 중간부터 여는 실행 지점**이다. 우리 쪽 접수 창에 해당하는 값은 `InputBufferComponent.BufferTimeWindow`(0.2초).

```
SB    ├──────────────┤·······················   앞에 관대 / 뒤에 엄격
      0             0.5s
우리  ·············├─────────────────────────┤   앞에 엄격(12f) / 뒤에 관대
                  f51                     f100
```

## 7. GA 계층 리팩토링

몽타주를 어디서 받느냐가 콤보/단발의 유일한 차이인데 클래스 이름에도 디테일 패널에도 안 드러나 헷갈렸다.

```
UGA_MeleeTraceBase
└─ UGA_PlayerMeleeAttackBase (Abstract)      ← 신설. 히트스탑 / 타격감 큐 / 락온 자동조준
   ├─ UGA_PlayerAttackBase   (Abstract)      = 콤보. 트리가 AttackMontage를 덮어씀
   │                                            HideCategories="Action|Montage" (안 쓰는 칸 숨김)
   └─ UGA_PlayerOneShotAttack                = 단발. 에디터 AttackMontage 직접 사용
```

두 계열에 복붙돼 있던 60줄을 부모로 올림. 220줄 → 142줄, **동작 변경 0.**
공중(`GA_PlayerAirAttackBase`)의 `DefaultAirMontages` 폴백도 제거해 지상과 통일 — 트리 실패가 폴백에 가려지지 않게.

## 검증

| 항목 | 결과 |
|---|---|
| 신규 몽타주 22개 블렌드/그룹/섹션 | 기준(콤보)과 일치, 이상 0건 |
| 소스 RM 플래그 | 계획표와 100% 일치 |
| 회피 트리밍 | 100f → 62f/47f 정확 |
| 노티 창 검증 | 트레이스가 전부 트레일 안 · CancelWindow가 타격 뒤에 열림 |
| `DA_AirComboTree` | 끊긴 참조 0 · GA `ComboInputTag`와 태그 일치 |
| 길동 참조 | GAS 전체 0건 |
| 빌드 | `GA_AirLightAttack` CDO에서 `default_air_montages` 소멸 확인 |

**PIE는 아직 안 돌렸다.** 이번 세션에 만진 전부가 미검증.

## 커밋

**소스**: `781b919` (GA 계층 리팩토링) — 공중 폴백 제거분 3파일은 미커밋
**Content**: `0354a1b` 트레일22 → `2755d89` 단발 → `b0d273c` 회피 → `e9e0819` PlayRate → `7e7d9e5` 배선 → `5eb5f76` 공중노티 → `1988190` AirComboTree

## 남은 것

1. 소스 커밋 + 푸시 (`origin/master` 대비 ahead 2)
2. **PIE 검증** — 공중 4타 / 회피 4방향 / 가드 / 반격 워프 / 피격
3. 코드 결함 4건 → 메모리 `project_pending_combat_code_fixes`
4. 입력 버퍼 0.2 → 0.5 (지상 15개 중 8개가 입력 증발 실측)
5. 트레일 NS 27개 / 사운드 노티 / 데미지 GE 26개 전부 미지정
6. 공중 콤보 재설계 — C안은 땜빵. `Air_01`이 총 클립이라 1타에 판정이 없다
