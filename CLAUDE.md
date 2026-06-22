# 프로젝트 CLAUDE.md — 2인 페어 UE5.6 + GAS

전역 `~/.claude/CLAUDE.md` 적용 (UE5 컨벤션 + Karpathy 4원칙). 본 파일 = **본 프로젝트 고유 룰**.

마스터 룰셋 = 본 파일 (단일 진실). 변경은 페어 합의 후.

---

## 0. 작업 영역

- **`.h` / `.cpp` = AI** — 작성자 의도 1~2줄 → AI draft → 작성자 30초 review → 승인/수정 1줄. architecture 통제권은 작성자 (의도 = 계약)
- **BP / 에셋 / Editor 작업 = 사람**
- **운영매뉴얼 = `docs/specs/<현재 마일스톤>-spec.md` (답안지)**
- 스펙 §결정사항 임의 변경 금지. 변경 시 페어 confirm → 스펙 patch → 진행

### 페어 프로그래밍 프로토콜 (의도 중심) ★

1. **작성자 의도** (1~2줄) — 클래스명 + 책임 + 핵심 노출
   - 예: `"HitStop 컴포넌트, 0.05~0.15s 시간 제어, BP에서 Duration 노출"`
2. **AI `.h` draft** — 의도 직결 멤버만 (사변적 virtual / hook / "혹시나" 추가 금지). UE5 매크로(UPROPERTY/UFUNCTION) + TObjectPtr + Null 체크 자동. 가정 1~2줄 보고
3. **작성자 review** (30초) — 승인 / 멤버 1줄 수정 / 가정 거부
4. **AI `.cpp`** — 헤더 시그니처 그대로. 1회용 인라인 OK, 2회+ 시 함수 분리 (Karpathy YAGNI)

**Edge cases:**
- 의도 모호 → AI가 1줄 질문 (다중 질문 X)
- "알아서" → AI가 최선 추측 + 가정 명시 보고
- UE5 API 불확실 → Context7 MCP 조회 (`/websites/dev_epicgames_en-us_unreal-engine`)

---

## 1. 아키텍처 룰 — 슈퍼 싱글톤 방지 ★

### 1-1. 금지 패턴
- 싱글톤 금지 (UE5 Subsystem 제외)
- `*Manager` 이름 클래스 금지
- GameInstance에 게임 로직 추가 금지 (런칭/영속성 관심사만)
- **Pawn 500줄 초과 금지** → 컴포넌트 분리
- **Component 300줄 초과 시 분리 검토**
- **GameplayAbility 200줄 초과 시 분리 검토**
- GA가 UI / Sound / Camera 직접 호출 금지 → GC 경유
- AttributeSet 어트리뷰트 8개 초과 시 분리

### 1-2. 필수 패턴
- 비주얼 / 오디오 효과 → **GameplayCue**
- 데미지 계산 → **ExecCalc** (인라인 금지) — 단, 단순 케이스는 `SetByCaller` 1회성
- 카메라 / HitStop / HitReact → **전용 Component**
- 캐릭터 간 통신 → **GE Context** (직접 포인터 금지)
- 영속성 → **SaveGameSubsystem**
- 글로벌 이벤트 → **GameplayMessageSubsystem** (Lyra 패턴)

### 1-3. 의존성 방향 (단방향 강제)

**허용:**
- Pawn → Component
- Component → ASC (읽기)
- GA → ASC, GE
- GC → Component (위임만)

**금지:**
- Component → Pawn (Owner 캐스팅 금지)
- Component → Component (직접 참조 금지, 메시지/델리게이트 사용)
- GA → Component (GC 경유)
- AS → 다른 시스템 (데이터만)

### 1-4. 슈퍼 싱글톤 방지 의식
- 매 코딩 세션 끝 5분 리뷰: 줄 수 + 의존성 방향 확인
- "어디 둘지 모르면 Pawn에" 금지 → 컴포넌트 후보 먼저 검토
- 주 1회 SOLID / God Class 점검
- 빠른 답이 나오면 의심: "결합도 높은 것 아닌가"
- AI 코드의 단위 테스트 가능성 질문 → 못 한다면 리팩토링 신호

---

## 2. GAS 컨벤션 함정

새 코드 짤 때 매번 확인:

1. **`ATTRIBUTE_ACCESSORS` 매크로** — 엔진 미제공. 새 `UAS_*` 헤더마다 `#define` 직접 추가
2. **`PreAttributeChange` 클램프** (예: Health 0~MaxHealth) / **`PostGameplayEffectExecute` 후처리**
3. **GAS 모듈 의존성 3개** — `GameplayAbilities`, `GameplayTags`, `GameplayTasks` 셋 다 `Build.cs PublicDependencyModuleNames`에 추가
4. **Tags 중앙 선언** — `NativeGameplayTags.h` 1개 파일. BP 등록 금지(머지 충돌). `UGameplayTagsSettings`는 `DefaultGameplayTags.ini`만 유효
5. **AT 깊이 2단계 이하** — AbilityTask 안에서 또 AT 호출 시 Call Stack 안 잡힘
6. **GameplayCue 5~10개만** — 모든 비주얼 GC 금지 (디버깅 끔찍). 나머지는 BP / AnimNotify
7. **GE 자식 CDO `AddComponent<>()` 금지** — UE5.6 fatal. `CreateDefaultSubobject` + `GEComponents.Add` 패턴 필수

> **Replication (멀티 대응)** — 멀티 확장이 결정된 시점부터 `UPROPERTY(Replicated)` / `OnRep_*` / `GetLifetimeReplicatedProps` / `GAMEPLAYATTRIBUTE_REPNOTIFY` 적용. 싱글 프로토타입 단계에서는 생략 가능 (페어 합의 후).

---

## 3. 프로토타이핑 룰

- **YAGNI**: 2회 이상 사용되는 코드만 함수 분리. 1회용은 인라인
- **수동 데이터 우선**: DataAsset / CurveTable 셋업은 후순위. 초기 마일스톤은 코드 안 상수
- **Editor 튜닝 우선**: 수치 변경 빈도 높은 값은 `UPROPERTY(EditAnywhere)` 노출 → BP에서 즉시 변경
- **ASC 모드 일관성**: 페어 합의 모드 1택 (예: Player = PlayerState / Enemy = Pawn 직결). 도중 변경 X
- **액션 우선순위 (참고)**: 입력 반응(1) > 타격 표시(2) > 시각 폴리싱(3)

→ 본 룰셋과 §1 슈퍼 싱글톤 방지 룰은 **공존**. 단순함과 분리는 다른 차원.

---

## 4. 검증 게이트

- 각 마일스톤 종료 시 해당 스펙의 §검증 항목 모두 ✓
- 마일스톤 성공 기준 미달 시 다음 마일스톤 진입 금지

---

## 5. 2인 협업 룰 ★

### 5-1. 코드 / 헤더 권한
- **`.h` 설계 권한 = 도메인 오너 1명** (Combat / AI / 입력 / 카메라 / GAS Core / UI 영역 분담)
- 타 도메인 `.h` 변경 필요 시 → 오너 confirm 1줄
- `.cpp`는 둘 다 자유 (인터페이스 깨지 않는 범위)
- `CLAUDE.md` / 스펙 변경 = 둘 다 합의

### 5-2. Perforce
- Workspace 네이밍: `<이름>_<PC명>`
- Stream: `//<프로젝트>/main` 1개 (개인 dev branch 없음, 페어 합의로 변경 가능)
- **체크아웃 충돌**: 먼저 한 쪽 우선. 늦은 쪽은 페어 채널 ping
- **Exclusive lock 풀기**: 퇴근 전 `Revert if Unchanged` + 작업 마무리한 `.uasset` / `.umap` submit
- **CL 단위**: 1기능 / 1CL. 24h 이상 보유 금지
- **빌드 바이너리** (`Binaries/`): depot 공유 안 함, 각자 로컬 빌드. `.p4ignore`로 제외 (2026-05-21 결정 — 바이너리 공유가 페어 환경에서 비효율로 판명)
- **CL 메시지**: `[태그] 한 줄 요약` + 본문(왜). 태그 예: `[Combat] [GAS] [Input] [Camera] [BP] [fix] [refactor] [doc] [chore]`

### 5-3. 페어 리뷰
- **Submit 전 셀프 리뷰 필수** (Diff Against Have)
- **페어 리뷰 트리거**: 아키텍처 변경 / 새 Component / 새 GA / 새 `.h` / 200줄+ CL
- 자잘한 수정은 셀프만 OK
- 리뷰 SLA: 24h 안

### 5-4. P4에 올리는 프로젝트 자산
- 프로젝트 `CLAUDE.md`, `docs/specs/`, `docs/design/`, `docs/reference/` = P4 submit
- **`.claude/` 전체 = P4 X (`.p4ignore`)** — skills · hooks · `settings.local.json` · state 전부 개인. 머신 절대경로(빌드 경로 등)는 워크스페이스마다 달라 공유 불가 → 각자 로컬 보유 (예: `ue-build-check` 는 각자 자기 워크스페이스 경로로)
- 개인 비밀 (`access.json`, API key, token) / 개인 글로벌 `~/.claude/` = P4 X
- 개인 워크플로우는 본인 스타일대로 — 강요 X

### 5-5. 싱크 / 블로커
- **데일리 싱크** (5분): 어제 / 오늘 / 블로커
- **30분 막힘 룰**: 30분 막히면 페어 / AI ping
- **단독 결정 시**: 본 CLAUDE.md / 스펙에 1줄 기록 (결정 비대칭 방지)

---

## 6. 참조 자료

- 본 `CLAUDE.md` — 프로젝트 룰 단일 진실
- `docs/specs/<현재>-spec.md` — 현재 마일스톤 답안지 (페어 작성, 검증 게이트) ★
- `docs/design/` — 기획 문서 (GDD, 메커닉, 시스템 설계)
- `docs/reference/` — Perforce 가이드 pptx, typemap 원본 등 외부 자료
- `~/.claude/CLAUDE.md` — UE5 + Karpathy 전역 (각자 개인 환경)

---

## 7. 작업 라우팅 (OMC agent + skill)

슬래시 커맨드는 두지 않음 — OMC agent와 중복(Karpathy §2 YAGNI). 도메인 지식(§1~3)은 본 CLAUDE.md에 박혀 있어 agent 호출 시 자동 주입.

### OMC agent 위임 (컨텍스트 격리 + 모델 선택)
| 작업 | 위임 대상 | 비고 |
|---|---|---|
| 설계 검토 | `architect` agent (READ-ONLY) | 구현 전 대안 비교 |
| `.cpp` 구현 | `executor` agent (`model=opus` 복잡 시) | 작성자 `.h` 시그니처 준수 |
| 결함 검출 | `code-reviewer` agent | GAS · Authority · 아키텍처 체크 |
| 에러 진단 | `debugger` agent | 빌드 / 런타임 근본 원인 |
| 구조 개선 | `code-simplifier` agent | 동작 변경 X |
| 검증 | `verifier` agent | 변경이 실제로 작동하는지 |

### 프로젝트 skill (`.claude/skills/`)
| skill | 시점 | 역할 |
|---|---|---|
| `ue-build-check` | `.h`/`.cpp` 수정 직후 | UE5.6 Build.bat 자동 호출, 에러/경고 분리 보고 |

> `.h` = 사람 영역. agent도 `.h` 수정 금지 — 시그니처 변경 필요 시 도메인 오너에게 보고.
