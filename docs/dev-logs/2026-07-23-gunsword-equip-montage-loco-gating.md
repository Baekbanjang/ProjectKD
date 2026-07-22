# 2026-07-23 Gun&Sword 발검/납검 몽타주(속도대별 상체 슬롯) + 로코모션 Stop 게이팅

검+총 동시 발검/납검을 속도대별 상체 몽타주로 구현하고, 그 과정에서 드러난 MM 로코모션의 Stop 오발동을 입력 게이팅으로 잡았다. 홀스터 소켓([2026-07-22 dev-log])에 이은 무기 시스템 수직슬라이스 완성 단계.

## 발검/납검 시스템

**흐름**: `State.Combat.InCombat` 토글 → `WeaponComponent::OnInCombatTagChanged`(검 컴포넌트만 broadcast) → `Event.Combat.WeaponToggle`(OptionalObject=선택된 몽타주) → `GA_WeaponToggle`(ActivateAbilityFromEvent → PlayMontageAndWait) → 몽타주의 `AN_WeaponAttach` 노티가 컴포넌트 전체 순회하며 검·총 각자 손 소켓에 부착.

- **컴포넌트 2인스턴스**: 검=`WeaponComp`(bBroadcastsToggleEvent=true), 총=`GunWeaponComp`(false). **몽타주 트리거는 검 1개만**(한 몸에 전신/상체 몽타주 2개 겹치면 충돌), 부착은 노티가 둘 다 처리. 구분은 이름 아닌 `WeaponComponentTag`(Sword/Gun).
- **AN_WeaponAttach**: `TargetWeaponTag`로 특정 무기 지정(None=전체 동시). Draw=ToHand(검→총 순서), Sheath=ToSheath(총→검 순서). 노티 2개 태그별 배치.
- 몽타주는 `DA_Weapon_Sword`에만 배선(총 DA는 null — 안 쏨).

## 속도대별 몽타주 (신규 코드)

`WeaponDataAsset.h`:
- `FEquipMontageSet { Idle, Walk, Run }` 구조체 신설, `DrawMontages`/`SheathMontages` 2필드로 기존 단일 필드 대체.

`WeaponComponent`:
- `SelectEquipMontage(const FEquipMontageSet&)` — `GetOwner()->GetVelocity().Size2D()`(Pawn 캐스팅 없음, §1-3 준수)로 밴드 판정 → Idle/Walk/Run 몽타주 1개 반환.
- 임계값 `WalkSpeedThreshold`(10) / `RunSpeedThreshold`(300) EditAnywhere 노출. 선택은 토글 순간 1회.

## ★상체 슬롯 결정 (핵심 아키텍처)

발검을 전신(DefaultSlot)으로 하니 이동이 막히거나(Idle) 감속(Walk/Run)됐다. → **6개 몽타주 전부 `UpperBody` 슬롯 + 루트모션 OFF**로 전환.
- ABP_SB에 상체 블렌드 추가: `Save cached pose`(로코 결과) → `Layered blend per bone`(Base=캐시) + `Slot 'UpperBody'`(Source=캐시) → Blend Poses[0], 블렌드 본=`spine_01`, Mesh Space Rotation Blend on.
- 하체·이동=MM 로코모션, 상체=발검 몽타주. 이동 안 막히고 발 슬라이딩 없음.
- **루트모션 함정**: 상체 슬롯이어도 몽타주에 RM 켜져 있으면 이동 지배 → 감속. 이 프로젝트는 로코모션이 RM 구동이라 발검(제자리 lock RM)이 속도를 끌어내림. 6개 다 RM OFF로 해결. (→ 볼트 트러블슈팅 박제)

## 로코모션 Stop 게이팅

C 걷기(저속)에서 MM이 Stop을 골라 **실제로 멈추는** 버그(루트모션 MM이라 Stop 선택=물리 정지). 원인=`PSD_SB_Stops`가 이동 중에도 항상 검색돼 저속서 Walk Loop보다 싸게 이김. 속도(WalkSpeed 250→160) 조정으로 안 됐음 → 구조 문제.

**해결 = ABP에서 `bHasMovementInput`(가속도 기준, 이미 AnimInstance에 존재)으로 DB 게이팅**:
- True(입력 있음) → `[Loco]`만 (Loop+Start)
- False(입력 없음) → `[Stops, Idles]` (Stop=발 심는 감속 / Idle=정지 포즈)
- 대칭 원칙: 이동 중엔 Stops 빼고, 정지 중엔 Loco 빼기(안 그러면 드리프트). 게이트는 **속도 아닌 입력** 기준이어야 달리다 뗄 때 발 심음.

## 소켓 (참고)

손 무기는 `Sword_Weapon_R`/`Gun_Weapon_L` **본에 직결**(DA HandSocketName). 팩이 무기 쥐라고 만든 전용 본이라 GripPoint 불필요. 어긋나면 소켓(`_Socket` 접미사 버전)으로 DA 바꿔 튜닝. `AttachSocketName`(WeaponComponent)은 DA 없는 적용 폴백일 뿐.

## 검증

- PIE: 정지/걷기/뛰기 발검·납검 각 속도대 몽타주 재생, 상체만 동작 이동 유지, 검→총·총→검 순서 정상.
- 로코: 걷기 중 Stop 안 튐, 달리다 떼면 발 심고 정지, Idle 정착.

## 다음

1. 커밋 (이 세션 코드+문서)
2. Armed 로코 (전투 스탠스 이동, PSD_SB_Loco_Combat + ABP InCombat 분기)
3. 전투 콤보 핵심경로 (팩 02_Attack Combo → 몽타주화 → 노티(MeleeTrace+CancelWindow) → GA_LightCombo repoint → PIE)
