# 버터 MM 로코모션 재구축 (진행 중 — Step 단위 기록)

> 2026-07-08 시작. **각 Step 완료 시마다 섹션 추가** — 전체 완료 시 이 문서가 최종 기록이 됨.
> 배경: ABP_Butter가 IdaFaber 데모용 ABP_ThirdPerson_F의 자식이라 출발/정지/방향전환 모션 부재 → GASP(Game Animation Sample) 애니를 리타게팅해 버터 전용 Motion Matching으로 재구축.
> 활성 플랜: `~/.claude/plans/happy-gliding-sky.md`

## 전체 구조 (6-Step)

1. ✅ UEFN→SKEL_UE5_F 리타게터 셋업
2. ✅ 배치 리타게팅 581개
3. ✅ MM 데이터베이스 구성 (스키마 4 + PSN 1 + PSD 6)
4. ✅ ABP_Butter_MM 독립 재구축
5. ✅ AnimClass 교체 + PIE 검증 + 튜닝/다이어트
6. ✅ 마무리 (리네임 스킵 — ABP_Butter_MM 이름 유지 결정)

---

## Step 1: 리타게터 셋업 ✅ (2026-07-08, MCP)

**무엇을**: `/Game/TrickalFanGame/Retarget/`에 `IK_UEFN_Mannequin` + `RTG_UEFN_to_UE5F` 생성.

**어떻게**:
- 소스 = GASP의 UEFN 마네킹 스켈레톤, 타겟 = 기존 `IK_SKEL_UE5_F` 재사용 (killdong 때 만든 것)
- Epic 표준 본 구조라 체인 28개 자동 매핑, 기본 오퍼레이션 6개로 충분
- 스팟 체크 2개(Idle/Walk) 변환 → 승환 눈 확인 통과

**결정**: 손-옷 클리핑(통바지)은 임시 캐릭터라 보정 없이 방치.

## Step 2: 배치 리타게팅 581개 ✅ (2026-07-08, MCP Python)

**무엇을**: GASP `UEFN_Mannequin/Animations/{Idle,Walk,Run,Sprint,Jump}` 전체 → `/Game/TrickalFanGame/Animation/MotionMatching/Locomotion/` 미러 구조로. Idle 17 + Walk 224 + Run 229 + Sprint 44 + Jump 67 = **581개**.

**어떻게**:
- 네이밍: search=`M_Neutral_` / replace=`AS_Butter_` 배치 옵션
- Crouch/BlendSpace 제외 (게임 스코프 밖), Box/Arc 스트레이프는 포함 (Phase 2 락온 재료)
- 리타게팅이 시퀀스 플래그(bEnableRootMotion/bLoop)를 자동 복사함 — 바이너리 대조로 확인, 수동 설정 불필요

**사건/해결**:
- **PoseSearch DDC 크래시** (`AnimSequence.cpp:1745` assert): GASP 원본 PSD들이 백그라운드 인덱스 빌드 중 압축 안 끝난 애니를 샘플링하다 사망. → 대응: 배치 후 즉시 저장 + 에디터 재시작 직후 배치 실행
- Run Transition 4개가 검색 필터에 안 걸려 누락 → 별도 재실행으로 회수

## Step 3: MM 데이터베이스 구성 ✅ (2026-07-09, 승환 수동 + AI 답안지)

**무엇을**: `/Game/TrickalFanGame/Animation/MotionMatching/`에 스키마 4종(`PSS_Butter_{Idle,Default,Stop,Jump}`) + 정규화 세트 `PSN_Butter` + 데이터베이스 6개.

**어떻게**:
- GASP 원본 스키마/PSN을 복제 → Skeleton만 SKEL_UE5_F로 스왑 (뼈 이름 동일해서 채널 유지)
- **6-DB 분할 근거**: GASP은 스키마 경계로 DB를 쪼갬 — 특히 PSS_Stop(미래 궤적 가중)이 발 브레이크 선택의 핵심이라 Stops를 별도 DB로 유지. 모든 DB가 PSN 공유 = 여러 DB 동시 검색 가능(점수 정규화 접착제)

| DB | 스키마 | 등록 수 |
|---|---|---|
| PSD_Butter_Idles | Idle | Stand_Idle_Loop + Break 6 |
| PSD_Butter_TurnInPlace | Default | Turn 8 + Idle_turn 2 |
| PSD_Butter_Loco | Default | **389/391** (W/R/S Loops+Starts+Pivots+Spin) |
| PSD_Butter_Stops | Stop | W/R/S Stops |
| PSD_Butter_Jumps | Jump | Jumps + Jumps_Far |
| PSD_Butter_Lands | Jump | Lands 전부 |

- 등록 목록은 GASP Dense PSD 바이너리에서 추출한 답안지(`~/.claude/tmp/gasp_db_contents.txt`) 기준 — GASP도 전량 등록 안 함(Arc/Strafe/Break 등 다이어트)
- PSD 설정 변경 2개만: 정규화 세트=PSN_Butter + 프리뷰 메시=SK_MechanicGirl_01 (나머지 전부 기본값 — LoopingCostBias -0.005, PCAKDTree, KNN 200)

**미결**: Loco 389 vs 391 (2개 미상) — PIE 후 Pose Search Debugger로 확인 예정.

## Step 4: ABP_Butter_MM 재구축 ✅ (2026-07-09, 승환 수동 배선 + AI 가이드)

**무엇을**: 순수 AnimInstance 부모의 독립 ABP `ABP_Butter_MM` — 컴파일 통과 + PIE에서 MM 로코모션 가동 확인.

**최종 배선**:
- 변수 4개 (전부 ABP 내부 소유, **C++ 수정 0줄** — Property Access로 캐릭터 읽기만):
  - `bIsInAir` ← TryGetPawnOwner→GetMovementComponent→IsFalling
  - `bIsMoving` ← Velocity XY길이>10 **OR** GetLastMovementInputVector XY길이>0 (입력 포함 = 출발 즉각 반응)
  - `Trajectory` / `DatabasesToSearch`(**배열**) ← ThreadSafe 함수가 매 프레임 채움
- ThreadSafe Update 함수: 궤적 = `Pose Search Generate Trajectory (for Character)` → `HandleTrajectoryWorldCollisions`(**Apply Gravity 필수**) → SET Trajectory. DB 분기 3개: 공중→[Jumps,Lands] / 지상+이동→[Loco,Stops] / 지상+정지→[Idles,TurnInPlace,Loco]
- AnimGraph: `MotionMatching(Database 핀 비움) → Pose History(Trajectory 변수, Collected Bones=pelvis/foot_l/foot_r) → Slot 'DefaultSlot' → Output`
- DB 주입: MM 노드 **On Update 바인딩** `UpdateMMDataBases` — Node→Convert to Motion Matching Node→Set Databases to Search(Interrupt Mode=Do Not Interrupt)

**함정/해결** (상세는 메모리 `reference_mm_abp_runtime.md`):
1. PA 픽커가 벡터를 X/Y/Z 강제 분해 → .X/.Y 뽑아 Make Vector 재조립
2. PawnOwner 밑에 CharacterMovement 없음(APawn 타입) → GetMovementComponent 경유로 대체
3. 궤적 노드 InOut 핀 3개 연결 필수(기본값 불가) → "변수로 승격"
4. DatabasesToSearch가 단일 타입이면 Make Array 연결 거부 → 배열로 변경
5. 한글 UI: DB 주입 = "함수→**업데이트 시**"("모션 매칭 상태 업데이트 시" 아님). MM 노드에 포즈 히스토리 이름 항목 없음 — Source 연결로 자동 인식(5.6)
6. Set Databases to Search 검색 시 **변수 SET 노드**가 먼저 잡힘 → MM 노드 참조에서 드래그해 f 함수 노드 선택

## Step 5: AnimClass 교체 + PIE 검증 + 튜닝/다이어트 ✅ (2026-07-09~10)

- AnimClass → ABP_Butter_MM 교체, PIE에서 로코모션 전 영역 가동 확인

### 5-a. 스프린트 움찔 (박빙 동점자 churn) 해결
- 증상: 직진 스프린트 중 0.3~0.5초마다 Turn/Reface 클립으로 갈아탐 (리와인드 디버거 확인)
- 처방 ① Bias 튜닝: `PSD_Butter_Loco` Continuing Pose Cost Bias -0.01→**-0.05** (현직 프리미엄 강화, 런타임 전용) + Looping Cost Bias -0.005→**-0.02** (루프 클립 우대, 인덱스 재빌드)
- 처방 ② 커버리지 캡처 클립 제거: Box/Prism → 잔여 동족 Diamond/Hourglass/Reface/Shuffle까지 제거 (상체 고정 기하 경로 모캡 — 비락온 이동에 부적합)

### 5-b. 스블식 다이어트: Turn/Pivot 제거
- 근거: SB 전체 에셋 목록 검증 — **EVE 지상 로코모션용 Turn/Pivot 클립 0개** (Turn은 BalanceWalk/슬라이딩/수영 등 특수 상황 전용, 몬스터만 Turn 보유). 플레이어 회전 = 캡슐 회전 코드 담당
- `PSD_Butter_Loco`에서 `Turn*` 28 + `Pivot*` 40 제거. **20° 커브 루프(`Run_Loop_F_L/R_20` 등)는 유지** — Turn 제거 후 커브 달리기 품질 담당
- PSD_Butter_TurnInPlace(제자리턴 DB)는 유지 (스블도 필요 영역)
- PIE: 와리가리/커브/급반전 이상 없음. 관찰 항목: 급커브 스케이팅 / 180° 반전 팝 (거슬리면 Pivot 40만 부활이 1순위 카드)

### 5-c. 점프 전환 지연 해결 — Interrupt Mode
- 증상: 점프 시 걷기 클립이 공중에서 지속되다 뒤늦게 Jump 재생
- 원인: `Set Databases to Search`의 Interrupt Mode 기본값 `Do Not Interrupt` — DB 목록이 지상→공중으로 바뀌어도 현직 클립이 후보로 잔류 (강화한 Continuing Bias -0.05가 악화 요인)
- 해결: **`Interrupt on Database Change`** — DB 목록 전환 프레임에만 현직 퇴장, 평상시 영향 없음. 개념 정리는 메모리 `reference_mm_abp_runtime.md` § Interrupt Mode

### 5-d. 검증 결과
- ✅ 전투 회귀: Light/Heavy 콤보·Dodge·Parry 몽타주 DefaultSlot 정상 재생
- ✅ 몽타주 종료 → MM 복귀 팝 없음
- ✅ 점프 즉시 전환
- ⏭️ 폴리싱 이월: ① 급정지 Stop 품질 ② **점프 자세 구부정 → GS APose_Jump 3클립(Start/Loop/End)으로 PSD_Butter_Jumps 통째 교체 결정** (착지 속도별 디테일 포기 = 스블식 통일, 리타게팅 1회 + 루트모션/캡슐 물리 확인 필요) ③ 달리기 부자연(발 슬라이딩=CMC 속도, 몸 기울기/지면 밀착=OrientationWarping·FootIK)

## Step 6: 마무리 ✅ (2026-07-10)

- **리네임 스킵 결정**: ABP_Butter_MM 이름 그대로 유지 (구 ABP_Butter로 리네임 안 함)
- PIE "블루프린트 컴파일 오류" 빨간 문구: 실害 없음 판단, 무시 (구 ABP_Butter 삭제 시 재확인)
- Phase 1 MM 로코모션 재구축 **완료** → 다음: Phase 2 전투 애니 교체 + PSD_Combat (고스트 사무라이 A급 재료: Attack 97 / Deflect 60 / Execution 67 / Dodge 27 / Movement 54 전투 스트레이프)
