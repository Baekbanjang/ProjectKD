# 2026-08-30 — 퍼펙트 패링 반격(우) 노티 배치

## 진행

08-28 패링 재설계의 후속. **퍼펙트 패링 후 강공 반격의 오른쪽 갈래(`GA_CounterSlash`) 몽타주에 노티 0개 → 판정·사격·사운드 전부 배선. 타이밍 실측 + 표준 추적 설정**. 

계획 전문 = `docs/handoffs/CURRENT.md` §2026-08-30 반격(우)

## 1. 배선은 이미 다 돼 있었다 — 노티만 없었다

### C++ 입력 분기

```cpp
// KDPlayerAbilityInputComponent.cpp:219~221
if (ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_CounterReady))
{
    if (ActivateByTag(ASC, GameplayTags::Ability_Player_CounterSlash)) return;
}
```

강공(`TryHeavyAttack()`) 분기에서 `State.Combat.CounterReady` 태그를 읽고
`Ability_Player_CounterSlash` 를 발동. 좌/우 분기는 GA 레벨에서만 일어난다.

### 상속 사슬 (콤보트리 미관여)

```
UKDGameplayAbility_CounterSlash (부모 지우고 Ability_Player_CounterSlash 태그만)
  └ UKDGameplayAbility_CounterThrust (Motion Warping 로직. 우리는 패링용이라 대시 없음)
    └ UKDGameplayAbility_PlayerMelee (판정 추적·사격 로직)
```

★ **콤보 노드를 거치지 않는다.** `State.Combat.CounterReady` 태그 하나로 발동하고 콤보트리의 분기·진입 제약이 없다. 08-28 설계에 따라 패링/회피 반격 모두 동일.

## 2. 스윙 구간 포즈 곡선 실측

`AS_Parry_Counter_Attack_R_Seq` 원본 시퀀스의 hand_r 본(컴포넌트 공간) 궤적 계산.

```
f  2~16    1600~2135 cm/s    1타 (칼 회전)
f 16~24      60~ 770         멈춤 (f20~22 = 기저 60)
f 25~34     480~1030         2타 (총 쏘는 자세 전환)
f 35~        150 이하         회수
```

### ⚠️ 시행착오 2건 (재사용 가치)

```
get_bone_pose_for_time(MCP)은 로컬 공간 — hand_r 위치가 거의 변하지 않음(peak 0)
  -> find_bone_path_to_root 로 사슬 구성 후 compose_transforms 로 누적 = 컴포넌트 공간
  
오일러 각 차이로 각속도 계산 시 랩어라운드 가짜 피크 발생 (f8: 2503 deg/s)
  -> 위치 궤적만 사용이 안전
```

**좌우 대칭 아님:** `AS_Parry_Counter_Attack_L` 피크는 f22~24 로 R보다 2초 뒤.
안무가 다르면 L 노티를 그대로 R에 복사해서는 타이밍이 어긋난다.

## 3. 최종 배치 — 승환이 애니를 눈으로 보고 프레임 결정

1타 = 칼 회전 / 2타 = 총 공격 구분.

```
MeleeTrace      f 4~13         오버라이드 전부 비움 = GA 기본값 상속
WeaponTrail     f 4~18         NS_SlashTrail_Basic_Loop / Sword / Sword_Bottom
PlayerCue       f 6             L 에서 유추한 배치
Shot            f 14.9 · 22    발사 두 번
Sound_Shot      f 14.9 · 22    ShotGun_Shot_Sound
Muzzle          f 11~17 · 19~25  NS_SB_Hit_Fire_Once / Gun / Muzzle
CancelWindow    f 28~96
MovementCancel  f 28~96
WeaponAttach    —              빈 트랙
Sound_Swing     —              빈 트랙
```

📌 `f14 -> f14.9` 는 승환이 총성을 뒤로 밀어 판정과 사운드를 정렬한 것.
`Sound_Shot` 도 같이 옮겨 쌍을 유지했다.

## 4. ★ 표준 트랙 구성 발견 (재사용 패턴)

기존 몽타주 4개(L/R 반격, 콤보 05_03 등) 비교로 추출한 표준 트랙 이름:

```
핵심 판정       MeleeTrace · CancelWindow · MovementCancel
움직임 표시     WeaponAttach · WeaponTrail · Warp
총격 세트       Shot · Sound_Shot · Muzzle
이펙트·사운드   PlayerCue · Sound_Swing
```

### ★★ 총격 1발 = 3노티 세트

```
Shot            발사 신호 (GA_Shoot 대기 또는 KDAnimNotify_Shoot 클래스)
Sound_Shot      枪 사운드 (KDAnimNotify_PlaySound)
Muzzle          총구 섬광 (KDAnimNotifyState_WeaponTrail, 재사용)
```

**콤보 05_03 실측:** 5발 = 3개 세트 × 5 (정렬 상태). 
따라서 새 총격 몽타주는 이 3종을 **반드시 함께 배치**.

### ★★ Muzzle 트랙은 WeaponTrail 클래스의 재사용

```
WeaponTrail(칼)  NS_SlashTrail_Basic_Loop / Sword        / Sword_Bottom
Muzzle(총구)     NS_SB_Hit_Fire_Once      / Gun          / Muzzle
```

클래스 기본값이 `Sword` / `Sword_Bottom` → **새로 만들면 칼 손잡이에서 총구 섬광이 난다.**
기존 몽타주에서 값을 복사 필수.

타이밍도 표준이 있다 — **`Muzzle` 은 `Shot` 앞뒤 0.05초씩 감싼다** (R 01_01 실측: `t=0.60±0.05 / dur=0.10`).

## 5. ★ MeleeTrace 는 값을 비우는 게 정석

`KDAnimNotifyState_MeleeTrace.h` 주석:
```
아래는 "이 창만 예외" 설정. 비워두면 GA 기본값이 그대로 쓰인다
StartSocketOverride = NAME_None
EndSocketOverride = NAME_None
CapsuleRadiusOverride = 0.f      // 0 → GA 값 상속. 판정이 없어지는 게 아님
```

소켓·반경은 `GA_CounterSlash` (부모 `GA_PlayerMelee`) 가 들고 있으므로
**노티는 타격 시간대(f4~13)만 정의하고 값은 비운다.**

## 6. ⚠️ MCP 에러 후속조치 함정 (전수 조사 필수)

```
execute_python 이 에러를 뱉어도 그 전까지의 부수효과는 남는다
  -> 첫 실행이 실패했다고 가정 후 재실행 → 노티 중복 생성(16개 됨)
  -> remove_all_animation_notify_tracks 로 싹 지우고 한 번에 다시

get_animation_notify_events_for_track 출력은 트랙이 겹쳐 보인다 (인덱스 매핑 추정)
  -> 실제 개수는 get_animation_notify_events 로 전수 확인

AnimMontage.notifies 는 protected → 파이썬 불가
  -> AnimationLibrary.get_animation_notify_events* 사용
  -> 개별 노티 프로퍼티는 get_editor_property 로 추출 (Muzzle 값 복사에 사용)

바이너리 스캔으로 노티 유무 판정 가능
  $ grep -aoE "KDAnimNotify[A-Za-z_]*|AnimNotify_PlaySound" file.uasset
  파일 크기도 신호 — 노티 0개 R(9,522B) vs 노티 5종 03_03(21,180B)
```

**시사:** 에셋 조회 script 실행 중 실패해도 "된다"고 착각하기 쉽다.
매번 결과를 **전수 검증**해야 한다.

## 7. 검증

```
빌드      불필요 (에셋만)
PIE       판정 ✓ / 총격 2발 ✓ / 사운드 ✓ / 총구 섬광 ✓ (승환)
```

## 남은 것

```
Sound_Swing 트랙      칼 회전(f2~16)에 SC_Sword_Swing 추가할지 미정
PlayerCue f6          L 에서 유추. 엉뚱한 이펙트면 트랙만 비우면 됨
Muzzle 타이밍        1발째 f11~17 / 2발째 f19~25
                    Shot(f14.9)보다 0.065초 먼저 시작 (표준 ±0.05 근처, 그대로 유지)

패링/회피 반격 태그   아직 State.Combat.CounterReady 공유 (분리는 나중에)
                    반격 애니 2개뿐이라 지금 나누면 회피 손해만 본다
```

커밋 = 에셋 (로컬만)
