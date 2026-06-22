# Project_KD 데모 — 3 Phase

> 3달 = 12주. 페어 2명. **"빌드를 켜면 보이는 것"** 기준.
> 결정·owner·데이터·자산 = `references.md` / 진입점 한 페이지 = `README.md`

---

> 분담 한 줄 + 변경점 = `README.md`. 본 문서 = 시간순 + Phase 디테일.

---

# 시간순 작업표

| Week | 필규 | 승환 | 페어 |
|---|---|---|---|
| **Day 0** (필규 1일) | GAS 인프라 + prototype 6종 마이그·fix + Perforce 첫 CL (상세 = §Day 0) | — | 합의 1시간 |
| **W1** | 단검 잡몹 + Collision/NavMesh | Player Pawn 신규 + IMC+IA 7 + 4 GA + 콤보 6 | 매일 30분 sync |
| **W2** | 손맛 페어 통합 참여 | 손맛 페어 통합 참여 + Player Pawn 마무리 | 손맛 3층 + HUD 베이스 + 사망/재시작 + Meshy 워크플로우 검증 |
| **게이트 W2** | **30초 슬라이스 즐거운가?** ✓ → W3 | | |
| **W3~5** | 잡몹 3종 BT + GA + Stagger 시스템 + 적 측 UI 4 WBP + HUD 3바 + 보스 기획 마무리 | 도술 3종 (화부 Projectile / 풍보 5단계 / 정승 Cone) + Doul 차오름 + 인벤 GA + 도술/인벤 슬롯 UI | 주 1회 2시간 sync + Meshy 잡몹 4종 + **마켓플레이스 애니/사운드/VFX 팩 평가·구매** |
| **W6~8** | 호위 엘리트 (3패턴) + 보스 BT + Phase swap + P1·P2 패턴 + 보스 HP·Stagger UI | AirCombo(R) 6단계 + F 처형 GA + QTE 프롬프트 UI + MMB 락온 + 캔슬 UPROPERTY | 주 1회 sync + 카메라 1차 (전투 거리·락온) + Meshy 호위·보스 외형 |
| **게이트 W8** | **풀세트 통합 빌드 작동?** ✓ → W9 | | |
| **W9~10** | 보스 P2 결전 마무리 + 적 측 분위기 + Stagger 수치 튜닝 | Beat grant 로직 + 도술 trace 마무리 + **카메라 2차** (락온 연동·1:1 줌·QTE·풍보 공중정지·보스전) | 레벨 5구역 + **NavMesh 셋업** + 체크포인트 5 (`UGdSaveGameSubsystem` 트리거) + 환경 인터랙션 액터 + BGM 페이즈 + 채도 PostProcess |
| **W11~12** | 적·보스·적측 UI 폴리싱 + 시그 #2 | Player·UI 폴리싱 + 시그 #1·#3·#4 | 시스템 메뉴 사이클 (메인메뉴/일시정지/저장/종료) + 카메라 폴리싱 + 시그 #5 + Meshy 디테일 + 사운드/VFX 통합 + **Shipping 빌드 1회** + 매일 풀 플레이 |
| **게이트 W12** | **Shipping 빌드 동결** (P3 광폭화 = 시간 남으면 보너스) | | |
| **W13+** | 스토브 인디지원 신청 | — | Steam 페이지 + 트레일러 + 외부 QA + 8~9월 Next Fest 등록 |

---

# 영역 한눈 정리

| | 필규 | 승환 | 페어 |
|---|---|---|---|
| GAS 인프라 | Tags / AS_Char / GE_Damage / AttributeMacros / **prototype 6종 마이그·fix** (Day 0) | — | — |
| 프로젝트 골격 | Collision Channel + NavMesh (W1) | IMC+IA + GameMode 셸 (W1+ 필요 시) | SaveGame + Subsystem (W9~10) |
| Player | — | **길동 Pawn 베이스 신규** + 콤보 GA 4 + 도술 GA 3 + AirCombo + F 처형 + 인벤 + MMB 락온 | — |
| 적 | 잡몹 4 + 호위 엘리트 + 보스 P1·P2 + Stagger | — | — |
| UI | HUD 3바 + 적 측 UI 4 WBP | Player UI 3 WBP | 시스템 메뉴 사이클 |
| 3D 자산 | — | — | Meshy 6 모델 (잡몹 4 + 엘리트 + 보스) |
| 레벨·연출 | — | — | 레벨 5구역 + 체크포인트 5 + 환경 + BGM + 채도 + 카메라 + 시그 #5 |
| 기획 ✓ | 적·보스 | 도술·콤보·UI·QTE·분위기·전투진행 | — |
| 시그 모먼트 | #2 (5:00) | #1 (0:30) / #3 (10:00) / #4 (15:00) | #5 (25:00) |
| 운영 | AWS / Perforce | — | — |

---

# Phase 1 — Vertical Slice (W1~2)

> **길동 1명이 단검 잡몹 1마리를 30초 안에 죽이고 죽는 빌드.**
> 이게 즐거우면 게임. 안 즐거우면 다 망함.

## W2 끝에 빌드 켜면

- 빈 회색 맵 + 길동 + 단검 잡몹 1마리
- WASD 이동 / Space 점프 / 마우스 카메라
- LMB 약공 3타 + RMB 강공 1타 + Shift 회피 + Q 패링
- 잡몹 추격 → 단검 휘두름
- 때리면 화면 살짝 멈춤 + 본/메시 흔들림
- 좌상단 HP/Stamina/Doul 3바 + 적 머리 HP 바
- 사망 → 재시작

## 제외 (Phase 2 이후)

도술 / 인벤 / AirCombo / 보스 / 다른 잡몹 / Stagger / 캔슬 / 레벨 / 메뉴 / 저장 / 사운드 / VFX 완성도

## 검증

1. 30초 안에 잡몹 1마리 죽일 수 있다
2. Dodge i-frame 작동 (Shift 누르면 적 공격 통과)
3. Q로 적 공격 막으면 적 잠깐 멈춤
4. 때리면 화면 멈춤 체감
5. HP 바 실시간 갱신
6. 사망 처리 작동

## 작업 분담

### Day 0 (필규 1일)

**페어 합의 (1시간)**: Perforce CL 룰 + Claude Code 환경 + 페어 리뷰 SLA + 데일리 sync

**필규 작업**:
- GAS 인프라: `Build.cs` GAS 3종 + `DefaultEngine.ini` ASC + `DefaultGameplayTags.ini` + `GdAttributeMacros.h` + `GdGameplayTags.h` 셸 + `UGdCharacterAttributeSet` (HP·MaxHP·Stamina·MaxStamina·AttackPower) + `UGdGE_Damage_Physical` (SetByCaller)
- prototype 6종 마이그·fix: 상세 = `references.md` §prototype 코드 표 (결함 없음 6종 sed rename / `WeaponTraceBase`·`AT_WeaponTrace` fix 5개 완료 후 복사)
- Perforce 첫 CL: `[chore] initial repo setup`

> ⚠ 프로젝트 골격 = YAGNI. UE5 디폴트 GameMode/PC/GI 그대로 시작. IMC+IA·Collision·NavMesh·SaveGame은 닿는 사람이 W1+ 필요 시점에 추가.

### Week 1
- **필규**: 단검 잡몹 (`AGdEnemyBaseCharacter` + BT + `UGdGA_Enemy_BasicAttack`) + Collision Channel (WeaponTrace / Pawn / Projectile / Environment) + NavMesh (Recast Dynamic)
- **승환**: Player Pawn 신규 (`AGdPlayerCharacter` — prototype `HKDPlayerCharacter` 재설계) + IMC_Default + IA 7개 (Move/Look/Jump/Light/Heavy/Dodge/Parry, 도술/Inv/AirCombo는 W3+에 추가) + 4 GA + 콤보 6 시퀀스. prototype `GA_LightAttack`/`GA_Dodge` raw 참고 가능 (fix 책임 = 승환)

### Week 2 (페어)
- 손맛 3층 통합 (HitStop GCN + 본 흔들림 Component + WPO Material)
- HUD 베이스
- 사망/재시작
- 매일 30분 sync (빌드 같이 봄)

---

# Phase 2 — Owner 갈라짐 (W3~8)

> **잡몹 5종 + 보스 P1·P2 + 도술 3종 + 공중QTE + UI 풀세트가 작동하는 빌드. 25분 흐름은 아직 없음.**

## W8 끝에 빌드 켜면

- Phase 1 빌드에 추가:
  - 잡몹 3종 더 (활/도끼/무당)
  - 호위 엘리트 1:1 게이트 (3패턴)
  - 보스 P1·P2 (HP 70% 전환)
  - 도술 3종 (`1` 화부 / `2` 풍보 5단계 / `3` 정승 Stagger 감소)
  - Doul 차오름 (적중 + 패링)
  - 인벤 2종 (`4` 술병 / `5` 약초)
  - 공중QTE (`R`, T1=Stagger 0 / T2=점프 공격)
  - Stagger 게이지 (잡몹 UI X / 보스 UI O)
  - MMB 락온 + ESC 일시정지

## 작업 분담

### 필규 (적·시스템·HUD)
- 잡몹 3종 추가 + 호위 엘리트 + 보스 P1·P2
- **Stagger 시스템** (attribute + 잡몹/보스 차등 로직)
- 적 측 UI 4 WBP (적 HP / `[F]` 프롬프트 / 보스 HP 3분할 / 보스 Stagger)
- HUD 3바 (HP/Stamina/Doul)
- 도술 3종 적 측 반응 (Stagger 감소 / 광역 hit)
- Meshy 6 모델 (잡몹 4 + 엘리트 + 보스 P1·P2 외형, 각시탈)
- 보스 기획 마무리 + Stagger 수치 1차 튜닝

### 승환 (Player·Player UI)
- **도술 3종** (Trace 각자 직접 — references.md 도술 trace 참조)
  - 화부 = Projectile + Sphere Overlap (부적 다발)
  - 풍보 = Mobility 5단계 (차지→점프→공중정지→활공→착지)
  - 정승 = Cone 110°/10m + Stagger GE
- Doul 차오름 로직 (적중 + 패링 누적)
- AirCombo (R) 6단계 + 입력 패턴 차등
- 잡몹 F 키 처형 GA (Stagger 0 트리거)
- 인벤토리 GA (술병/약초)
- MMB 락온 시스템
- Player UI 3 WBP (도술 슬롯 / 인벤 슬롯 / QTE 프롬프트)
- 캔슬 UPROPERTY 박기

### 페어 (sync + 카메라 1차)
- 주 1회 2시간 sync — 손맛 회귀 점검 + 캔슬 충돌 + 통합 빌드
- 게임플레이 카메라 1차 (W6~8): 전투 거리·락온·Stagger 처형 컷

## 검증

**W5 중간**
1. 잡몹 4종 모두 작동
2. 도술 3종 발동 (화부 폭발 / 풍보 5단계 / 정승 Stagger 감소)
3. Doul 차오름 (시작 0%)
4. 잡몹 Stagger 0 → `[F]` 프롬프트 + 처형 모션
5. Phase 1 손맛 회귀 0

**W8 끝**
1. 호위 엘리트 3패턴 + 1:1 게이트 카메라 줌인
2. 보스 P1·P2 전환 (HP 70%)
3. 보스 Stagger HUD 하단 + Stagger 0 시 공중QTE T1 자동
4. R 누르면 6단계 시퀀스
5. MMB 락온 토글
6. UI 풀세트 작동
7. 30분 플레이 테스트 크래시 0

---

# Phase 3 — 통합 + 폴리싱 (W9~12)

> **5구역에서 25분 진행 → 보스 클리어 → 데모 클리어 화면 = 출시 가능.**

## W12 끝에 빌드 켜면

- 메인메뉴 (시작 / 종료)
- 5구역 진입 + 첫 야영지 (도적 통조림 잡담 → 발각 → 전투):
  - 1구역 산기슭 (0~3분, 단검, 모놀로그 컷)
  - 2구역 입구 (3~5분, 단검+활, 잡몹 다운 컷)
  - 3구역 내부 (5~10분, 도끼+무당, 화부 첫 발동 컷)
  - 4구역 내부 (10~15분, 환경 인터랙션, 정승 첫 발동 컷)
  - 5구역 두목 앞마당 (15~20분 호위 엘리트 / 20~25분 보스 P1·P2)
- 체크포인트 5 + 즉사 → 마지막 체크포인트 복원
- 보스 P2 처치 → **처형 컷 25:00** + 데모 클리어 화면
- 인게임/아웃게임 흐름: 메인메뉴 → 게임 → 일시정지 → 저장 → 종료 → 메인메뉴 복귀
- 정오 맑음 + 황토색 + 흙먼지/낙엽/산바람 VFX
- BGM 4트랙 (평시·전투·P1·P2) + 채도 -30%/-50% + SFX 풀세트

> P3 광폭화 추가 시 (시간 남으면): 보스 0.4배속 + Stamina·Doul 무제한 + 캔슬 풀오픈 + 각시탈 산산조각 + BGM P3 + 채도 -40%

## W9~10 — 통합 (2주)

### 페어
- 레벨 5구역 BP + 첫 야영지 + **NavMesh 셋업** (Recast Dynamic + 환경 액터 NavLink)
- 트리거 볼륨 (구역 진입 시 스폰 + Beat grant)
- 환경 인터랙션 액터 (통나무·짚단·대용폭탄)
- 체크포인트 5 (Day 0 `UGdSaveGameSubsystem` 위에 트리거)
- BGM 페이즈 가변 + 채도 PostProcess

### 필규
보스 P2 결전 마무리 (처치 → 결말 트리거) + 적 측 분위기 (사망 모션 H)

### 승환
Beat grant 로직 (도술 5/10/15) + 도술 3종 trace 마무리 폴리싱 + **게임플레이 카메라 2차** (락온과 자연 연동, 1:1 줌·QTE·풍보 공중정지·보스전)

### W10 검증
1. 처음부터 끝까지 한 번에 25분 플레이
2. 보스 P2 클리어 → 데모 클리어 화면
3. 5개 체크포인트 작동
4. 인게임/아웃게임 사이클 작동
5. BGM 페이즈 전환 자동
6. 환경 인터랙션 봉 트리거
7. 게임플레이 카메라 핵심 4종

## W11~12 — 폴리싱 + 출시 빌드 (2주)

### 작업
- 수치 튜닝 (HP / Stagger / 데미지 / 회복량 / HitStop)
- VFX 본 작업 (HitImpact / 도술 3종 / 페이즈 전환) — 자체 + 마켓플레이스 통합
- 사운드 통합 (BGM 4트랙 + SFX 풀세트 + 국악) — 마켓플레이스 팩 통합
- 카메라 폴리싱 (전체 튜닝)
- 시그니처 모먼트 5개 Sequencer
- **시스템 메뉴 사이클** (페어) — 메인메뉴 / 일시정지 / 저장 슬롯 1 / 종료 → 메인메뉴 복귀
- 버그 픽스
- 30분 플레이 테스트 5회+
- **Shipping 빌드 1회** (W12 마지막 주 = cooking 통과 검증, 평균 60 FPS 측정)

### 시그니처 모먼트 분담
| # | 시간 | 모먼트 | 담당 |
|---|---|---|---|
| 1 | 0:30 | 모놀로그 + 발끝 보는 컷 | 승환 |
| 2 | 5:00 | 잡몹 황당 다운 0.5s | 필규 |
| 3 | 10:00 | 화부 첫 발동 | 승환 |
| 4 | 15:00 | 정승 첫 발동 ("어흠—") | 승환 |
| 5 | 25:00 | 처형 컷 + 어깨 으쓱 | 페어 |

### W12 출시 빌드 게이트 (= "Shipping 빌드 동결")
1. 5회 풀 플레이 크래시 0 + ensure trigger 0
2. 25분 안에 보스 P2 클리어 가능
3. 손맛 Phase 1 30초 슬라이스만큼 즐거움 (회귀 0)
4. UI 모든 항목 placeholder 0
5. BGM 4트랙 페이즈 전환 + SFX 풀세트
6. 시그니처 5개 발현 (0:30 / 5:00 / 10:00 / 15:00 / 25:00)
7. 슬로우모 4종 트리거 + 채도 단계별
8. 게임플레이 카메라 6종 자연스러움
9. 인게임/아웃게임 사이클 완전 작동
10. **Shipping 빌드 cooking 통과 + 평균 60 FPS (5분 연속 측정)**

> 보너스 (P3 추가 시): 광폭화 자동 진입 5회 + 각시탈 파괴 컷

---

# Phase 4 — 출시 준비 (W13~, 12주 외 후속)

> W12 = Shipping 빌드 동결. **Steam 즉시 공개 X.** 시장 노출까지 +N주.

## 트랙

| 일정 | 작업 | Owner |
|---|---|---|
| W13 | 스토브 인디지원 프로그램 신청 | 필규 |
| W13~14 | Steam 페이지 개설 (스크린샷 8 + 짧은 영상 + 설명 + 시스템 요구사항 + Steamworks 셋업) | 페어 |
| W13~15 | 데모 트레일러 영상 (1~2분, OBS 캡처 + 편집) | 페어 |
| W14~15 | 외부 QA 라운드 (5~10명) + 버그 hotfix | 페어 |
| W15 | EULA / 개인정보 처리방침 (한·영) | — |
| 8~9월 사이 | **Steam Next Fest 등록** | 페어 |
| Next Fest 직전 | 마케팅 (트위터 / 유튜브 / 디스코드 / 언리얼 코리아 / 인디라 등) | 페어 |

## 시스템 요구사항 측정

- 저사양 PC 1대 + 권장 사양 1대로 측정
- 최소: 30 FPS / 권장: 60 FPS 기준

## 출시 후

- Steam 커뮤니티 + 디스코드 피드백 채널
- hotfix 1~2주 단위
- 본편 기획 시작

---

# 게이트 (페이즈 진행 조건)

| 게이트 | 조건 | 실패 시 |
|---|---|---|
| **W2** Phase 1→2 | 30초 슬라이스 즐거움 (페어 둘 다 OK) | Phase 1 1주 연장 → Phase 2 5주로 압축 |
| **W8** Phase 2→3 | 잡몹 5 + 보스 P1·P2 + 도술 3 + AirCombo + UI 풀세트 통합 빌드 작동 | Phase 3 통합(W9~10)에 흡수 → 폴리싱 1주로 압축 |
| **W12** 출시 | 위 9개 검증 기준 | — |

---

# PD 원칙 (전 페이즈 공통)

1. **빌드 우선** — 매 페이즈 끝에 플레이 가능한 빌드
2. **회귀 금지** — Phase 1 손맛이 후속 페이즈에서 망가지면 즉시 fix
3. **owner 명확** — Phase 2부터 영역 침범 시 페어 합의
4. **욕심 자르기** — 추가 잡몹/패턴/메뉴 제안 = "Phase 4 (출시 후)"로
5. **사운드/VFX 마지막** — Phase 3 전까지 placeholder

> 다음 액션 = `README.md` §즉시 다음 액션 참조
