# 프로젝트 CLAUDE.md — 1인 솔로 UE5.6 + GAS

전역 `~/.claude/CLAUDE.md` 적용 (UE5 컨벤션 + Karpathy 4원칙). 본 파일 = **본 프로젝트 고유 룰**.

마스터 룰셋 = 본 파일 (단일 진실). 변경 시 본 파일에 날짜와 함께 기록 후 진행.

---

## 0. 작업 영역

- **`.h` / `.cpp` = 작성자가 직접 작성. AI는 설계·가이드까지** (2026-08-12 변경) — 작성자 의도 1~2줄 → **AI가 설계 + 넣을 위치·코드 제시** → **작성자가 직접 입력** → AI 검토. architecture 통제권은 작성자 (의도 = 계약)
  - **이유 = 작성자가 코드를 직접 보고 이해하기 위함.** AI가 대신 치면 그 기회가 사라진다
  - **AI의 코드 편집은 작성자가 "네가 수정해"라고 명시 요청할 때만.** "구현해줘" "코드 수정하고 빌드 가자"는 편집 허가 X
  - 가이드 형식 = `파일 경로 : 줄 위치` + 코드 블록 + include·전방선언 등 부수 변경까지
  - **서브에이전트에도 동일 적용** — 조사·설계까지만, Edit/Write 위임 금지
- **BP / 에셋 / Editor = 사람 원칙** — 단, 아래는 Unreal MCP로 AI 허용 (2026-07-12 결정):
  - 테스트맵/그레이박스 생성, 몽타주 노티파이 배치, 일괄 반복작업(수치 일괄 변경 등), 조회/스크린샷/PIE 검증
  - **본편 레벨·핵심 BP 로직·아트 에셋은 사람 유지**
  - **MCP 에셋 작업 전 Content 커밋 의무** (`cd Content && git add -A && git commit`)
  - MCP 세션 중 자동승인(bypassPermissions) 금지 / `bAllowNonLoopback` 금지
- **운영매뉴얼 = `docs/specs/<현재 마일스톤>-spec.md` (답안지)**
- 스펙 §결정사항 임의 변경 금지. 변경 시 스펙 patch + 1줄 기록 → 진행

### 세션 시작 프로토콜 ★ (2026-07-27 신설, 2026-07-28 확장)

**설계·제안 전에 아래를 읽는다. 매 세션 예외 없이.**

#### 1) 필수 MD 3개 (순서대로)

| 문서                              | 왜                                                     |
| --------------------------------- | ------------------------------------------------------ |
| `docs/PROJECT_OVERVIEW.md`        | 현재 방향·아키텍처·진행상황. **부트 문서**             |
| `docs/INDEX.md`                   | 전체 카탈로그. 이번 작업에 뭘 더 읽을지 여기서 고른다  |
| 이번 작업 영역의 **최신 dev-log** | 직전에 뭘 정했는지. INDEX 하단 dev-logs 표에서 최신 순 |

- **`docs/archive/` 는 읽지 않는다** — 폐기된 버터/길동 라인. 여길 현역으로 읽고 설계하는 게 사고의 주원인
- 기획 참조가 필요하면 `docs/design/기획/` — 단 **스토리·세계관은 무효(길동 기반), 시스템 설계만 유효**

#### 2) 코드 읽기 (실측 기준 · 2026-07-28)

현재 규모: **`.h` 86개 = 4,000줄 / `.cpp` 83개 = 6,900줄.**

- **`.h` 는 전수로 읽어도 된다** (4,000줄, 한 세션에 감당 가능). 코드 작업이 예정된 세션은 **헤더 전수 훑기를 기본으로** 한다
- **`.cpp` 는 닿는 것만.** 전체는 과하다(6,900줄)
- 읽는 순서: 디렉터리 구조 → 닿는 영역 `.h` 정독 → 나머지 `.h` 훑기 → 관련 `.cpp`

#### 3) 그 위에서 설계 — 기존 구현이 진실 기준

- **금지**: 코드를 안 읽은 상태에서 클래스·필드·흐름을 추측해 설계안을 내는 것
- 이미 있는 것을 다시 만들자고 제안하거나, 없는 것을 있다고 말하는 사고의 원인이 전부 이것
- **에셋 값도 마찬가지다.** BP·DA·몽타주의 실제 값은 MCP로 조회해 확인할 것 — 코드 기본값과 다를 수 있고, **폐기된 라인의 값이 남아 있는 경우가 실제로 있었다**(2026-07-28: 공격 GA 5개가 길동 창 소켓 `Spear_Tip`을 물고 있어 근접 판정이 죽어 있었음)
- **새 클래스 제안 전 같은 역할의 기존 클래스 검색 의무** (2026-08-03 신설) — 클래스 선언 전수 조회 1회(`rg "class PROJECT_KD_API"`)면 끝난다. 생략하면 중복 구현. 실제 사례: 히트스톱 설계 중 Bone Shake를 "없다"고 두 번 단정했으나 `UHitFeedbackComponent`에 이미 구현돼 있었다

### 코딩 프로토콜 (의도 중심) ★

1. **작성자 의도** (1~2줄) — 클래스명 + 책임 + 핵심 노출
   - 예: `"HitStop 컴포넌트, 0.05~0.15s 시간 제어, BP에서 Duration 노출"`
2. **AI `.h` 설계 제시** — 의도 직결 멤버만 (사변적 virtual / hook / "혹시나" 추가 금지). UE5 매크로(UPROPERTY/UFUNCTION) + TObjectPtr + Null 체크 자동. 가정 1~2줄 보고. **작성자가 입력**
3. **작성자 review** (30초) — 승인 / 멤버 1줄 수정 / 가정 거부
4. **AI `.cpp` 설계 제시** — 헤더 시그니처 그대로. 1회용 인라인 OK, 2회+ 시 함수 분리 (Karpathy YAGNI). **작성자가 입력 후 AI 검토**

**Edge cases:**

- 의도 모호 → AI가 1줄 질문 (다중 질문 X)
- "알아서" → AI가 최선 추측 + 가정 명시 보고
- UE5 API 불확실 → ① 로컬 엔진 소스 확인 (`D:\epicStore\UE_5.6\Engine\Source`) ② Context7/웹 문서 (개념·사용법 — UE 색인 얕음, 과신 금지)

### 주석 문체 ★ (2026-08-11 명문화 — 코드 쓸 때마다 적용)

**합격본 = `Source/Project_KD/Combat/KDProjectile.h` / `.cpp`.** 새 주석은 이 파일을 기준으로 삼는다.

| 규칙                                   | X                                         | O                                   |
| -------------------------------------- | ----------------------------------------- | ----------------------------------- |
| **명사구로 끝낸다** (문장체 X)         | `발사체는 Pawn이 아니다`                  | `발사체 = Projectile`               |
| **이게 무엇인지만** (왜·누가 읽는지 X) | `퍼펙트 회피 대상 여부 — GA_Dodge가 읽음` | `퍼펙트 회피 대상 여부`             |
| **부정은 `X` / `x`**                   | `인스턴싱 안 됨`                          | `인스턴싱 X`                        |
| **특수문자 X** (`★` `⚠️` 한자)         | `★ 필수`                                  | `필수` — `—` `\|` `/` `=` `→` 는 OK |
| **결과·경고 문장 삭제**                | `안 하면 판정이 조용히 죽는다`            | (주석에서 빼고 dev-log·대화로)      |
| **불리언은 `~ 유무`**                  | `조준 해제 시 피치 복귀`                  | `피치 중앙 복귀 유무`               |
| **추상 표현 X → 실제 변수명**          | `위치 = 부모 결과`                        | `위치 = RelativeSocketLocation`     |
| **비유·의인화 X**                      | `카메라가 설 자리`                        | `카메라 위치`                       |

- **함수 본문 첫 줄에 `// 기능 : ~`** — 헤더 선언부가 아니라 여는 중괄호 다음 줄
- **계산 단계마다 위쪽 한 줄** — 무엇을 구하는 줄인지만
- 여러 줄이면 `A = B` 나열형 / `switch case`·대입문 옆 인라인 짧은 주석 OK
- 용어: **"붐" X → "카메라스프링"** / **"열쇠" X → "키"** / **"정함" X → "선택"**

> 승환 평 — **"쓰잘데기 없이 너무 길고 이해가 안 돼."** 주석이 길면 코드를 가린다. 배경은 문서에 있으니 주석은 "이 줄이 뭐냐"만 답한다.
> 자가검사: 종결어미(`~한다` `~이다`)가 있으면 명사구로 줄인다 → 특수문자 스캔 → 결과·경고 문장 삭제.

---

## 1. 아키텍처 룰 — 슈퍼 싱글톤 방지 ★

### 1-1. 금지 패턴

- 싱글톤 금지 (UE5 Subsystem 제외)
- `*Manager` 이름 클래스 금지 — 단 **엔진 기반 클래스 상속으로 이름이 강제되는 경우 제외**(`APlayerCameraManager` 등)
- GameInstance에 게임 로직 추가 금지 (런칭/영속성 관심사만)
- **Pawn 500줄 초과 금지** → 컴포넌트 분리
- **Component 300줄 초과 시 분리 검토**
- **GameplayAbility 200줄 초과 시 분리 검토**
- GA가 UI / Sound / Camera 직접 호출 금지 → GC 경유
- AttributeSet 어트리뷰트 8개 초과 시 분리

### 1-2. 필수 패턴

- 비주얼 / 오디오 효과 → **GameplayCue**
- **데미지 계산 = `SetByCaller` + 메타어트리뷰트 게이트웨이** (2026-08-12 명문화 — 실제 구현이 이쪽인데 룰이 반대로 적혀 있었다)
  - 모든 데미지는 `UAS_Combat::IncomingDamage` 한 곳으로 들어와 `PostGameplayEffectExecute`에서 분기한다(전방판정·퍼펙트패링·일반패링·피격). **새 데미지 로직은 거기 붙인다**
  - ExecCalc는 현재 **0개.** 계산이 여러 어트리뷰트를 곱하고 나누기 시작하면 그때 도입 검토
  - ⚠️ `PostGameplayEffectExecute` **사후 로직 금지** — Health 0이면 `HandleDeath`가 동기 완료되므로 그 뒤에 코드 추가 X
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

- **Component → Pawn** — 엔진 기반 클래스(`APawn`/`ACharacter`) 캐스팅은 ✅ 허용(컨트롤러·시점 조회에 불가피). **구체 프로젝트 Pawn 캐스팅(`Cast<AKDPlayerCharacter>`)은 ❌ 금지** — 컴포넌트가 특정 캐릭터 전용으로 굳는다
- **Component → Component** — 엔진 컴포넌트(`UCapsuleComponent`·`USkeletalMeshComponent` 등) 조회는 ✅ 허용. 엔진의 `FGameplayAbilityActorInfo::InitFromActor`가 쓰는 패턴이다. **우리 컴포넌트끼리의 직접 참조·상태 변경은 ❌ 금지** — 메시지/델리게이트
- **GA → Component — 읽기 허용 / 구체 Pawn 캐스팅 금지** (2026-08-12 개정. 엔진 소스 실측으로 이전 룰을 뒤집음. 근거 = 볼트 `notes/언리얼/GAS_어빌리티_컴포넌트접근_룰근거.md`)
  - ✅ **읽기(조회) 허용.** `FGameplayAbilityActorInfo`가 `SkeletalMeshComponent`·`AnimInstance`·`MovementComponent`를 `BlueprintReadOnly`로 이미 넘겨준다(`GameplayAbilityTypes.h:138`). `UGameplayAbility`에도 `GetOwningComponentFromActorInfo()`가 있다(`GameplayAbility.h:180`)
  - ✅ 그 외 컴포넌트는 **`AvatarActor->FindComponentByClass<T>()`** — 엔진의 `InitFromActor` 자신이 이 방식을 쓴다(`GameplayAbilityTypes.cpp:23`)
  - ❌ **구체 Pawn 클래스 캐스팅 금지** (`Cast<AKDPlayerCharacter>`). 이게 진짜 결합이다 — GA가 그 Pawn 전용으로 굳고 다른 Pawn에선 **에러도 로그도 없이 조용히 죽는다.** Epic 예제도 캐스팅은 하되 엔진 기반 클래스(`ACharacter`)까지만
  - ❌ **컴포넌트 상태 변경(쓰기) 금지** — GameplayEvent / GC 경유
  - **접근자는 `UGA_ActionBase` 한 곳에 모은다.** 이름은 Epic 규약 `Get○○ComponentFromActorInfo`
  - ⚠️ `AvatarActor`는 **null이거나 기대한 타입이 아닐 수 있다** — ActorInfo는 할당(`OnRegister`)과 채움(`InitAbilityActorInfo`)이 2단계라 그 사이 구간이 정상 존재한다. 접근자에서 항상 null 체크
- AS → 다른 시스템 (데이터만)

### 1-4. 슈퍼 싱글톤 방지 의식

- 매 코딩 세션 끝 5분 리뷰: 줄 수 + 의존성 방향 확인
- "어디 둘지 모르면 Pawn에" 금지 → 컴포넌트 후보 먼저 검토
- 주 1회 SOLID / God Class 점검
- 빠른 답이 나오면 의심: "결합도 높은 것 아닌가"
- AI 코드의 단위 테스트 가능성 질문 → 못 한다면 리팩토링 신호

### 1-5. 같은 함수 두 번째 버그 = 설계 의심 (2026-08-03 신설)

한 함수에서 버그가 두 번 나오면 세 번째 증상을 고치지 말고 **그 함수의 입력과 소유자를 의심한다.**

- **입력** — 이 계산이 남의 *출력*을 읽고 있지 않은가. 부모가 입힌 효과가 계산에 섞여 든다
- **소유자** — 이 상태를 들고 있어야 할 주체가 정말 여기인가

실제 비용: 카메라 `UpdateLookRotation` 3세션(짐벌락→어깨오프셋→랙스윙, 뿌리 하나) / 히트스톱 `ApplyHitStop` 2회(`SetPlayRate(0)`→`Pause`, 그리고 재진입). 둘 다 증상을 따라가다 뿌리를 늦게 봤다.

---

## 2. GAS 컨벤션 함정

새 코드 짤 때 매번 확인:

1. **`ATTRIBUTE_ACCESSORS` 매크로** — 엔진 미제공. 새 `UAS_*` 헤더마다 `#define` 직접 추가
2. **`PreAttributeChange` 클램프** (예: Health 0~MaxHealth) / **`PostGameplayEffectExecute` 후처리**
3. **GAS 모듈 의존성 3개** — `GameplayAbilities`, `GameplayTags`, `GameplayTasks` 셋 다 `Build.cs PublicDependencyModuleNames`에 추가
4. **Tags 중앙 선언** — `NativeGameplayTags.h` 1개 파일. BP 등록 금지. `UGameplayTagsSettings`는 `DefaultGameplayTags.ini`만 유효
5. **AT 깊이 2단계 이하** — AbilityTask 안에서 또 AT 호출 시 Call Stack 안 잡힘
6. **GameplayCue 5~10개만** — 모든 비주얼 GC 금지 (디버깅 끔찍). 나머지는 BP / AnimNotify
7. **GE 자식 CDO `AddComponent<>()` 금지** — UE5.6 fatal. `CreateDefaultSubobject` + `GEComponents.Add` 패턴 필수
8. **ASC 초기화 = `InitAbilityActorInfo`** — Player: `PossessedBy`에서 `(PS, this)` / Enemy: `(this, this)` (현행 코드 패턴 유지). 멀티 전환 시 클라 쪽 `OnRep_PlayerState`에서도 호출 필수 (현재 싱글이라 미구현 — 의도적)

> **Replication (멀티 대응)** — 멀티 확장이 결정된 시점부터 `UPROPERTY(Replicated)` / `OnRep_*` / `GetLifetimeReplicatedProps` / `GAMEPLAYATTRIBUTE_REPNOTIFY` 적용. 싱글 프로토타입 단계에서는 생략 (결정 시 본 파일에 기록).

---

## 3. 프로토타이핑 룰

- **YAGNI**: 2회 이상 사용되는 코드만 함수 분리. 1회용은 인라인
- **데이터 승격 기준** (2026-08-03 개정): 값이 1곳에서만 쓰이면 코드 상수, **에셋마다 달라지거나 3곳 이상에서 갈리면 DataAsset**. 초기 프로토타입 룰("DA 셋업은 후순위")은 종료 — 현재 DA 7개(`ComboTree` `HitConfirmProfile` `LockOnConfig` `WeaponDataAsset` `ExecutionProfile` `PlayerExecutionProfile` `EnemyDefinitionDataAsset`) + 커브 3개가 정상 운영 중
- **Editor 튜닝 우선**: 수치 변경 빈도 높은 값은 `UPROPERTY(EditAnywhere)` 노출 → BP에서 즉시 변경
- **ASC 모드 일관성**: 1택 고정 후 도중 변경 X — 현행: Player = PlayerState / Enemy = Pawn 직결 (변경 필요 시 본 파일에 기록 후)
- **액션 우선순위 (참고)**: 입력 반응(1) > 타격 표시(2) > 시각 폴리싱(3)

→ 본 룰셋과 §1 슈퍼 싱글톤 방지 룰은 **공존**. 단순함과 분리는 다른 차원.

---

## 4. 검증 게이트

- 각 마일스톤 종료 시 해당 스펙의 §검증 항목 모두 ✓
- 마일스톤 성공 기준 미달 시 다음 마일스톤 진입 금지

---

## 5. 솔로 운영 룰 ★ (2026-06-22 솔로 전환, 2026-07-12 개정)

### 5-1. 버전 관리 (Git 이중 구조)

- **코드 repo** (`Project_KD\.git`): Source/Config/docs만 추적 → GitHub 푸시
- **에셋 repo** (`Content\.git`): 로컬 전용 세이브 포인트. **원격 푸시 금지** (13GB)
- **커밋 단위**: 1기능 / 1커밋. 메시지 = `[태그] 한 줄 요약` + 본문(왜)
  - 태그: `[Combat] [GAS] [Input] [Camera] [Anim] [BP] [fix] [refactor] [doc] [chore]`
- **Content 커밋 시점**: MCP/AI 에셋 작업 전(의무) + 에셋 대량 변경 후 + 마일스톤 종료 시
- 비밀(토큰·API key) 커밋 금지 — `.mcp.json`은 gitignore 유지

### 5-2. 리뷰 게이트 (페어 리뷰 대체) ★

**대상**: 아키텍처 변경 / 새 Component / 새 GA / 새 `.h` / 200줄+ 변경

1. **AI 코드 설계·가이드 -> 작성자 입력** (§0 프로토콜)
2. **AI 설계 브리핑** — 클래스 목록+책임 / 의존성 방향 / 줄 수(§1 한도 대비)
3. **본인 리뷰** — 설계가 의도에 맞는지 판단 (승인 / 수정 지시)
4. **code-reviewer agent 검수** — §1·§2 위반, 잠재 버그 기계 검출
5. **본인 최종 판단** (지적 수용/기각) → 커밋

- 자잘한 수정(수치·오타·1회용): 2~3 생략, 커밋 전 diff 셀프 리뷰만
- 커밋 전 `git diff` 셀프 리뷰는 항상 필수

### 5-3. Claude 자산

- `.claude/` = 로컬 개인 (커밋 X) — skills·hooks·settings 전부
- 개인 비밀 / `~/.claude/` = 커밋 절대 금지

### 5-4. 블로커

- **30분 막힘 룰**: 30분 막히면 AI에게 ping (debugger agent / 웹 조사)
- **단독 결정 기록**: 아키텍처·스펙 결정은 본 CLAUDE.md 또는 스펙에 1줄 기록 (미래의 나 = 팀원)

---

## 6. 참조 자료

- 본 `CLAUDE.md` — 프로젝트 룰 단일 진실
- `docs/specs/<현재>-spec.md` — 현재 마일스톤 답안지 (검증 게이트) ★
- `docs/design/` — 기획 문서 (GDD, 메커닉, 시스템 설계)
- `docs/reference/` — 외부 자료 (구 Perforce 자료 포함 — 참고용)
- `docs/dev-logs/` — 기능별 개발 기록
- `~/.claude/CLAUDE.md` — UE5 + Karpathy 전역 (개인 환경)

---

## 7. 작업 라우팅 (OMC agent + skill + MCP)

슬래시 커맨드는 두지 않음 — OMC agent와 중복(Karpathy §2 YAGNI). 도메인 지식(§1~3, §8~9)은 본 CLAUDE.md에 박혀 있어 agent 호출 시 자동 주입.

### OMC agent 위임 (컨텍스트 격리 + 모델 선택)

| 작업        | 위임 대상                               | 비고                                                            |
| ----------- | --------------------------------------- | --------------------------------------------------------------- |
| 설계 검토   | `architect` agent (READ-ONLY)           | 구현 전 대안 비교                                               |
| `.cpp` 설계 | `executor` agent (`model=opus` 복잡 시) | **코드블록 반환만. 파일 편집 금지** — 작성자 `.h` 시그니처 준수 |
| 결함 검출   | `code-reviewer` agent                   | §5-2 리뷰 게이트 4단계 담당                                     |
| 에러 진단   | `debugger` agent                        | 빌드 / 런타임 근본 원인                                         |
| 구조 개선   | `code-simplifier` agent                 | 동작 변경 X                                                     |
| 검증        | `verifier` agent                        | 변경이 실제로 작동하는지                                        |

### 프로젝트 skill (`.claude/skills/`)

| skill             | 시점                            | 역할                                                                   |
| ----------------- | ------------------------------- | ---------------------------------------------------------------------- |
| `ue-build-check`  | `.h`/`.cpp` 수정 직후           | UE5.6 Build.bat 자동 호출, 에러/경고 분리 보고                         |
| `explain-plainly` | 코드·엔진 동작 설명 시 **항상** | 추상 표현·지어낸 이름·미검증 단정 금지. 유저 전역(`~/.claude/skills/`) |

### Unreal MCP (에디터 자동화 — McpAutomationBridge)

- 용도: §0의 제한적 허용 범위(테스트맵·노티파이·일괄작업·조회) 내에서만
- 연결: 에디터 실행 + `● MCP :3000` 확인 → `.mcp.json` 자동 인식
- 안전: 토큰 인증 켜짐 / loopback 전용 / **작업 전 Content 커밋**

> `.h` / `.cpp` = 작성자 입력 영역. **agent도 코드 파일 수정 금지** — 설계·코드블록 반환까지만 (§0)

---

## 8. 애니메이션 룰 ★ (2026-07-12 신설)

### 8-1. 역할 분담

- **이동/로코모션 = 모션매칭(MM)** — ABP의 PoseSearch 노드 + PSD. 이동 애니를 스테이트머신으로 짜지 않음
- **전투/액션 = 몽타주(AM\_)** — 공격·회피·패링·리액션. **MM 검색 대상(PSD)에 전투 애니 넣지 않기** (DB 오염 = 이동 중 공격모션 튀어나옴)
- **PSD 분리 원칙**: 용도별 DB (예: Idles / Stops / Loops) — 하나의 거대 DB 금지

### 8-2. 히트 판정 = AnimNotify가 단일 진실

- 공격 판정 프레임 = 몽타주의 **AnimNotify(State)** 로만 정의 (코드에 하드코딩 금지)
- 노티파이 → GameplayEvent(태그) → GA가 수신 (§1 의존성 방향 준수: 애니가 GA를 직접 호출하지 않음)
- 판정 타이밍 수정 = 몽타주에서만 (코드 재컴파일 없이 튜닝)

### 8-3. ABP 구조

- **상하체 분리 = Layered blend per bone** — 하체 MM 이동 + 상체 몽타주 (이동 중 공격)
- 슬롯 표준: `DefaultSlot`(전신) / `UpperBody`(상체) 2개만 — 슬롯 난립 금지
- 루트모션: **전투 몽타주 = 루트모션 사용** (공격 전진·회피 이동) / 일반 이동 = CMC. 변경 시 본 파일에 기록

### 8-4. 리타게팅 파이프라인

- 외부 애니(Mixamo 등) → SKEL*UE5_F 리타겟 → 결과물만 프로젝트 네이밍(`AS*버터\_동작`) 적용
- 원본 소스 애니는 별도 폴더 보존 (재리타겟 대비)
- 리타겟 후 필수 확인: 루트 위치 / 발 슬라이딩 / 손목 뒤틀림

---

## 9. 네이밍 컨벤션 (실사용 기준 명문화, 2026-07-12 신설)

**원칙**: 우리가 만드는 에셋만 적용. 외부 팩(GhostSamurai 등) 원본은 리네임하지 않음 (레퍼런스 깨짐).

### 코어 / BP

| 접두사      | 대상                    | 예                 |
| ----------- | ----------------------- | ------------------ |
| `BP_`       | Blueprint 액터/컴포넌트 | BP_Butter          |
| `WBP_`      | 위젯 BP (UI)            | WBP_HealthBar      |
| `BPI_`      | BP 인터페이스           | BPI_Interactable   |
| `BPFL_`     | 함수 라이브러리         | BPFL_CombatHelpers |
| `E_` / `F_` | 열거형 / 구조체         | E_WeaponType       |

### GAS

| 접두사 | 대상                      | 예                                    |
| ------ | ------------------------- | ------------------------------------- |
| `GA_`  | GameplayAbility           | GA_Dash                               |
| `GE_`  | GameplayEffect            | GE_Damage_Base                        |
| `GC_`  | GameplayCue Notify        | GC_HitSpark                           |
| `UAS_` | AttributeSet (C++ 클래스) | UAS*Health — ※에셋 `AS*`(애니)와 다름 |

### 애니메이션

| 접두사                   | 대상                      | 예                   |
| ------------------------ | ------------------------- | -------------------- |
| `ABP_`                   | Animation Blueprint       | ABP_Butter           |
| `AS_`                    | Anim Sequence             | AS_Butter_Run_Loop_F |
| `AM_`                    | Anim Montage              | AM_Butter_Combo1     |
| `BS_`                    | Blend Space               | BS_Butter_Locomotion |
| `PSD_` / `PSS_`          | 포즈서치 DB / 스키마 (MM) | PSD_Butter_Idles     |
| `SKEL_` / `SK_` / `SKM_` | 스켈레톤 / 스켈레탈메시   | SKEL_UE5_F           |
| `PA_`                    | Physics Asset             | PA_Butter            |
| `CR_`                    | Control Rig               | CR_Butter            |

### 아트 / 이펙트

| 접두사                       | 대상                                | 예               |
| ---------------------------- | ----------------------------------- | ---------------- |
| `SM_`                        | Static Mesh                         | SM_Rock01        |
| `M_` / `MI_` / `MF_` / `ML_` | 머티리얼 / 인스턴스 / 함수 / 레이어 | M_Butter_Body    |
| `T_`                         | 텍스처 (+접미사 `_D`/`_N`/`_R`)     | T_Butter_Body_D  |
| `NS_`                        | Niagara System                      | NS_HitSpark      |
| `VFX_`                       | (팩 관례) 이펙트 그룹               | 신규는 NS\_ 사용 |

### 사운드 / 데이터 / AI / 입력 / 레벨

| 접두사                 | 대상                           | 예               |
| ---------------------- | ------------------------------ | ---------------- |
| `SFX_` / `SC_` / `SW_` | 사운드 이펙트 / 큐 / 웨이브    | SFX_Slash        |
| `DA_`                  | Data Asset                     | DA_EnemyDef_Komi |
| `DT_` / `CT_`          | Data Table / Curve Table       | DT_DropTable     |
| `BT_` / `BB_`          | Behavior Tree / Blackboard     | BT_Komi          |
| `AIC_`                 | AI Controller                  | AIC_EnemyBase    |
| `EQS_`                 | EQS 쿼리                       | EQS_FlankPos     |
| `IA_` / `IMC_`         | Input Action / Mapping Context | IA_Dash          |
| `L_`                   | 레벨(맵)                       | L_TestArena      |
