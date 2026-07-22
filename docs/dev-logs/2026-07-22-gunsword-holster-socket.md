# 2026-07-22 Gun&Sword 홀스터 소켓 셋업 + DA 교차오염 정정

SB_Style 마이그레이션 무기 시스템(검+총 발검/납검)의 선결 작업. 총 칼집(홀스터) 소켓을 검과 동일 방식으로 배선하고, 그 과정에서 드러난 DataAsset 교차오염을 정상화. **소켓 장착 준비까지** 완료 — 발검/납검 동작·PIE 검증은 다음 단계.

## 왜

검 칼집 소켓(`Sworld_Holder_L_Socket`)은 지난 세션에 완료됐으나, 총 칼집 소켓은 미완이었다. 인게임에서 "손 소켓에 유령 무기" + "칼집에 엉뚱한 메시" 증상이 있었고, 원인 가설이 여럿(스킨 박힘 / 소켓 붕괴 / DA 메시 오배선) 열려 있어 확정이 필요했다.

## 한 일

### 1. 손 소켓 유령무기 가설 검증 → 기각

- SKM_Manny_GunSword 머티리얼 슬롯 = **정확히 2개**(`M_torso`=MI_Manny_02, `M_HeadLegs`=MI_Manny_01). 무기 관련 슬롯 없음 → **무기가 메시에 스킨으로 박힌 게 아님.** Hide Material Section 불필요.

### 2. 총 홀스터 소켓 생성 (검과 동일 방식)

- `Gun_Holder_R` 본은 **thigh_r 직속 자식**이지만 **SKM 메시엔 없는 본**이라, 그 본에 소켓을 달면 런타임에 원점(바닥)으로 붕괴(→ 트러블슈팅 참조). 실제로 옛 `Gun_Holder_R_Socket`(붕괴 본, identity)에 총 DA를 잠깐 물려 PIE→바닥 추락으로 실증.
- **해결 = 메시에 있는 진짜 본(`thigh_r`)에 새 소켓 `Gun_Holster_R_Socket` 생성 + `Gun_Holder_R` 본의 로컬 트랜스폼을 baked.** (검이 pelsis에 소켓 달고 Sword_Holder_L 값을 baked한 것과 동일 패턴.)
- 회전은 MCP로 못 읽어(아래) 에디터 프리뷰에서 눈맞춤 후 소켓 Details 숫자 확정.

### 3. DA 교차오염 정상화 (진짜 원인)

- `DA_Weapon_Gun`↔`DA_Weapon_Sword`가 **메시·손·칼집 소켓을 서로 바꿔** 물고 있었음(Gun 카드에 검 메시+검 소켓 3종 등). 컴포넌트 배정(WeaponComp=Sword / GunWeaponComp=Gun)은 정상 → DA 5필드 un-swap으로 해결.
- 이게 "유령무기 + 칼집 총메시" 증상의 진짜 원인. (구 미해결 #1·#2 대체.)

## 최종값

| | 검 | 총 |
|---|---|---|
| WeaponStaticMesh | Sword | Gun |
| HandSocketName | Sword_Weapon_R | Gun_Weapon_L |
| SheathSocketName | Sworld_Holder_L_Socket | **Gun_Holster_R_Socket** |

**`Gun_Holster_R_Socket`** (SK_Mannequin_GunSword):
- 부모 본 `thigh_r` / 위치 [1.947507, -11.872075, 8.257447]
- 회전 [Pitch 76.1735, Yaw -89.8483, Roll 81.3434] / 스케일 [1,1,1] (검과 달리 미러 없음)

> 옛 `Gun_Holder_R_Socket`(붕괴 본)은 에디터에서 삭제(잔재 정리). 코드 `AttachWeaponToSocket`는 이미 `SnapToTargetIncludingScale`(지난 세션) — 소켓 미러 스케일이 런타임에도 반영됨.

## 이슈 / 함정 (→ 볼트 트러블슈팅에도 박제)

1. **스켈레톤 전용 본 소켓 붕괴** — Holder 본이 스켈레톤엔 있으나 메시엔 없어 소켓이 원점 추락. 진짜 본+오프셋으로 우회.
2. **MCP 본 회전 못 읽음** — `list_bones`=location만, `BoneTree`=리타겟모드만, `ReferenceSkeleton`=조회불가. 회전은 에디터 눈맞춤 후 소켓 Details 숫자 읽기가 유일.
3. **DA 교차오염** — 무기 카드 2개가 메시·소켓을 서로 바꿔 물음(팩 데모 마네퀸 기반 복제 잔재 추정).
4. **손 무기 스킨 박힘 가설 기각** — SKM 슬롯 2개 확정으로 배제.

## 검증

- 소켓 위치·회전 = 에디터 프리뷰 눈맞춤 완료. `thigh_r`는 메시에 있는 본이라 런타임도 프리뷰대로 나와야 정상.
- **미완**: PIE 발검/납검 동작(비전투 홀스터 방향 / 전투 발검 손 / 납검 복귀). 발검 몽타주 배선 후 일괄 검증 예정.

## 다음

1. **발검/납검 몽타주** — 팩 `Walk/Run_Equip_to_Combat` 몽타주화(AM_) + `AN_WeaponAttach` 노티(TargetWeaponTag로 검/총 부착) → 전투 태그 On/Off 자동 발검 → PIE 검증
2. Armed 로코(PSD_SB_Loco_Combat + ABP InCombat 분기)
3. 전투 콤보(핵심 경로): 팩 Combo_Attack 몽타주화 → 노티 → GA_LightCombo repoint → PIE
