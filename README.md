# Project_KD

> UE5.6 + GAS 기반 액션 프로토타입. 2인 페어 + Claude Code(OMC) + Perforce.

---

## 빠른 시작

### 1) 사전 요구사항
- **Unreal Engine 5.6** (`C:\Program Files\Epic Games\UE_5.6`)
- **Visual Studio 2022** (Game development with C++ 워크로드)
- **Perforce P4V + p4 CLI** (서버 접근 권한 페어에게 요청)
- **Claude Code + oh-my-claudecode** (선택, 페어 작업 시 권장)

### 2) Perforce 워크스페이스
```
Workspace 이름:  <이름>_<PC명>   (예: kim_DESKTOP-ABC)
Stream:          //Project_KD/main
Root:            F:\Projects\Project_KD   (예시, 본인 디스크에 맞춤)
```

워크스페이스 생성 후 **반드시**:
```cmd
p4 set P4IGNORE=.p4ignore
```
이 안 하면 `.p4ignore`가 적용 안 됨 → `Binaries/`, `.omc/`, `.vs/` 등이 전부 submit 대상으로 잡힘.

### 3) typemap (Perforce admin이 1회만)
UE 바이너리 에셋에 exclusive lock 걸어야 페어가 동시에 같은 `.uasset` 못 건드림.
```
p4 typemap
```
편집 후 아래 추가:
```
binary+l    //....uasset
binary+l    //....umap
binary+l    //....uexp
binary+l    //....ubulk
binary+l    //....fbx
```

### 4) 빌드
1. P4V로 `//Project_KD/main` 동기화
2. `Project_KD.uproject` 우클릭 → **Generate Visual Studio project files**
3. `Project_KD.sln` 열고 **Development Editor / Win64** 빌드 (F5)
4. UE Editor가 자동으로 열림

### 5) Claude Code (선택)
프로젝트 루트에서 `claude` 실행. 본 레포의 `CLAUDE.md`가 자동 로드됨.

---

## 디렉토리 구조

```
Project_KD/
├── CLAUDE.md                    ★ 페어 룰 단일 진실 (P4)
├── README.md                    본 파일 (P4)
├── .p4ignore                    P4 ignore 패턴 (P4)
├── Project_KD.uproject          UE 프로젝트 파일 (P4)
├── Config/                      DefaultEngine.ini 등 (P4)
├── Content/                     에셋 (P4)
├── Source/Project_KD/           C++ 코드 (P4)
├── docs/                        ★ 페어 작성 (P4)
│   ├── specs/                   현재 마일스톤 답안지 (M1, M2, ...) — 검증 게이트
│   ├── design/                  기획 문서 (GDD, 메커닉, 시스템 설계)
│   └── reference/               Perforce 가이드 등 외부 자료
├── .claude/skills/              팀 공유 skill (P4)
│   └── ue-build-check/          UE5 빌드 자동 검증
└── (ignored) Binaries/ Saved/ Intermediate/ DerivedDataCache/ .vs/ .omc/ .idea/
```

---

## 페어 워크플로우 (요약)

상세는 **`CLAUDE.md`** — 본 README는 환경 셋업만 다룸.

- **`.h` = 사람**, `.cpp` = AI draft → 30초 review
- **CL 단위**: 1기능 1CL, 24h 보유 금지
- **퇴근 전**: `Revert if Unchanged` + 잠금 풀기
- **페어 리뷰 트리거**: 아키텍처 변경 / 새 `.h` / 200줄+ CL → 24h SLA
- **막힘**: 30분 막히면 페어 / AI ping

### 작업 라우팅
| 작업 | 도구 |
|---|---|
| `.h` 설계 | 도메인 오너 (사람) |
| `.cpp` 구현 | Claude (`executor` agent) |
| 빌드 검증 | `ue-build-check` skill (자동) |
| 결함 검출 | `code-reviewer` agent |
| 에러 진단 | `debugger` agent |
| 구조 개선 | `code-simplifier` agent |

---

## 트러블슈팅

| 증상 | 원인 / 해결 |
|---|---|
| `Binaries/`가 submit 후보로 잡힘 | `p4 set P4IGNORE=.p4ignore` 안 했음 |
| 같은 `.uasset` 페어 동시 편집 후 머지 손실 | typemap에 `binary+l` 누락 |
| `Generate VS project files` 실패 | UE 5.6 설치 확인, `.uproject` 우클릭 |
| 빌드 시 `LNK2019` (GameplayAbilities) | `Build.cs`의 `PublicDependencyModuleNames`에 `GameplayAbilities`, `GameplayTags`, `GameplayTasks` 셋 다 |
| Editor 켜져있을 때 빌드 hang | `-WaitMutex` 정상 동작. Editor 닫으면 즉시 진행 |

---

## 참조

- **`CLAUDE.md`** — 페어 룰 / GAS 함정 / 아키텍처 가드레일 (단일 진실)
- **`docs/specs/<마일스톤>-spec.md`** — 현재 마일스톤 답안지 (검증 게이트)
- **`docs/reference/`** — Perforce 가이드, typemap 원본
- **`~/.claude/CLAUDE.md`** — UE5 + Karpathy 전역 룰 (각자 개인 환경, P4 X)
