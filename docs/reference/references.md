# 참고 자료 (3-phases.md 보조)

> 3-phases.md = 일정·분담·Phase 본문. 본 파일 = 결정·owner·데이터·자산 파이프라인.

---

# 확정 결정 (변경 시 페어 합의)

## 액션·전투

1. **적 구성** — 잡몹 4종 (단검·활·도끼·무당) + 호위 무사 엘리트 (3패턴: 콤보·Unblockable·Fatal) + 보스 (각시탈) P1·P2. P3 광폭화 = 시간 남으면
2. **도술 3종 + Trace 차등** — 화부술(`1`) Projectile / 풍보(`2`) Mobility(no trace) / 정승(`3`) Cone Overlap. 정승 효과 = **Stagger 게이지 감소**. ⚠ 분신/축지 폐기. 도술은 `ActionBase`만 상속, **도술 베이스 만들지 X**
3. **Stagger 시스템**:
   - 잡몹 = 게이지 UI X → Stagger 0 시 `[F] 처형` 키 프롬프트 → 짧은 처형 모션
   - 보스 = 게이지 HUD 하단 UI → Stagger 0 시 공중QTE T1 자동
4. **콤보** — LMB 4타 + RMB 탭/0.5s 홀드. 6 시퀀스. 봉 특수(LMB+RMB) 폐기. 캔슬 윈도우 0.25s
5. **공중QTE** — R 키, 0.2x 슬로우, 입력 0.8s, 6단계. T1=Stagger 0 / T2=점프 공격
6. **보스 페이즈** — HP 70% (P1→P2). P3 = 시간 남으면 (HP 40% 자동, 광폭화)
7. **환경 인터랙션** — 봉 공격 자동 트리거 (G키 X). 통나무·짚단·대용폭탄
8. **MMB 적 토글** — 락온 단발 탭. 화부 시너지
9. **게임플레이 카메라 = 페어** — 전투 거리·락온·1:1 줌·QTE·풍보 공중정지·보스전

## 캔슬·손맛

10. **캔슬** = 규칙(필규 SB 톤) + GA UPROPERTY(승환). 11×11 매트릭스 폐기
11. **손맛** = Phase 1 페어 통합 / Phase 2 각자 사용
12. **Doul 차오름** — 적중 + 패링 누적. 시간 회복 X. 시작 0%

## 분위기·사운드

13. **환경 톤** — 정오 맑음 / 황토·이끼·먹·솔잎. 핏빛·네온 X
14. **병맛** — 진지 70% + 코미디 30%. 시그니처 모먼트 5개
15. **BGM** — 페이즈 가변 (평시/전투/P1/P2). 보이스 X 자막만
16. **채도** — 슬로우모 -30% / 처형 -50% (P3 추가 시 광폭화 -40%)
17. **슬로우모 4종** — 패링·카운터·처형·동에번쩍 (P3 추가 시 광폭화 +1)

## UI·System

18. **UI = UMG 직빵** (CommonUI X)
19. **UI 범위** — HUD(HP/Stamina/Doul) + 적 HP·Stagger + 인벤(2) + 도술 슬롯(3) + QTE + 보스 HP 3분할 + 튜토리얼 텍스트. 자막/미니맵/길동 초상화/HP 수치 X
20. **System 범위** — 메인메뉴 → 게임 → 일시정지 → 저장 → 종료. 저장 슬롯 1 + 체크포인트 5 (구간1~4 + P2 직전). 해상도/창모드/리매핑/디버그 콘솔 deferred

## 인프라

21. **GAS 인프라 + prototype 6종 마이그·fix = 필규 Day 0** — 승환은 raw 위에 GA 작성 (fix 부담 X). 단 `GA_LightAttack`/`GA_Dodge` = 승환 재설계. 상세 = §prototype 코드 표
22. **프로젝트 골격 = YAGNI** — Day 0 미리 X. UE5 디폴트 GM/PC/GI로 시작. **Player Pawn 신규 = 승환 W1** (`HKDPlayerCharacter` 재설계). owner·시점 = §공유 인프라 표
23. **기획 완료** — 필규(적·보스, 적-컨셉 ✓ / 보스 다듬기) / 승환(도술·콤보·UI·QTE·분위기·전투진행 ✓)
24. **싱글 한정** — Replication 코드 X

---

# 공통 클래스 owner (충돌 방지)

## C++ 클래스

| 클래스 | Owner | 사용 |
|---|---|---|
| `UGdGA_ActionBase` (모든 GA 부모) | **필규** | 양쪽 상속 |
| `UGdGA_WeaponTraceBase` | **필규** | 승환 Light/Heavy + 필규 잡몹 |
| `UAT_WeaponTrace` | **필규** | WeaponTraceBase 내부 |
| `UAnimNotifyState_WeaponTrace` / `CancelWindow` / `AnimNotify_EarlyEndAbility` | **필규** | Montage 박음 |
| `UInputBufferComponent` | **필규** | 승환 Pawn 부착 |
| `AGdBaseCharacter` | **필규** | 양쪽 상속 |
| `UGdCharacterAttributeSet` (HP/MaxHP/Stamina/MaxStamina/AttackPower) | **필규** | 양쪽 부착 |
| `UGdAS_Stagger` (Phase 2) | **필규** | Enemy attribute |
| `UGdGE_Damage_Physical` (SetByCaller) | **필규** | 양쪽 적용 |
| `GdGameplayTags.h` (중앙 선언) | **필규** | 양쪽 추가 (PR 머지) |
| `AGdPlayerCharacter` | **승환** | — |
| `UGdGA_LightAttack` / `HeavyAttack` / `Dodge` / `Parry` | **승환** | — |
| `UGdGA_Doul_Hwabu` / `Pungbo` / `Jeongseung` | **승환** | — |
| `UGdGA_AirCombo` (R) | **승환** | — |
| `UGdGA_FExecute` (잡몹 처형) | **승환** | 필규 Stagger 0 트리거 |
| `UGdGA_Targeting` (MMB 락온) | **승환** | — |
| `AGdEnemyBaseCharacter` / `UGdGA_Enemy_*` / 보스 BT + Phase swap | **필규** | — |

## WBP (UMG)

| WBP | Owner |
|---|---|
| `WBP_HUD` (좌상단 3바) / `WBP_BossHP` / `WBP_BossStagger` / `WBP_EnemyHP` / `WBP_FExecutePrompt` | **필규** |
| `WBP_DoulSlot` / `WBP_InventorySlot` / `WBP_QTE` | **승환** |
| `WBP_MainMenu` / `WBP_PauseMenu` / `WBP_SaveSlot` | **페어** |

## 공유 인프라

| | Owner | 시점 |
|---|---|---|
| `Build.cs` / `DefaultEngine.ini` / `DefaultGameplayTags.ini` / `GdAttributeMacros.h` | **필규** | Day 0 |
| IMC_Default + IA 14 + 패드 매핑 | **승환** | W1 (Player Pawn 입력 받을 때) |
| Collision Channel (WeaponTrace ObjectType / Pawn / Projectile / Environment) | **필규** | W1 (첫 트레이스 검증) |
| NavMesh (Recast Dynamic) | **필규** | W1 (단검 잡몹 BT 작동) |
| `LogGdCombat` / `LogGdAI` / `LogGdGAS` | 누구든 | 첫 `UE_LOG` 박을 때 |
| `AGdGameMode` (ASC GiveAbility) / PC / GI | **승환** | 필요 시점 (Beat grant 도입 W3+) |
| `UGdSaveGame` / `UGdSaveGameSubsystem` | **페어** | W9~10 (체크포인트 도입 시) |
| `DA_EncounterMatrix` | **필규** | W3~5 |
| 레벨 5구역 / 체크포인트 5 / 환경 인터랙션 / BGM 페이즈 / `UPostProcessVolume` / 카메라 | **페어** | W9~10 |
| `ULevelSequence` 시그니처 ×5 | 분담 (#1·#3·#4=승환 / #2=필규 / #5=페어) | W11~12 |

## 충돌 가능 지점

- **`GdGameplayTags.h`** — 둘 다 새 태그 추가. PR 단위 머지 (commit 직전 pull, conflict 시 양쪽 유지)
- **`UGdCharacterAttributeSet`** — Phase 2 Doul attribute 추가 시 필규 commit 후 승환 알림
- **`UGdGA_WeaponTraceBase`** — fix 후 시그니처 변경 시 필규 commit 전 승환 알림
- **`IMC_Default` / IA 14** — 승환 W1 셋업 후 도술/Inv 추가 시 본인이 IA 신규. Collision Channel은 필규 W1 정의 후 변경 시 알림

---

# 자산 파이프라인

## 3D 모델 (페어, Meshy)

| 모델 | 가면 |
|---|---|
| 단검·활·도끼·무당 도적 (잡몹 4) | X |
| 호위 무사 (엘리트) | X (수염·흉터) |
| 도적 두목 (보스) | 붉은 각시탈 (P3 산산조각, 추가 시) |

**워크플로우** (길동으로 검증됨): 원화 → Meshy AI → Blender (리토폴로지/UV/머티리얼) → UE 표준 휴머노이드 본 (Quinn 호환) → IK Rig + Retargeter → Montage 호환 테스트

**진행 전략**:
- Phase 1: 단검 잡몹 placeholder (UE 매네킹)
- Phase 2 W3~5: 잡몹 4종 점진 교체
- Phase 2 W6~8: 호위 + 보스 P1·P2 외형
- Phase 3 W11~12: 디테일 폴리싱 + P3 가면 산산조각 (optional)

## 애니메이션 (마켓플레이스 팩 + Quinn 리타겟)

> ⚠ Meshy = 메시만. 모션은 별도 조달. **마켓플레이스 휴머노이드 팩 구매 + 리타겟**.

**필요 클립 ~36개**:
- 잡몹 4종 × 6모션 (Idle / Walk / Run / BasicAttack / HitReact / Death) = 24
- 호위 엘리트 × 5 (위 + Unblockable + Fatal) = 5
- 보스 × 7 (위 + Phase swap + 봉술 콤보 + 차지 사격) = 7

**구매 시점**:
- Phase 1 시작 전 = 휴머노이드 잡몹팩 1개 (Mixamo 무료 보조)
- Phase 2 W3 = 보스 봉술 + 차지 사격 팩 (또는 Mixamo 검 모션 변형)

**추천 후보**: Marketplace의 *Combat Animation Pack* / *Sword Attacks Animations* / *Polearm Master* 류.

## 사운드 (마켓플레이스 팩)

- **BGM 4트랙** (평시 / 전투 / 보스 P1 / 보스 P2) — Marketplace 국악·동양풍 BGM 팩
- **SFX** — Marketplace SFX 라이브러리 (HitImpact / 발자국 / UI / 도술)
- **국악 톤** — Marketplace 한국 전통 악기 팩 또는 freesound.org 국악 무료 트랙
- **VO 없음** — 자막 only

**구매 시점**: Phase 2 W6 결정 → Phase 3 W11 통합

## VFX (자체 보유 + 마켓플레이스 보조)

- **자체 보유 자산**: 흙먼지·낙엽·산바람·HitImpact 기본
- **마켓플레이스 보조**: 도술 3종 이펙트 / 페이즈 전환 / 처형 컷
- **추천 후보**: *Realistic Starter VFX Pack Vol 2* / *Niagara Effects Sample* / *Mystic Powers*

**구매 시점**: Phase 1 W2 손맛 통합 전 결정

## prototype 코드 (필규 Day 0 마이그·fix)

| 영역 | 클래스 | Day 0 작업 |
|---|---|---|
| **결함 없음 (그대로 OK)** | `ActionBase`, `ANS_WeaponTrace`, `ANS_CancelWindow`, `AN_EarlyEnd`, `InputBufferComponent`, `BaseCharacter` | sed rename 후 복사 |
| **결함 있음 (필규 fix)** | `WeaponTraceBase` (캐싱 / AlreadyHit 리셋 위치) / `AT_WeaponTrace` (Rot1만 / 첫 틱 시딩 / SubSteps / debug 잔재) | fix 5개 완료 후 복사 |
| **재설계 (승환)** | `HKDPlayerCharacter` (SB 톤 위반 / Mugong 하드코딩 / Tick polling) | 승환이 W1에 처음부터 신규 작성 |
| **참고만 (raw)** | `GA_LightAttack`, `GA_Dodge` (콤보 + i-frame 패턴) | 승환이 W1에 보고 새로 작성 |

## 컨트롤러 지원 (IA 패드 매핑만)

- Enhanced Input IA에 게임패드 입력 추가 (LMB↔X, Shift↔A, 1·2·3↔십자키 등)
- **UI 컨트롤러 지원 X** (Steam Deck verified 미목표, 키보드+마우스 또는 패드 IA만)

## 외부 QA = 출시 후

- 본 12주 (W1~W12) = 페어 2명 self-test만
- 외부 QA = Phase 4 (W13+) 스토브 인디지원 / Steam Next Fest 신청 후

---

# 데이터

## 인카운터 매트릭스

| 구역 | 시간 | 적 구성 | 동시 | 학습 강제 |
|---|---|---|---|---|
| 산기슭 | 0~3분 | 단검×1 | 1 | 회피 |
| 도적촌 입구 | 3~5분 | 단검×2~3 | 3 | 회피 복습 + 콤보 |
| 도적촌 내부 | 5~10분 | 단검×3, 활×2 | 5 | 풍보 (활 즉접근) |
| 도적촌 내부 | 10~15분 | 도끼×2, 무당×1, 단검×2 | 5 | 패링 (도끼) + 화부 (무당) |
| 두목 앞마당 | 15~20분 | 호위 엘리트×1 (1:1) | 1 | 동에번쩍 + 3패턴 |
| 두목 P1·P2 | 20~25분 | 보스 + 잡몹×2 (P2 호각) | 3 | 정승 stress + 보스 |

→ `DA_EncounterMatrix` 1개. Spawner Actor 직접 참조.
→ 첫 야영지 (0~3분 직전): 도적 4~5명 통조림 잡담 비전투 컷.

## Beat Sheet — 메커니즘 해금

| 시간 | 해금 |
|---|---|
| 0~5분 | LMB 4타 / RMB / Shift / Q |
| 5분 | 화부술 (`1`) |
| 10분 | 풍보 (`2`) |
| 15분 | 정승 스턴 (`3`) |
| 15~20분 | 동에번쩍 (`R`) |
| 20~25분 | 보스 P1·P2 (P3 = 시간 남으면) |

→ GameMode가 구역 진입 시 `ASC.GiveAbility`. 별도 Subsystem X.

## 보스 페이즈

| 페이즈 | 시간 | 외형 | 패턴 | 전환 |
|---|---|---|---|---|
| P1 | 20:00~22:00 | 각시탈 + 의연한 봉술 + 군용 코트 | 봉술 콤보 + 차지 강공 | HP 70% |
| P2 | 22:00~25:00 | 가면 균열 + 갑주 일부 벗음 | 봉 + 폭약 + 호각 + 차지 사격 | HP 0% (처치 = 결말) |
| **P3 (optional)** | (P2 단축, 23:30~25:00) | **각시탈 산산조각** + 코트 찢음 + 광인 | **광폭화** — 0.4배속 + Stamina·Doul 무제한 + 캔슬 풀오픈 | HP 0% |

→ BT 1개 + Blackboard `CurrentPhase` + 서브트리. `GA_Boss_PhaseTransition`이 BT 외부에서 swap.

## 슬로우모 트리거

| 트리거 | 위치 | 채도 |
|---|---|---|
| 패링 | Q 성공 | -30% / 0.3s |
| 카운터 | 패링 후 강공 | -30% / 0.3s |
| 처형 | 보스 P2 처치 / 잡몹 F | -50% / 1.0s |
| 동에번쩍 | R QTE 진입 | -30% / 시퀀스 동안 |
| 광폭화 (P3 추가 시) | P3 진입 | -40% + 햇빛 강조 |

## 시그니처 병맛 모먼트 5개 (Sequencer)

| # | 시간 | 모먼트 | 담당 |
|---|---|---|---|
| 1 | 0:30 | 모놀로그 "*백운도사 말대로… 발끝부터.*" + 발끝 보는 컷 | 승환 |
| 2 | 5:00 | 다수전 적 한 명 "엇!" 황당 다운, 멈춤 표정 0.5s | 필규 |
| 3 | 10:00 | 화부술 첫 발동 — 부적 슈우~ + 폭발 컷 | 승환 |
| 4 | 15:00 | 정승 첫 발동 — "어흠—" + 적 머리 별 이펙트 | 승환 |
| 5 | 25:00 | 처형 컷 후 길동 어깨 으쓱 + 페이드 | 페어 |

→ `ULevelSequence` ×5 + GameplayCue 트리거.

---

# prototype 참고 카탈로그

- 보존: git tag `m1-prototype-final` + README + 영상 1분 archive
- Day 0에 신 프로젝트로 가져갈 자산 = §prototype 코드 표 (위)
- 아래는 prototype 자체 카탈로그 (가져가지 않는 자산 포함, 학습용)

## 참고 가능 자산 (prototype 원본 클래스명)

| 영역 | 클래스 | 비고 |
|---|---|---|
| GA 베이스 | `HKDGA_ActionBase`, `HKDGA_WeaponTraceBase` | SafetyTimer + EarlyEnd, Montage+Trace+Damage |
| AbilityTask | `AT_WeaponTrace` (`ETraceMode`) | 무기 충돌 — Phase 1 손맛 통합 |
| Player GA | `GA_LightAttack`, `GA_Dodge` | 콤보 + i-frame + JustWindow |
| Enemy | `HKDEnemyCharacter`, `EnemyAttackSchedulerComponent`, `GA_Enemy_BasicAttack` | ASC 외부 주입 |
| 손맛 | `GCN_HitImpact_Light`, `HKDHitFeedbackComponent`, `HKDEnemyAnimInstance` | per-actor PlayRate / BoneShake / alpha bone |
| Notify | `ANS_WeaponTrace`, `ANS_CancelWindow`, `AN_EarlyEnd` | Montage 구간 마킹 |
| AttributeSet | `AS_Base`, `AS_Combat` | M1 검증 완료 |
| Tags | `HKDGameplayTags.h` | 중앙 선언 |
| 선입력 | `InputBufferComponent` | 0.2~0.3s 버퍼 |

## 풀린 패턴 7개

1. **GA 2단 추상화** — ActionBase → WeaponTraceBase → 구체 (~250 LOC dedup)
2. **콤보 = Montage section jump + InputBuffer + ANS_CancelWindow** (트리 DA X)
3. **GameplayCue 위임 HitStop** — GA → GC 단방향, per-actor Montage PlayRate 0
4. **컴포넌트 → AnimBP 단방향** — HitFeedback alpha → AnimBP read
5. **컴포넌트 → ASC 주입 init** — Pawn 캐스팅 금지, `Init(ASC)` 외부 주입
6. **AnimNotify가 GA 라이프사이클 제어** — `AN_EarlyEndAbility` → Event.Ability.EarlyEnd
7. **GE Duration이 single source** — 코드 override X, 디자이너 편집 우선

---

# 보류 / 스코프 축소 옵션

**보류**
- AirCombo 단순화 — Phase 2 진행 후 재산정
- Phase별 +30% buffer 인정 여부

**축소 옵션 (게이트에서 risk 발생 시 발동)**

| 옵션 | 부담 감소 |
|---|---|
| 잡몹 무당 deferred | 필규 Phase 2 -1주 |
| 보스 P3 광폭화 deferred | 필규 Phase 3 -1주 |
| 메뉴 / 저장 슬롯 deferred | 페어 Phase 3 -0.5주 |
| 튜토리얼 텍스트 deferred | 승환 Phase 2 -0.5주 |
| AirCombo 단순화 (단발 고정) | 승환 Phase 2 -1주 |
