# 스킬 애니 실측 + 노티 배치 (2026-09-01)

캐릭터 스킬 4개의 몽타주를 만들고, 타격·총발사 시점을 실측해 `MeleeTrace`/`Shot` 노티를 배치했다.
부수적으로 **총 발사 판별 방법을 다시 세웠다** — 기존 방법이 틀렸다는 걸 확인했기 때문.

---

## 1. 몽타주 6개 + 리타겟

| 몽타주 | 소스 시퀀스 | 길이 / fps | 루트 이동 |
| --- | --- | --- | --- |
| `AM_SB_Skill_01` | `AS_Skill_03_Seq` | 3.583s / 60 | 300cm |
| `AM_SB_Skill_02` | `AS_Skill_04_Seq` | 2.583s / 60 | 507cm |
| `AM_SB_Skill_03_Start` | `AS_Speed_Attack_Start_Seq` | 0.167s / 60 | 100cm |
| `AM_SB_Skill_03_Loop` | `AS_Speed_Attack_Loop_Seq` | 0.833s / 60 | 500cm |
| `AM_SB_Skill_03_End` | `AS_Speed_Attack_End_Seq` | 1.250s / 60 | 409cm |
| `AM_SB_Skill_04` | `AS_SB_Skill_04_Seq` | 3.467s / **30** | 403cm |

`AS_SB_Skill_04_Seq` = GhostSamurai `SPAttack04` 를 `RTG_GunAndSword_to_GhostSamurai` 로 리타겟한 결과.
UE5.6 리타게터는 Op Stack 기반이라 **`디폴트 오퍼레이션 추가`로 6개를 넣어야** 변환이 돈다
(`Pelvis Motion → FK Chains → Run IK Rig → IK Chains → Root Motion → Curve Remap`).

### 루트모션이 꺼져 있었다

소스 시퀀스 5개가 `Enable Root Motion = False` 인데 루트는 100~507cm 움직이는 상태였다.
이대로면 **메시만 앞으로 미끄러지고 캡슐은 제자리**에 남아 판정·카메라·워프가 전부 어긋난다.
기존 공격 몽타주 31개는 전부 True 였다(이동 0인 `Aim_the_Target` 3개만 False). 5개 모두 켰다.

---

## 2. 총 발사 판별 — 기존 방법이 틀렸다

### 총열 축 확정

`SK_Mannequin_GunSword` 소켓 실측:

```
검 = 오른손  Sword_Weapon_RSocket
총 = 왼손    Gun_Weapon_LSocket   scale (-1,-1,-1)
```

`GA_ShotBlast` 는 `ShotDir = MuzzleXf.GetUnitAxis(EAxis::X)` 로 총열을 잡는다
(`KDGameplayAbility_ShotBlast.cpp:74`). 그런데 소켓 스케일이 -1이라 **실제 총열은 `-X` 축**이다.

캘리브레이션은 `AN_Shoot` 이 `t=0` 에 박혀 있는 `AS_Aim_the_Target_Shoot_Seq` 로 했다.
f0 에서 `-X` 가 정면 24.2도 — 6개 축 중 최소. 총열 = `-X` 확정.

### 기존 "총 스파이크" 는 발사가 아니라 반동이었다

검증된 발사 프레임 주변을 프레임 단위로 보면:

```
Combo_03_03  f24~31  총열각 23.3도 완전 고정(각속도 0) · 총손속도 5
             f32     각속도 3716 · 총손속도 3148     <- 폭발
Combo_05_04  f28~34  총열각 1.5~8도 · 총손속도 467→66 감속
             f36     각속도 3667 · 총손속도 3935     <- 폭발
```

**발사 = 조준 정지 구간의 마지막 프레임.** 그 다음 프레임의 스파이크는 발사 후 팔이 튕기는 반동이다.
이전 실측 자료가 "총 발사 프레임"으로 기록한 f32·f36 은 전부 반동 프레임이었다.

### 그래도 운동학으로는 판정이 안 된다

`Sk03_Loop` 에 이 시그니처를 적용해 6개를 뽑았는데, 실제 정답과 **1개만 일치**했다.
난사에서는 조준 유지 구간이 2프레임뿐이라 검 스윙에 딸려가는 팔 움직임과 구분이 안 된다.

**대신 리듬으로 접근하니 맞았다.**

```
1  자기상관으로 반복 주기 검출        검속도 -> 0.167s (상관 0.48)
2  주기로 등분, 총열각<45도인 발 수가 최대인 위상 선택
3  결과  0.000 / 0.167 / 0.333 / 0.500 / 0.667   (정면 5/5)
   실제  0.006 / 0.167 / 0.333 / 0.499 / 0.667   <- 눈으로 보고 배치한 값
```

오차 최대 6ms. 총 발사는 **검출이 아니라 제안** 문제다.

---

## 3. 배치 결과

```
AM_SB_Skill_01       MeleeTrace  1.017(d.167)  1.250(d.100)  1.400(d.250)
                     Shot        1.833 · 2.017 · 2.200        0.183s 등간격 3연사
AM_SB_Skill_02       MeleeTrace  0.100(d.250)  0.400(d.150)  0.783(d.133)
AM_SB_Skill_03_Start (없음)
AM_SB_Skill_03_Loop  MeleeTrace  0.117(d.067) 0.250(d.083) 0.433(d.083) 0.633(d.050) 0.750(d.083)
                     Shot        0.006 · 0.167 · 0.333 · 0.499 · 0.667   0.166s 등간격 총검난사
AM_SB_Skill_03_End   MeleeTrace  0.167(d.083)   Shot  0.250
AM_SB_Skill_04       MeleeTrace  1.167(d.167)
```

- **`Skill_03_Start` 는 판정 없음** — 0.167초에 10프레임뿐이라 속도 피크의 50% 구간이 애니 전체를 덮는다. 돌진 개시 전용
- **`Skill_02` 마지막은 땅 찍기**(f85 = 1.417 착지 임팩트). 무기 트레이스로 표현이 안 되므로 노티를 비웠다. 번지는 AoE 어빌리티가 붙을 자리
- **`Skill_03_Loop` Shot 은 루프 경계가 맞물린다** — 마지막 0.667 에서 다음 사이클 첫 발(0.839)까지 0.172s 로 리듬이 안 끊긴다

### PIE 에서 확인할 것

`Sk03_Loop` 의 `Shot t=0.006` 은 60fps 기준 **첫 프레임 안쪽**이다. 루프가 되감길 때 이 위치의
노티는 건너뛰거나 두 번 터질 수 있다. **2바퀴째 첫 발이 빠지거나 두 발 나가는지** 확인하고,
문제가 있으면 `0.017`(f1)로 한 프레임 민다.

---

## 4. 스캔 스크립트 개선 — 실패 4건에서

첫 배치안과 최종본을 대조해 실패 원인을 정리했다.

| 실패 | 원인 | 해법 |
| --- | --- | --- |
| 헛검출 2건 | 속도 피크 = 타격이라 단정 | 후보만 출력, 판정은 사람 |
| 진짜 타격 오탈락 | 검 전방좌표를 필터로 사용 | **회전 베기는 몸 뒤에서 최고속.** 전방·높이는 참고 숫자로만 |
| 난사 통째로 미검출 | 프레임 단위만 봄 | 자기상관 주기 검출 |
| 총 발사 6개중 1개 적중 | 운동학으로 발사 판정 | 검출 포기, 리듬 제안으로 전환 |

추가로 **내려찍기는 `min(왼발Z, 오른발Z)` 로 못 잡는다** — 한 발만 들면 반대발이 지면에 붙어 있어
최소값이 안 변한다. 발별로 추적하니 `Skill_02` 땅 찍기가 `착지[오른발] f84(1.400) · f85(1.417)` 로 잡혔다.

성능도 고쳤다. 본마다 체인을 매 프레임 재계산하던 것을 **체인 1회 캐시 +
`get_bone_poses_for_frame` 로 프레임당 API 1회**로 줄였다. 시간은 `get_time_at_frame` 사용
(`f*dt` 는 프레임 번호가 1씩 밀렸다).

스크립트 = `D:\tmp\anim_pose_scan.py`.
MCP `execute_python` 의 `file` 인자는 보안 차단이라 내용을 `code` 로 붙여넣어 실행한다.

---

## 5. 함정 — `UKDGameplayEffect_StaminaCost` 는 그대로 못 쓴다

스킬 자원을 ST 로 정하면서 확인한 것.

헤더 주석(`KDGameplayEffect_StaminaCost.h:14`)은 이렇게 적혀 있다:

> GA의 `CostGameplayEffectClass`로 등록 시 ASC가 "Stamina < 소모량" 자동 reject

**엔진 소스가 반대다.**

```cpp
// GameplayAbility.cpp:1094-1100  CheckCost
UGameplayEffect* CostGE = GetCostGameplayEffect();          // CDO 그대로
AbilitySystemComponent->CanApplyAttributeModifiers(CostGE, Level, Context)

// GameplayEffect.cpp:5181-5203  CanApplyAttributeModifiers
FGameplayEffectSpec Spec(GameplayEffect, EffectContext, Level);   // 새 Spec 생성
Spec.CalculateModifierMagnitudes();                               // SetByCaller 주입 지점 없음
if (CurrentValue + CostValue < 0.f) return false;
```

이 GE 는 `FSetByCallerFloat` 이라 값을 넣을 자리가 없어 **매그니튜드가 0으로 계산**된다.

```
CheckCost   0 + 0 < 0 = false  ->  항상 통과 (ST 0이어도 발동)
ApplyCost   ST 0 소모 + 매번 SetByCaller not found 에러 로그
```

비교로 `UKDGameplayEffect_AmmoCost` 는 `FScalableFloat(-1.0f)` 고정값이라 정상 작동한다
(`GA_Shoot` 이 실사용 중). 스태미나 폐기 이후 이 GE 를 아무도 안 써서 드러나지 않았다.

**해법** = 스킬 GA 에서 `CheckCost`/`ApplyCost` 를 오버라이드하고
`MakeOutgoingGameplayEffectSpec` → `SetSetByCallerMagnitude(SetByCaller.Stamina, -StaminaCost)` 로 직접 주입.

---

## 6. 다음 — GA 설계 (승인 대기)

```
KDGameplayAbility_Skill.h/.cpp      : UKDGameplayAbility_PlayerMelee    스킬 1·2·4
KDGameplayAbility_SkillHold.h/.cpp  : UKDGameplayAbility_Skill          스킬 3
```

`UKDGameplayAbility_PlayerMelee` 가 몽타주·데미지GE·배수·소켓·트레이스·히트스톱·슬로모·자동조준·
접근워프를 전부 `EditDefaultsOnly` 로 노출하고 있어, **스킬별 값은 BP 에셋 4개가 그대로 데이터**가 된다.
DataAsset 을 끼우면 단계만 늘어난다(`CLAUDE.md §3` 승격 기준 미달).

총 발사는 스킬 GA 가 몰라도 된다 — `Shot` 노티가 `Event.Montage.ShotBlast` 를 쏘고
`GA_ShotBlast` 가 독립 발동한다. 콤보에서 이미 쓰는 경로다.

### 스킬 3 = 몽타주 통합 권장

현재 `Start`/`Loop`/`End` 가 몽타주 3개인데, 600cm/s 로 이동하는 중에 몽타주 전환 블렌드가 끼면 튄다.
단일 몽타주 5섹션(`Start`/`Loop1`/`Loop2`/`Loop3`/`End`) + `Loop3→Loop1` 링크,
키 릴리즈 시 `Montage_SetNextSection(루프섹션, "End")` 가 정석이다.

```
Loop1  2타 (0.150 · 0.317)     Loop2  2타 (0.467 · 0.650)     Loop3  1타 (0.783)
```
각 섹션이 자기 마지막 타격 직후에 끝나므로 키를 떼도 타격이 잘리지 않는다.

⚠️ `OnActivated` 에서 `ClearSafetyTimer()` 필수. 부모가 `몽타주길이 × 1.5` 로 안전 타이머를 거는데
통합 몽타주 2.25s × 1.5 = 3.375s 라 3바퀴(3.92s) 돌면 어빌리티가 강제 종료된다.

몽타주 3개를 유지하려면 부모 `UKDGameplayAbility_MeleeTrace` 의 private 멤버
(`OnTraceBeginEvent`·`AlreadyHitActors` 등)를 열어야 한다 — 콤보 GA 20개가 상속 중인 공용 부모다.

---

## 검증

- [x] 소스 시퀀스 5개 루트모션 ON — 기존 공격 몽타주 31개와 일치
- [x] 몽타주 6개 슬롯 `DefaultSlot` · 섹션 `Default` · 블렌드 0.25 — 기존과 동일
- [x] 노티 19개 배치 후 강제 저장 확인 (mtime + git diff)
- [x] 총열 축 캘리브레이션 — `AN_Shoot` 애니로 검증
- [x] 리듬 제안이 수동 배치를 오차 6ms 로 재현
- [ ] PIE — 스킬 발동·판정·총 발사 (GA 미구현이라 대기)
- [ ] `Sk03_Loop` `Shot t=0.006` 루프 경계 동작

## 커밋

```
4785073  [Anim] 스킬 애니 루트모션 + 스킬 몽타주 6개 + SPAttack04 리타겟
3b509b1  [Anim] 스킬 몽타주 4개 노티 배치 - MeleeTrace 10 + Shot 9
```

## ⚠️ MCP 함정 — `save_asset` 이 조용히 아무것도 안 한다

`EditorAssetLibrary.save_asset(path)` 의 기본값은 `only_if_is_dirty=True` 인데,
`AnimationLibrary` 로 노티를 추가해도 **패키지가 dirty 로 안 찍힌다.**
`save=True` 를 반환하지만 디스크에는 안 써진다.

이번에 `AM_SB_Skill_01`(노티 6개)과 `AM_SB_Skill_03_End`(2개)가 이 상태로 남아 있었고,
mtime 이 커밋 시각보다 이전인 것으로 발견했다. 에디터를 닫았으면 날아갔다.

**파이썬으로 에셋을 고쳤으면 `only_if_is_dirty=False` 로 강제 저장하고,
mtime 이나 `git status` 로 실제 기록을 확인할 것.**
