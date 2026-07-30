# 핸드오프 — 2026-07-30 · 2세션 병행 체제

> **이 문서를 읽는 세션에게**: 아래 §0에서 **네가 어느 레인인지** 먼저 확인하라.
> 레인을 어기면 두 세션이 같은 파일을 다르게 고쳐 커밋이 충돌한다.

---

## 0. 레인 구분 (충돌 방지 규칙) ★

| | **A레인 — 기획·설계** | **B레인 — 코드·에셋** |
|---|---|---|
| 하는 일 | 기획 검토, 설계안, 스펙 patch, 문서 정리, 자료조사 | `.h`/`.cpp` 수정, BP·에셋 작업, 빌드, PIE 검증 |
| 쓰기 허용 | `docs/**` · 옵시디언 볼트 | `Source/**` · `Config/**` · `Content/**` |
| **금지** | `Source/` `Config/` `Content/` 수정 | 기획 문서 재작성(스펙 patch는 A가) |
| Git 커밋 | `docs/` 만 | `Source/` `Config/` + Content repo |
| **UE 에디터 / MCP** | ❌ **쓰지 않는다** | ✅ B만 사용 |
| 메모리 쓰기 | ❌ 읽기만 | ✅ B만 기록 |

**왜 이렇게 나누나**
- **UE 에디터는 하나뿐이다.** 두 세션이 MCP로 동시에 에셋을 쓰면 어느 쪽 변경이 남는지 알 수 없다
- **메모리 `MEMORY.md`는 두 세션이 공유한다.** 동시에 쓰면 나중 쓰기가 앞 줄을 덮어 사라진다
- 빌드도 하나뿐이다. A가 코드를 고치면 B의 빌드가 이유 없이 깨진다

**A가 코드를 고쳐야 할 결론에 도달했을 때**: 고치지 말고 `docs/` 안에 "제안" 형태로 남기고 승환에게 알린다. B가 받아서 구현한다.

---

## 1. 지금 어디까지 왔나 (2026-07-30 종료 시점)

### 프로젝트 상태
- **Gun & Sword 피벗 라인** (2026-07-21). SB(스텔라 블레이드) 스타일 지상 액션
- 캐릭터 = 마네퀸(`SK_Mannequin_GunSword`), 이동 = 모션매칭 + Chooser, 전투 = 몽타주
- 코드 규모: `.h` 86개 ≈ 4,000줄 / `.cpp` 83개 ≈ 6,900줄
- **`docs/archive/` 는 읽지 않는다** — 버터/길동 폐기 라인

### 오늘 완료된 것

| 항목 | 결과 |
|---|---|
**`State.Combat.InAction` 우산 태그** | 코드 + BP 7개 + 빌드 + PIE 이상 없음 ✅ |
**회피 캔슬 목록 확대** | Light만 → 공격 4개. 세 곳의 서로 다른 목록을 헬퍼 하나로 통합 ✅ |
**ABP_SB 슬롯 재배치** | `Slot 'DefaultSlot'`을 Locomotion SM 뒤로 + 중복 노드 제거 → 공중 몽타주 출력됨 ✅ |
**`AM_SB_Combo_Air_01` 노티 복원** | `WeaponAttach@0f` + `CancelWindow 28~55f`. PIE 로그로 4타 연결 확인 ✅ |
**퍼펙트 패링 큐 발동 노드** | `GA_PerfectParryReaction`에 `Execute GameplayCue On Owner` 추가 ✅ |
**코드 설명서 8문서** | 옵시디언 볼트 `notes/코드구조/` 2,776줄 신설 ✅ |

### 커밋

```
소스 repo (GitHub 푸시됨)
  6cc9340 [doc]    INDEX에 코드 설명서 포인터
  474bd52 [Combat] 회피 캔슬 목록을 공격 4개로 확대
  4a44cfd [GAS]    State.Combat.InAction 우산 태그

Content repo (로컬 전용, 푸시 금지)
  72017a2 [Anim] AM_SB_Combo_Air_01 노티 + 스프린트/반격 GA에 Attacking
  33b9957 [BP]   InAction 태그 7개 + 퍼펙트패링 큐 + ABP DefaultSlot 이동
```

---

## 2. 즉시 다음 (B레인) ★ 최우선

### ✅ ① GameplayCue 스캔 경로 — 적용 완료 (소스 `a54a6a0`, PIE "잘됨")

아래는 원인 기록. 같은 종류가 또 나오면 `GameplayCue.PrintGameplayCueNotifyMap`부터.

**이펙트가 안 나오는 진짜 원인.** 에셋은 다 있는데 큐 매니저가 그 폴더를 안 뒤진다.

```
Config/DefaultGame.ini:17-19  현재
  /Game/Blueprints/AbilitySystem/GameplayCueNotify   ✅ 존재 (여기 3개만 살아 있다)
  /Game/Blueprints/Player/GC                          ❌ 폴더 없음 (길동 잔재)
  /Game/TrickalFanGame/GAS/GC                         ❌ 폴더 없음 (버터 잔재)

우리 큐 6개 실제 위치 = /Game/SB_Style_GameProject/GAS/GC/   ← 목록에 없다
```

`GameplayCue.PrintGameplayCueNotifyMap` 실측 결과 — **연결 3개 / 미연결 9개**:

| 상태 | 태그 |
|---|---|
✅ 연결 | `Enemy.Telegraph`(+3리프), `Combat.ParryClash`, `Combat.HitImpact.Light` |
❌ unmapped | `Combat.PerfectParry.SlowMo`, `Combat.PerfectDodge`, `Combat.PlayerHitConfirm`, `Combat.CounterThrust`, `Combat.HitReact`, `Combat.Staggered`, `Combat.Execution`, `Camera.Execution`, `Camera.DashTrail` |

**고칠 것**: `DefaultGame.ini`에 `+GameplayCueNotifyPaths=/Game/SB_Style_GameProject/GAS/GC` 추가, 없는 폴더 2줄 삭제. 옛 폴더 줄은 살아 있는 큐 3개가 아직 거기 있으니 **남긴다**.

**에디터 재시작 필요** (이 목록은 `AbilitySystemGlobals` 초기화 때 1회만 읽는다).
검증 = 재시작 후 같은 콘솔 명령 → `unmapped`가 숫자로 바뀌는지.

> 미확정: 각 GCN의 `GameplayCueTag`를 조회하니 6개 다 비어 보였는데 **잘 되는 옛 3개도 똑같이 비어 보였다** → 조회 방식이 부정확. 태그 문제로 단정하지 말 것. 경로만 확정 원인이고, 재시작 후에도 unmapped면 그때 BP 디테일에서 태그를 직접 박는다.

### ✅ ② 입력 버퍼 창 확대 — 적용 완료 (2026-07-30)

`BP_SBPlayer` → `InputBuffer` 컴포넌트 디테일에서 `BufferTimeWindow` 0.2 → **0.5**, `MaxBufferSize` 4 → **2**. 코드 수정 0줄(둘 다 `EditAnywhere`, `ClampMax`가 정확히 0.5).

지상 콤보 15개 중 8개에서 입력이 증발하던 문제 → **12개 해결.**
**남은 3개**: `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74)는 `ANS_CancelWindow`가 너무 늦게 열려 0.5초로도 못 덮는다 → **노티를 앞으로 당길지 별도 판단.**

---

## 3. 보류 목록 (B레인, 우선순위 순)

| # | 항목 | 내용 |
|---|---|---|
1 | **캔슬 윈도우 늦은 몽타주 3개** | `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74). 버퍼 0.5초로도 못 덮는다. `ANS_CancelWindow`를 앞으로 당기는 게 유일한 해법 |
2 | 월드 전체 액터 순회 2곳 | `GA_Dodge.cpp:181` + `EnvQueryContext_AllyEnemies`가 `TActorIterator`로 레벨 전체를 훑는다(거리 검사가 **그 다음**이라 필터가 아니다). 액터 늘면 회피마다 프레임 튄다. `LockOnComponent.cpp:112`의 `OverlapMultiByObjectType` 방식으로 교체 |
3 | `AirComboResetTime` | `ComboResetTime 1.5f` 하나를 지상·공중이 공유. 급하지 않다 — 실제 유예는 **떨어지는 시간**이 정한다. 공중 재설계와 함께 |
4 | `ANS_WeaponTrail` 잔류 위험 | 노티는 인스턴싱 안 되는데 `SpawnedComponent`를 멤버로 들고 있다. 같은 몽타주를 두 액터가 동시 재생하면 트레일 영구 잔류. 재생자 1명이면 안 터짐 |
5 | `OnInActionTagChanged` 재호출 | GA가 겹치면 `NewCount` 1→2로 재호출. 같은 소켓 재부착이라 결과 동일, `GripPoint` 역보정만 한 번 더. 문제 생기면 수정(승환 판단) |

### 닫힌 항목

- ✅ **`PerfectDodgeWindowSec` 삭제** (2026-07-30) — 참조 0건인 죽은 값. 실제 퍼펙트 판정은 시간이 아니라 **상태**(적 ASC의 `State.Combat.EnemyAttackHitWindow` 태그). ⚠️ `PerfectDodgeCheckRadius`(500)는 살아 있다
- ✅ **`Block_End` = A안(연결 안 함)으로 확정** (2026-07-30 승환) — C++·BP 그래프 전수 확인 결과 재생 경로가 없고, 지금 자연스럽게 보이는 건 **ABP 블렌드 아웃(0.25초)**이다. 연결하면 가드를 놓고 0.6초간 묶여 "가드 풀었는데 못 움직인다"는 새 문제가 생긴다. SB도 블렌드로 처리
  - 뒤집을 때 방법 3개: ① 짧은 전용 GA ② `GA_Parry` 종료를 몽타주 끝까지 지연 ③ GA 없이 `AnimInstance`에 직접 재생(가장 단순, 순수 연출이라 판정 없음 — 단 §1-3 의존성 방향 확인)

---

## 4. 설계 미결 (A레인) ★

코드가 아니라 **판단이 필요한** 것들. 여기가 A레인의 일감이다.

### ① 스탠스 체계
SB는 `Default(=Sword) / Tachy / Fusion / Gun계열 / Fishing / 특수(사망·동결·튜토리얼)`로 나뉜다. **평시 스탠스가 없다** — Eve는 항상 무장 상태다.
우리 초안(승환): `Default(평시 전투 = Sword&Gun) / Gun(진짜 사격만) / 사망`.
→ **사격 로직 착수 전까지 보류 결정됨**(YAGNI). 사격을 시작할 때 이 표를 확정해야 한다.

### ② 공중 콤보 재설계
`Air_01`이 **총 클립**이라 1타에 검 판정이 없다(`MeleeTrace` 없음이 의도).
방향: **07 유지 + 14~16 `Attack_Air_to_Floor`를 마무리로 붙여 지상 콤보로 연결**(공중 공격 → 지상 찍기 → 자연스럽게 지상 콤보).
`02_Attack` 폴더는 **전부 루트모션** — InPlace 클립이 없다. "제자리 공중 공격"은 RM을 끄는 게 아니라 **이동량 0인 클립**으로 얻어야 한다.

### ③ 스태미나 폐기
점프·달리기 자원 폐기 결정(스킬 코스트는 유지). SB 644행 실측으로 검증됨(SB의 Stamina = 적 격파 게이지). **미착수.** 소모처 3곳 값 0 + BP Cost GE 함정 주의.

### ④ 락온 애니
락온 상태 전용 애니가 보류 상태. 검 콤보 → 총 순서로 미뤄뒀다.

### ⑤ 미착수 폴리싱
트레일 NS 27개 미배정 / 사운드 노티 없음 / 데미지 GE 26노드 비어 있음 / LoP식 방사형 회피 이펙트.

---

## 5. 세션 시작 시 읽을 것

프로젝트 `CLAUDE.md §0 세션 시작 프로토콜`이 단일 진실이다. 요약:

1. `docs/PROJECT_OVERVIEW.md` — 부트 문서
2. `docs/INDEX.md` — 전체 카탈로그
3. **이 핸드오프** + 이번 작업 영역의 최신 dev-log
4. 코드 작업 세션이면 `.h` 전수 훑기(4,000줄이라 감당 가능), `.cpp`는 닿는 것만
5. **코드 설명서 = 옵시디언 볼트 `ProjectKD/notes/코드구조/`** (8문서 2,776줄) — 클래스별 용도·함수 기능·흐름·핵심 코드 발췌. `00_코드구조_MOC`부터

**에셋 값은 MCP로 조회해 확인할 것** (B레인만). 코드 기본값과 다를 수 있고, 폐기된 라인의 값이 남아 있던 사례가 실제로 있었다 — 2026-07-28 공격 GA 5개가 길동 창 소켓 `Spear_Tip`을 물고 있어 근접 판정이 죽어 있었다.

---

## 6. 오늘 배운 함정 3개 (재발 방지)

1. **`.h` 선언과 `.cpp` 정의는 짝이다.** `InAction`이 `.cpp`만 있어서 빌드가 깨졌다. 태그 추가 시 양쪽 확인
2. **구독과 해제는 같은 태그로.** `RegisterGameplayTagEvent(A).Add()` 하고 `RegisterGameplayTagEvent(B).Remove()` 하면 조용히 실패한다 — 핸들 리셋은 그대로 돌아서 코드가 깨끗해 보인다
3. **`search_assets`는 패턴을 무시하고 100개를 통째로 반환한다.** 목록·집계는 `execute_python`으로. 실측: 821개 중 100개가 돌아와 1만 토큰 낭비
