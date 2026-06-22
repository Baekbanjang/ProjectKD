# Project_KD

> UE5.6 + GAS 기반 액션 프로토타입. 솔로 + Claude Code(OMC) + Git.

---

## 빠른 시작

### 1) 사전 요구사항
- **Unreal Engine 5.6** (`C:\Program Files\Epic Games\UE_5.6`)
- **Visual Studio 2022** (Game development with C++ 워크로드)
- **Git** (+ 선택: GitHub 계정)
- **Claude Code + oh-my-claudecode** (선택, 권장)

### 2) Git 클론 / 세팅
```cmd
git clone https://github.com/Baekbanjang/ProjectBT.git Project_KD
cd Project_KD
```
이미 로컬에 있으면 최신화만:
```cmd
git pull
```

> ⚠️ **자산(`Content/`)은 Git에 없음.** 소스코드만 추적하므로 `.uasset`/`.umap`(블루프린트 포함)·마켓 자산은 클론에 안 들어옴. 자산은 로컬 폴더로 따로 백업·복원해야 함.

### 3) 빌드
1. `git pull`로 소스 최신화
2. `Project_KD.uproject` 우클릭 → **Generate Visual Studio project files**
3. `Project_KD.sln` 열고 **Development Editor / Win64** 빌드 (F5)
4. UE Editor가 자동으로 열림

### 4) Claude Code (선택)
프로젝트 루트에서 `claude` 실행. 본 레포의 `CLAUDE.md`가 자동 로드됨.

---

## 디렉토리 구조

```
Project_KD/
├── CLAUDE.md                    ★ 작업 룰 단일 진실
├── README.md                    본 파일
├── Project_KD.uproject          UE 프로젝트 파일
├── Config/                      DefaultEngine.ini 등
├── Source/Project_KD/           C++ 코드
├── docs/                        ★ 기획/스펙 문서
│   ├── specs/                   현재 마일스톤 답안지 (M1, M2, ...) — 검증 게이트
│   ├── design/                  기획 문서 (GDD, 메커닉, 시스템 설계)
│   └── reference/               참고 자료
├── .claude/skills/              팀 공유 skill (단, .claude/는 .gitignore로 추적 제외)
│   └── ue-build-check/          UE5 빌드 자동 검증
├── .gitignore                   Git ignore 패턴
├── .p4ignore                    (구 Perforce 잔재, 미사용)
└── (ignored) Content/ Binaries/ Saved/ Intermediate/ DerivedDataCache/ .vs/ .omc/ .idea/
```

> `Content/`(자산·블루프린트 전부)는 `.gitignore`로 **추적 제외** → 소스코드만 Git, 자산은 로컬 백업.

---

## 솔로 워크플로우 (요약)

상세는 **`CLAUDE.md`** — 본 README는 환경 셋업만 다룸.

### 커밋 3단계
```cmd
git add <변경파일>
git commit -m "[태그] 한 줄 요약"
git push
```
- 커밋 태그 예: `[Combat] [GAS] [Input] [Camera] [fix] [refactor] [doc] [chore]`
- 메시지: `[태그] 한 줄 요약` + 본문(왜).

### 코드 작성 프로토콜
- **`.h` = 사람**(의도 설계), `.cpp` = AI draft → 30초 review
- 자산/BP/Editor 작업 = 사람

### 작업 라우팅
| 작업 | 도구 |
|---|---|
| `.h` 설계 | 사람 |
| `.cpp` 구현 | Claude (`executor` agent) |
| 빌드 검증 | `ue-build-check` skill (자동) |
| 결함 검출 | `code-reviewer` agent |
| 에러 진단 | `debugger` agent |
| 구조 개선 | `code-simplifier` agent |

---

## 트러블슈팅

| 증상 | 원인 / 해결 |
|---|---|
| 클론했는데 자산이 없음 | 정상. `Content/`는 Git 추적 제외 — 로컬 백업에서 복원 |
| `Generate VS project files` 실패 | UE 5.6 설치 확인, `.uproject` 우클릭 |
| 빌드 시 `LNK2019` (GameplayAbilities) | `Build.cs`의 `PublicDependencyModuleNames`에 `GameplayAbilities`, `GameplayTags`, `GameplayTasks` 셋 다 |
| Editor 켜져있을 때 빌드 hang | `-WaitMutex` 정상 동작. Editor 닫으면 즉시 진행 |

---

## 참조

- **`CLAUDE.md`** — 작업 룰 / GAS 함정 / 아키텍처 가드레일 (단일 진실)
- **`docs/specs/<마일스톤>-spec.md`** — 현재 마일스톤 답안지 (검증 게이트)
- **`docs/design/`** — GDD, 메커닉, 시스템 설계
- **`~/.claude/CLAUDE.md`** — UE5 + Karpathy 전역 룰 (개인 환경)
