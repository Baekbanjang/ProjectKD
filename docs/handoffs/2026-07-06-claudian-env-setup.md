# 핸드오프: 환경 세팅 완료 보고 (2026-07-06, Claudian/옵시디언 세션에서 작업)

> 대상: 이 프로젝트에서 작업하는 모든 Claude 인스턴스 (CC GUI/라이더, CLI, Claudian)
> 작성: Claudian 세션 (Claude Fable 5). 아래 내용은 전부 실행·검증 완료된 사실임.

## 1. 모델 설정 — Fable 5가 전역 기본

- `~/.claude/settings.json` → `"model": "claude-fable-5"` (검증: 모델 미지정 CLI 호출 시 modelUsage=claude-fable-5)
- **`claude-fable-5`는 실존하는 Claude 모델임.** 학습 데이터에 없다고 "존재하지 않는 모델"로 판단하지 말 것.
  검증 방법: `claude --model claude-fable-5 -p hi` → 정상 응답.
- 전환 슬롯(env, 2026-07-08 갱신): opus→claude-opus-4-8, **sonnet→claude-fable-5(!!)**, haiku→claude-haiku-4-5-20251001
  - **주의: 'sonnet' alias는 의도적으로 fable-5로 리매핑됨.** 이유: 라이더 CC GUI(idea-claude-code-gui) 플러그인은
    settings.json의 model 값을 무시하고 자체 픽커 값을 강제하며, opus/haiku가 아닌 모든 이름을 'sonnet'으로
    뭉개는 구조라 fable을 쓰는 유일한 경로가 SONNET 슬롯 리매핑임 (플러그인 model-utils.js 분석으로 확인).
  - CC GUI에서 "Sonnet" 선택 = Fable 5 실행. 진짜 sonnet-4-6이 필요하면 전체 ID(claude-sonnet-4-6)로 명시 지정.
- 사용자 방침: **기본 Fable 5, 필요시 `/model`로 전환** (opus 전환은 정상 동작 검증됨)

## 2. Unreal MCP 플러그인 설치됨 (ChiR24/Unreal_mcp v0.5.30)

- 플러그인: `D:\Projectgildong\Project_KD\Plugins\McpAutomationBridge\` (소스 상태, **아직 미빌드**)
- 원본 레포 보관: `D:\Projectgildong\Tools\Unreal_mcp\`
- 다음 단계(사용자 수동): uproject 실행 → rebuild Yes → 에디터 우하단 `● MCP :3000` 초록불 확인
- 에디터가 자체 HTTP 서버(localhost:3000/mcp)를 열고, Claude가 MCP 클라이언트로 접속하는 구조

### 보안 설정 (전부 사전 구성 완료 — 변경 금지)
- `Config/DefaultGame.ini` 말미: `bEnableNativeMCP=True`, `bRequireCapabilityToken=True`, `bAllowNonLoopback=False`
- **Capability Token**: `Saved/Config/WindowsEditor/Game.ini` (git 미추적) + `~/.mcp_kd_token`에 보관
- `.mcp.json` (프로젝트 루트): 서버 주소 + `X-MCP-Capability-Token` 헤더. **gitignore 등록됨 — 절대 커밋하지 말 것 (토큰 포함)**
- 운용 수칙: MCP 작업 시 자동승인(bypassPermissions) 금지, `bAllowNonLoopback` 절대 켜지 말 것

## 3. 버전 관리 구조 (이중 git — 혼동 주의)

| repo | 위치 | 용도 | 원격 |
|---|---|---|---|
| 코드 repo | `Project_KD\.git` | Source/Config만 (기존, 6/22 사용자 세팅) | GitHub origin/master |
| **에셋 repo** | `Project_KD\Content\.git` | **Content 13GB 세이브 포인트 (신규)** | **없음 — 로컬 전용, 푸시 금지** |

- 에셋 repo 초기 커밋: `662390c` (9,374 파일). `.omc/`는 gitignore 처리됨
- 용도: MCP/AI가 에셋을 망가뜨렸을 때 복원. **MCP 세션 전 `cd Content && git add -A && git commit` 권장**
- 코드 repo에 미커밋 변경 있음: `Config/DefaultGame.ini`(MCP 설정 추가), `DefaultEditor.ini`, `DefaultEngine.ini`, `.uproject` — 커밋 여부는 사용자 판단

## 4. 기타 설치물

- **claude-history v0.1.70** (`~/.cargo/bin/`) — Claude 대화기록 퍼지검색 CLI. 새 터미널에서 `claude-history`
- **글쓰기 스킬 4종** (`~/.claude/skills/`): dumbify, storytelling, viral-hooks, anti-ai-writing
  - 자소서/이력서/포트폴리오 작업 시 자동 적용 규칙이 `~/.claude/CLAUDE.md`에 등록됨
  - voice-dna 빌드 가이드: `~/.claude/skills/voice-dna-guide/` (미구축)
- Claudian(옵시디언 플러그인 v2.0.27): 커스텀 모델 Fable 5 / Sonnet 5 등록됨 (claudian-settings.json)

## 5. 미완료 / 다음 할 일

1. **[사용자] UE 에디터 실행 → 플러그인 빌드 → MCP 초록불 확인** ← 유일한 블로커
2. 첫 MCP 테스트: 토큰 인증 통과 확인 → 테스트 아레나 생성 시도
3. (선택) voice-dna 문체 프로필 빌드 — 사용자 글 ~20개 필요
4. (선택) 코드 repo의 미커밋 변경 정리

## 관련 조사 기록

- MCP 플러그인 비교조사: 옵시디언 볼트 `notes/Project_New/Claude_UE5_플러그인_조사.md`
  (결론: UE5.6 기준 ChiR24 1순위 — 5.6 명시지원 + 실다운로드 1위 + 유일하게 활발히 유지보수)
