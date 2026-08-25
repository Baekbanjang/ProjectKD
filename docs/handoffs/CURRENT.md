# 현재 상태 — 2026-08-24

> **세션 시작 시 여기부터.** 진행상황 · 다음 할 일 · 보류 목록 · 설계 미결.
>
> **이 파일에는 현행만 둔다.** 절이 쌓이면 월 단위로 `archive/` 로 덜어낸다.
> 지난 기록 = [`archive/2026-07~08-past-sessions.md`](archive/2026-07~08-past-sessions.md)
>
> ⛔ 옛 **A/B 레인(기획A / 코드B)은 2026-07-31 종료**. 그 규칙을 따르지 말 것.
> ✅ 2026-08-24부터 **다른 체제로 2세션을 돌린다** — `코드·에셋 = KD` / `문서·지식 = 볼트`.
> 규칙과 대기 항목은 `CLAUDE.md §0 1-1` + 세션 브릿지(`볼트 notes/_세션브릿지.md`)에.

---

## 🟢 2026-08-25 (2) — B2 완료 + 상태 바 버그 해결. **다음 세션은 여기부터**

dev-log = `2026-08-25-refactor-b2-onhitreceived.md` · `2026-08-25-enemy-statebar-resubscribe.md`

### 끝난 것

```
커밋·푸시 6개      리팩토링 A/E/B1 + 처형 큐 태그 주석 + CLAUDE.md 이사분 + docs
B2                OnHitReceived 135줄 -> 4함수. 빌드·PIE 통과
                  (경직 진입하는 타에 안 밀림 = ApplyPoiseDamage 반환값 검증)
상태 바 버그       재락온 후 갱신 정지. Construct 에 재구독 배선. 08-18 부터 있던 버그
보류 13번          닫힘 — PoiseDamageByAttack 키는 전 DA 가 Ability.Player.Parry 하나뿐
```

### C2 완료 (2026-08-25)

dev-log = `2026-08-25-knockback-component.md`

```
UKnockbackComponent 신설     LaunchCharacter(속도) -> RootMotionSource(거리)
DA 필드                      KnockbackStrength(cm/s) -> KnockbackDistance(cm)
                             ⚠️ 뜻이 바뀌어 리다이렉트 일부러 안 걸었다
brain 정지                   Pawn 유지 (StaggerComponent 선례 - brain 소유자 = Pawn)
디버그                       목표 cm 대비 실제 cm. 재현성 확인됨
```

**남은 값 작업** — 더미만 `KnockbackDistance` 200. 적 5종은 체급 기준으로 나중에.

### 다음 (순서)

```
1  Poise 설계 결정        평타로도 깎을지 / 패링 전용 유지할지        <- 판단 대기
2  C1                    콤보 노드 3칸(InputWindow·DamageMultiplier·KnockbackMultiplier)
                         전부 0 -> 타격마다 계수가 안 갈린다. C2 로 넉백 축이 준비됨
3  C3~C5                 아래 이월 표
4  볼트 브릿지 이관       상태 바 dev-log §6 함정 4개 -> 트러블슈팅 박제
```

### 값 작업 완료 (2026-08-25, Content `65fb89d`)

```
KnockbackDistance   6종 전부 200 (C2 로 cm 단위가 된 뒤 첫 실값)
MaxPoise            체급비 5배 유지 — 밴딧·패리밴딧·더미 15 / 궁수 10 / 엘리트 25
PoiseDamageByAttack Parry 4.0 / Light 0.5 / Heavy 0.9  <- 평타로도 깎이게 확장
                    DA_Dummy_Immortal 은 99999 + 빈 맵 유지 (불멸 더미 의도)
```

새 템포 — 밴딧 평타 30대 / 강공 17대 / 패링 4회. 길면 Light 를 0.8~1.0 으로.

### 🟡 미해결

```
적 상태 바 칸 폭     엘리트 MaxPoise 25 = 250px 인데 바 폭이 198. 넘친다.
                    위젯이 SizeBox_Poise 폭을 MaxPoise x 10 으로 잡는 구조.
                    -> 위젯 조정 보류 (승환 판단)
공중 넉백           AccumulateMode::Override 가 중력 누르는지 미검증
오사 본 셰이크       적이 화살 막을 때 뼈 흔들림 뜨는지 미구별.
                    PlayHitFeedback 이 C++ 유일 호출처 — 로그 한 줄로 C++/BP 갈림
Content 잔여        Robot3/ 만 미추적 유지. 나머지는 전부 커밋됨
ShieldAttackDamageRate  SB 적 바의 실드 4칸인지 스태미나 15칸인지 미확정
```

### 🧹 정리 잔가지

```
KDEnemyBaseCharacter.cpp:30   #include "HAL/IConsoleManager.h" 고아
                              (CVarShowKnock 이 컴포넌트로 갔는데 include 만 남음)
KnockbackBrainPause 0.15 < Duration 0.2   측정엔 영향 없음 확인. 연출 판단만
```

---

## ✅ 2026-08-25 (1) — 리팩토링 A·E·B1 완료

dev-log = `docs/dev-logs/2026-08-25-refactor-dedup-deadcode.md` (진단 전문·SB 구조 비교 포함)

### ✅ 1순위 — 해결됨 (2026-08-25 빌드·재시작 완료)

**에디터 종료 → 빌드 → 재시작으로 태그 복구 반영. 피니셔 정상 출력 확인.**
아래는 원인 기록 — 같은 실수 재발 방지용.

#### 원인 — 태그 2개 복구분이 미반영이었다

**A5에서 죽은 코드로 지운 태그 2개가 실은 에셋이 이름으로 쓰던 것이었다.**

```
GameplayCue.Combat.Execution   ← DA_Execution_Axe · DA_Execution_Bandit  (피니셔 VFX)
GameplayCue.Camera.Execution   ← DA_PlayerExecution · GCN_ExcutionCamera  (처형 시네 카메라)
```

증상 = **처형 시 설정한 에셋 시점이 안 나온다** (승환 PIE 실측). C++ 참조는 0이었지만 **에셋이 문자열로 들고 있으면 네이티브 등록을 지우는 순간 태그 해석이 조용히 실패**한다.

→ `KDGameplayTags.h/.cpp` 에 **복구 코드는 이미 넣어뒀다.** 빌드만 남았다.

⚠️ **라이브코딩(Ctrl+Alt+F11)으로는 안 살아날 가능성이 높다.** `UE_DEFINE_GAMEPLAY_TAG` 는 모듈 로드 시 정적 초기화로 등록되는데 라이브코딩 패치는 그걸 다시 돌리지 않는다.

```
에디터 종료 → 빌드 → 에디터 재시작 → 처형 카메라 확인 + 피니셔 VFX(적 2종) 확인
                                     ^^^^ 2026-08-25 실행 완료. 피니셔 정상
```

📌 **교훈 = 태그는 "C++ 참조 0" 만으로 죽은 코드 판정하면 안 된다.** 에셋 문자열 참조를 grep 해야 한다. 복구한 두 줄에 주석으로 박아뒀다.

### 🟡 2순위 — 미조사 1건 (리팩토링 무관으로 보임)

**적끼리 오사 — 데미지는 정상적으로 안 들어가는데, 적이 화살을 막을 때 본 셰이크 같은 게 발동하는 것 같다** (승환 관측).

`KDProjectile::OnSphereOverlap` 은 friendly fire 면 `SendHitEvent` 전에 return 하므로 **히트 이벤트 경로는 아니다.** 후보 = `BP_Arrow` 자체 오버랩 연출(Niagara/사운드) 또는 Destroy 시 이펙트. 미확인.

### PIE 검증 결과 (리팩토링 회귀)

**위 처형 카메라 1건 빼고 전부 이상 없음.** 지상·공중 콤보 / 회피 합류 / 이동 캔슬 / 총격·화살 / 패링 3종 / 카운터 FOV 펀치 통과.

### ✅ 커밋 완료 (2026-08-25, `058349c` ~ `dd4a4c7` 푸시됨)

```
리팩토링   A(소품5) · E(공통화2) · B1(콤보통합)   34파일
태그 복구   KDGameplayTags.h/.cpp                  ← 빌드·확인 후 함께
docs       dev-logs/2026-08-25-*.md · INDEX.md
```

커밋 제안 4분할
```
[refactor] 캔슬 판단 InputComponent 이사 + 회피 진입 DA화 + 죽은 코드 정리
[refactor] 데미지 파이프라인·Self GE 공통화 - KDAbilityStatics / GA_ActionBase
[refactor] 콤보 노드 소비를 GA_PlayerMeleeAttackBase 로 통합
[fix] 처형 큐 태그 2개 복구 - 에셋이 이름으로 참조 중이었다
[doc] 08-25 dev-log
```

### 이월 — 리팩토링 잔여 (목록·판정 근거 = dev-log)

```
B2  OnHitReceived 4분해          독립. SB ResultTable 축(인지/수치/이동/연출)
C1  DA_ComboTree InputWindow 값   B1 끝나 선행조건 해소. SB 실측치 준비됨
C2  넉백 RootMotionSource + KnockbackComponent 추출   B2 선행이면 수월
C3  GA_PlayerOneShotAttack 층 제거   공중 콤보 재설계 때 (BP 리페어런팅 동반)
C4  락온 LoS 방식 통일               락온 재작업 때
C5  PostGEExec 헬퍼 분리            다음 데미지 로직 붙일 때
```

### 📌 룰 파일 재편 (2026-08-25)

전역 `~/.claude/CLAUDE.md` 175행 → 122행 다이어트. 잘라낸 상세는 **`~/.claude/reference/CLAUDE-md-이관-2026-08-25.md`**(자동 로드 X)에 보존.
본 프로젝트 `CLAUDE.md` 에 이사분 3건 반영 완료 — dev-log 자동 생성(§0) · 새 클래스 파일 목록 승인(§0) · MCP 대량 조회 규칙 + **Fable 세션 하청 룰**(§7).

🟡 **승환 할 일** — 취업 `CLAUDE.md` **110~122행 삭제**. 그 폴더엔 §스킬 파이프라인(63~78행)이 이미 있는데 이관 블록이 덧붙어 **중복 2벌**이 됐다(그 파일 37행이 경고하는 바로 그 상황). 원본이 더 최신이라 붙인 쪽을 지우면 된다.

---

## ✅ 2026-08-24 — 문서 대정리 완료

dev-log = `docs/dev-logs/2026-08-22-debug-console-player-init.md` (직전 = `2026-08-21-aim-knockback-camera.md`)

### ✅ `Project_KD` 는 깨끗하다 — 변경 0 / 미푸시 0

오늘 커밋 11개 전부 푸시됨.

```
fc4b1d3  [docs] INDEX dev-log 표를 최신순 + 최근 15개만 펼침
ac03f99  [docs] 핸드오프를 CURRENT + archive 로 분리 + 버터맵 통합본 삭제
5000aef  [docs] design/ 참고용 복구 - 삭제 대신 거짓 목록을 붙여둔다
f31bf02  [docs] design/ 폴더 삭제
c74ccf6  [doc]  KDPlayerState 주석의 Dosul 을 Ammo 로
05b3abe  [doc]  핸드오프를 실제 상태로 + 작동 안 하는 복구 명령 표시
9da756c  [doc]  두 세션 역할 구분 + 세션 브릿지 등록
09c40f7  [docs] 길동 시대 문서 정리                      <- 볼트 세션
e71cd87  [docs] 08-21 / 08-22 dev-log + 현재 상태        <- 볼트 세션
5c32389  [Input] 입력 버퍼 보관 기한 상한 0.8
a975cb2  [refactor] 개발용 온스크린 표시를 콘솔 변수로

Content (원격 없음, 로컬 세이브포인트)
716f0c0  [BP] 촬영용 임시값 원복
070f3b2  [BP] 촬영용 디버그 표시 원복 - 누락 4개
```

### 오늘 한 것 요약

**코드** — 온스크린 디버그 4종을 콘솔 변수로(`KD.ShowDamage`/`ShowDodge`/`ShowApproach`/`ShowKnock`, `ECVF_Cheat`) / 입력 버퍼 `ClampMax` 0.5 → 0.8(기본값 0.2 유지) / `KDPlayerState.h:33` 주석 `Dosul` → `Ammo`

**Content** — 촬영용 임시값 원복(밴딧 Poise 1→3, 퍼펙트 패링 창 0.5→0.2) + 디버그 표시 원복 누락 4건(`GA_AirLightAttack`·`CounterThrust`·`ShotBlast`·`SprintAttack`). **에디터 안 켜고 git 으로만** 했다

**문서** — `design/` 삭제 후 참고용 복구(거짓 4건 표 부착) / 핸드오프 1,052행 → `CURRENT.md` 233 + `archive/` 834 / 버터맵 통합본 2,381행 삭제 / `INDEX` dev-log 표 최신순 + 접기. **저장소 MD 13,648 → 11,267행**

**운영** — 세션 브릿지 신설(볼트 세션과 역할·파일별 담당 확정) / Fable 5 원인 규명

### 🟡 남은 것 = `Content` 8개 (7 M + `Robot3/`)

촬영 관련은 전부 커밋됐다. 아래는 **촬영 커밋 6개가 건드리지도 않은** 잔여분이다.

```
🟡 확인 필요   BP_Dummy         -688   마지막 커밋 180ab96 (08-19)
🟡 확인 필요   BP_Bandit_Parry  -450   마지막 커밋 662390c (MCP 도입 전)
⬜ 노이즈      BP_Bandit -7 / BP_Axe_Elite +8 / Arrow ±0 / Arrow2 ±0
⬜ 맵          LV0_Test.umap    -4699  촬영 조명·노출 원복으로 추정
⬜ 미추적      Robot3/          유지
```

⚠️ **앞의 둘은 크기가 줄었다 = 내용이 실제로 빠졌는데 무엇인지 모른다.** 에디터 켤 일 있을 때 같이 볼 것. 급하지 않음.

### 🔒 승환만 가능 (잠금 영역)

```
docs/reference/README.md:27   "시스템 설계 -> docs/design/" 이 부정확 (design 은 참고용)
GA_MeleeTraceBase.h:60        TipLine 인데 .cpp:21 생성자가 Sweep 로 덮는다. 3개월째
                              헤더만 읽으면 오해한다. 한 줄 수정
```

### 🔀 세션 브릿지 — 볼트 세션과의 유일한 통신로

```
C:/Users/asdasd/Desktop/Obsidian_organize/ProjectKD/notes/_세션브릿지.md
```

**세션 시작 시 이 파일부터 읽는다.** 역할 구분·파일별 편집 담당·대기 항목이 거기 있다.
판정 권한 원칙 = **자기 눈으로 원본을 볼 수 있는 쪽만 단정한다.** 코드·에셋·빌드 = KD / 문서·노트 = 볼트.

### 📌 오늘 세운 방법 (재사용)

- **`.uasset` 원복 판정** — 값이 같아도 재저장하면 바이트가 바뀌어 해시 비교가 무의미. **크기 이력**(`git cat-file -s`)을 본다. 같은 bool 토글은 항상 같은 바이트(-44)라 `변경전 → 변경 → 현재` 세 값이면 판정된다
- **에디터 없이 에셋 되돌리기** — ①에디터가 꺼져 있고 ②되돌릴 커밋 이후 그 파일을 건드린 커밋이 0개면 `git checkout <커밋>^ -- <파일>`. **에디터가 켜져 있으면 금지**(메모리의 옛 버전으로 덮어씀)
- **문서는 낡는 게 아니라 거짓이 된다** — `design/` 을 코드와 대조하니 없는 클래스를 인용하고, 쓰이는 채널을 "미사용"이라 하고, 끝난 구현을 "대기"라 하고 있었다. stale 경고만으로는 어느 줄이 거짓인지 모른다

---

## 3. 보류 목록 (우선순위 순)

> 🔴 **읽기 전에** — 아래 항목 다수가 **2026-07-31 ~ 08-12 에 적힌 것**이라, 그 뒤 dev-log 20편이 쌓이는 동안 이미 해결된 게 섞여 있다.
> **착수 전에 코드로 확인할 것.** 실제 사례 = 1번의 "`DamageMultiplier` 칸 자체가 없다"는 2026-08-24 실측 결과 **거짓**이었다
> (`ComboTreeDataAsset.h:66` 에 있고, 곱하는 자리도 `GA_MeleeTraceBase.cpp:215` 에 배선돼 있다 — 08-19 에 들어옴).


| # | 항목 | 내용 |
|---|---|---|
1 | ~~**`DA_ComboTree` 값 채우기**~~ ✅**칸은 둘 다 생겼다** (2026-08-24 실측: `InputWindow` = `ComboTreeDataAsset.h:62`, `DamageMultiplier` = `:66`, 곱셈 = `GA_MeleeTraceBase.cpp:215`). **남은 건 값 채우기뿐** — 아래 SB 실측치 참조 | **두 값의 상태가 다르다 — 헷갈리지 말 것** (2026-07-31 A레인 지적으로 정정)<br>· **`InputWindow`** = **칸 있음 / 값 전부 0** → 아직 `ComboResetTime 1.5f` 공용값으로 돈다<br>· **`DamageMultiplier`** = **칸 자체가 없다.** `.h` 실측 확인 — DA를 열어도 그 칸은 안 보인다. `FComboNode`에 추가부터 해야 함(`InputWindow` 바로 아랫줄, 같은 형식)<br>SB 입력창 실측: 1~2타 0.7~0.8 / 3~4타 0.9~1.2 / 마무리 1.4~2.0 / 회피 0.8 / 저스트회피 1.5<br>⚠️ **DA는 2개다** — `DA_ComboTree` + `DA_AirComboTree`(같은 `FComboNode` 구조)<br>⚠️ `FComboNode`에 **`DamageEffectClass`(노드별 GE)가 이미 있다** — 계수를 float으로 넣을지 노드별 GE로 갈지 먼저 정할 것. 26노드 × 개별 GE = 에셋 26개라 **float 계수가 가볍다** |
2 | **캔슬 윈도우 늦은 몽타주 3개** | `Combo_02_02`(f62) · `Combo_05_03`(f70) · `Combo_02_03`(f74). 버퍼 0.5초로도 못 덮는다. `ANS_CancelWindow`를 앞으로 당기는 게 유일한 해법 — 단 안무 자체가 후딜이 긴 동작일 수 있어 포즈 재확인 필요 |
4 | ~~발사체 리팩토링 3건~~ **2/3 이미 닫힘** (2026-08-13 실측) | ① 델리게이트 바인딩 → `KDProjectile.cpp:53~54`에서 **`BeginPlay`로 이미 이동됨**(08-10 `b36c1c2`) ✅<br>② `GA_Dodge`가 발사자를 안 봄 → `GA_Dodge.cpp:198`에 **`&& Proj->GetInstigator() != Avatar` 이미 있음** ✅<br>③ faction 게이트 비대칭 → `KDProjectile.cpp:72~77`이 "적→적 통과"만 검사. **살아 있으나 소환수·동료가 생겨야 터진다. 급하지 않음**<br>곁가지 = `InitProjectile`에 방향을 정하는 줄이 없다(스폰 회전을 그대로 씀). 버그가 아니라 현재 설계 |
5 | `EnterNode`가 `Context`를 안 받는다 | 트리를 지상→공중 순차 조회로 우회 중. 노드 ID가 안 겹쳐서 지금은 확실하지만, 겹치는 ID가 생기면 깨진다 |
6 | `OnInActionTagChanged` 재호출 | GA가 겹치면 `NewCount` 1→2로 재호출. 같은 소켓 재부착이라 결과 동일. **제약**: `AttachWeaponToHand()`에 1회성 작업(사운드·이펙트) 넣지 말 것 |
7 | **트레일 NS 변수 검증** | `SwordLength`/`TrailWidth`가 새 NS에 먹는지 PIE 확인. 안 먹으면 NS User Parameter 이름을 맞춰야 함 |

### 2026-08-12 신규 보류 (dev-log `2026-08-12-input-component-and-muzzle-socket.md` 참조)

| # | 항목 | 내용 |
|---|---|---|
8 | ~~`AM_SB_Combo_05_03` 첫 `Shot` 노티 각도~~ **닫힘** (2026-08-13 MCP 실측) | **5개 전부 `muzzleDir=True` / `halfAngle=180` / `ignoreHitStop=True`로 이미 통일돼 있다.** "1번만 10, 나머지 179"는 지나간 기록<br>**총격 노티 전수 = 13개 몽타주 20발.** 05_03(5발)만 위 설정이고 **나머지 12개는 전부 기본값**(`muzzleDir=False` / `halfAngle=0`→GA 값 20도 / `ignoreHitStop=False`)<br>→ 08-13에 붙인 총격 자동 조준은 **12개 몽타주 15발에 먹는다.** 05_03은 각도 180(=전방위)이라 방향이 판정에 영향 없음 |
9 | ~~360° 콘이면 한 적이 5번 맞는다~~ **의도로 확정** (2026-08-13 승환) | 발당 데미지 분배 **안 한다.** 5연타 전방위가 의도. `ignoreHitStop=True`도 그 의도와 맞물림(SB도 다단히트 스텝은 히트스톱을 끈다). `ShotRange` 500도 유지 |
10 | **디버그 구체 그리기 미적용** ★ | 각도 90° 이상이면 `DrawDebugSphere`로 대체. **05_03이 정확히 그 경우라 지금 그 판정 범위를 눈으로 볼 방법이 없다**(`DrawDebugCone`이 180°에서 뒤쪽 한 점으로 뭉쳐 바늘로 보임 — `LineBatchComponent.cpp:515~546`). 판정은 정상, 그리기만 문제<br>코드 = `GA_ShotBlast.cpp:107`의 `ConeRad` 선언을 `else` 안으로 옮기고 `if (HalfAngle >= 90.f) DrawDebugSphere(World, Origin, ShotRange, 24, ...)` 분기. include 불필요(`:163`에서 이미 사용) |
11 | **일반 공격 자동 조준(미착수)** | 락온 안 걸었을 때 가장 가까운 적 쪽으로 자동 회전. `ULockOnComponent::FindBestTarget()`이 public이라 재사용 가능 — `GA_PlayerMeleeAttackBase::OnActivated`의 락온 게이트만 바꾸면 됨. 미결 = 카메라 정면 기준(현재 동작, ±45°)이냐 스틱 입력 방향 기준이냐 |
12 | **`UKDPlayerAbilityInputComponent.cpp` 352줄** | §1 Component 300줄 선 초과. 분리 여부 미결 |
13 | **`DA_Sword_Bandit` 등 적 정의 4개 `PoiseDamageByAttack` 키 미확정 관측** | python 조회 결과 비어 보이나 조회 한계일 수 있음. 에디터에서 직접 확인 필요 |

### 닫힌 항목 (2026-08-12)

- ✅ **`ANS_EnemyAttackWindow`의 `AttackWindowTag`** — 생성자 이니셜라이저 한 줄 추가(`: AttackWindowTag(GameplayTags::State_Combat_EnemyAttackHitWindow)`). 이미 배치된 노티에도 소급 적용(CDO 기본값 직렬화 미포함). 코드 1줄로 끝났다 — 아래는 원인 기록
  - 증상은 맞았다 — 비어 있으면 퍼펙트 회피가 영원히 안 뜬다. 형제 클래스 `ANS_CancelWindow`가 이미 생성자에서 기본값을 넣는 패턴인데(`CancelTag(GameplayTags::State_Combat_CanCancel)`) 이쪽만 빠져 있었다

### 닫힌 항목 (2026-07-30~31)

- ✅ **`GA_Dodge` 레벨 전체 순회 → 반경 물리 조회** (`84b57af`). 채널 2개(Pawn + `ECC_GameTraceChannel1`) 필수 — 발사체는 Pawn이 아니다. `EnvQueryContext_AllyEnemies`는 **고칠 필요 없다**: 타입 지정 순회 + "아레나 3~12마리라 가볍다"고 파일 주석에 판단 근거가 있다
- ✅ **`ANS_WeaponTrail` 액터별 분리** (`84b57af`). 노티 객체는 몽타주 에셋 소속 1개라 모든 액터가 공유 → 멤버 대신 `TMap<MeshComp, Trail>`
- ✅ **`PerfectDodgeWindowSec` 삭제** (`84b57af`)
- ✅ **`FComboNode.InputWindow` 칸 + 배선** (`c0ccc9e`). `ProcessInput` 타이머를 노드 결정 후로 이동, `EnterNode` 우선순위 3단계
- ✅ **입력 버퍼 0.5 / 2** (Content `60698c5`)
- ✅ **`AirComboResetTime` 폐기** (2026-07-31) — `DA_AirComboTree`도 같은 `FComboNode`라 **`Air_1~4` 각각이 이미 `InputWindow` 칸을 가진다.** 컴포넌트 레벨 노브보다 정밀하고 코드 0줄. 항목 자체가 사라졌다
- ✅ **트레일 NS 27개 배정** (Content `4f1ac60`)

### 닫힌 항목

- ✅ **`PerfectDodgeWindowSec` 삭제** (2026-07-30) — 참조 0건인 죽은 값. 실제 퍼펙트 판정은 시간이 아니라 **상태**(적 ASC의 `State.Combat.EnemyAttackHitWindow` 태그). ⚠️ `PerfectDodgeCheckRadius`(500)는 살아 있다
- ✅ **`Block_End` = A안(연결 안 함)으로 확정** (2026-07-30 승환) — C++·BP 그래프 전수 확인 결과 재생 경로가 없고, 지금 자연스럽게 보이는 건 **ABP 블렌드 아웃(0.25초)**이다. 연결하면 가드를 놓고 0.6초간 묶여 "가드 풀었는데 못 움직인다"는 새 문제가 생긴다. SB도 블렌드로 처리
  - 뒤집을 때 방법 3개: ① 짧은 전용 GA ② `GA_Parry` 종료를 몽타주 끝까지 지연 ③ GA 없이 `AnimInstance`에 직접 재생(가장 단순, 순수 연출이라 판정 없음 — 단 §1-3 의존성 방향 확인)

---

## 4. 설계 미결 — 판단 대기 ★

코드가 아니라 **판단이 필요한** 것들. 착수 전에 승환이 정해야 한다.

### ① 스탠스 체계

> **★2026-07-31 — 실목록과 수치를 찾았다.** `Content/Local/Data/CharacterStanceTable.json`. 스탠스가 곧 이동 속도 세트다.

| 스탠스 | Walk | Jogging | **Run** | LockOn Run |
|---|---|---|---|---|
| **`P_Eve_Default`** (평시 = 검) | 150 | 300 | **500** | **280** |
| `P_Eve_BlockSword` / `Fusion` / `Tutorial` / `AirDead` / `Freeze` | 150 | 300 | 500 | 280 |
| **`P_Eve_Tachy`** (각성) / `Fusion2` | 150 | 450 | **600** | 400 |
| **`P_Eve_Gun`** / `Gun_Gorgon` | **100** | **100** | **100** | 100 |
| `P_Eve_GunNikke` / `GunBlockSword` / `GunTutorial` | 120 | 120 | 120 | 120 |
| `P_Eve_Fishing` | 0 | 0 | 0 | 0 |

**여기서 나오는 것 3가지**
- **총 스탠스 = 100. 걷기(150)보다 느리다.** "총은 딜링이 아니라 근접의 준비 도구"라는 §1-B 해석이 수치로 확증됐다. **사격 착수 시 이 값을 그대로 쓴다**
- **락온 전용 속도가 따로 있다** (500 → 280, 44% 감속). 우리는 이 개념이 없다
- **스프린트가 없다.** SB는 150/300/500 3단계. 우리는 250/500/700/800 4단계로 **전반적으로 빠르다** — 우리 Jog(500)가 SB 최고속과 같다

SB는 `Default(=Sword) / Tachy / Fusion / Gun계열 / Fishing / 특수(사망·동결·튜토리얼)`로 나뉜다. **평시 스탠스가 없다** — Eve는 항상 무장 상태다.
우리 초안(승환): `Default(평시 전투 = Sword&Gun) / Gun(진짜 사격만) / 사망`.
→ **사격 로직 착수 전까지 보류 결정됨**(YAGNI). 사격을 시작할 때 이 표를 확정해야 한다.

> **2026-07-31 갱신 — 이제 확정할 재료가 다 모였다.** §1-B의 사격 조사 참조.
> 팩 구성이 **두 설계를 다 지원한다**: (a) SB식 홀드 조준(임시 모드, 놓으면 검 복귀) / (b) 스탠스 전환식(검·총 대등, `Combo_Attack_Shoot` 4타 사용).
> **권장은 (a)** — 검 콤보 구조를 안 건드리고 MM Chooser 컬럼 하나로 조준 로코모션이 들어온다. (b)는 `DA_GunComboTree`가 하나 더 필요하고, 그건 아직 값도 안 채운 콤보 트리를 하나 더 이고 가는 것.
> **사격 착수 전에 확정할 것.**

### ② 공중 콤보 재설계
`Air_01`이 **총 클립**이라 1타에 검 판정이 없다(`MeleeTrace` 없음이 의도).
방향: **07 유지 + 14~16 `Attack_Air_to_Floor`를 마무리로 붙여 지상 콤보로 연결**(공중 공격 → 지상 찍기 → 자연스럽게 지상 콤보).
`02_Attack` 폴더는 **전부 루트모션** — InPlace 클립이 없다. "제자리 공중 공격"은 RM을 끄는 게 아니라 **이동량 0인 클립**으로 얻어야 한다.

### ③ 스태미나 폐기
점프·달리기 자원 폐기 결정(스킬 코스트는 유지). SB 644행 실측으로 검증됨(SB의 Stamina = 적 격파 게이지). **미착수.** 소모처 3곳 값 0 + BP Cost GE 함정 주의.

### ④ 락온 애니
락온 상태 전용 애니가 보류 상태. 검 콤보 → 총 순서로 미뤄뒀다.

### ⑤ 미착수 폴리싱
~~트레일 NS 27개 미배정~~ **✅ 완료 (2026-07-31, Content `4f1ac60`)** / **사운드 노티 없음** / 데미지 GE 26노드 비어 있음 / LoP식 방사형 회피 이펙트.

> **사운드가 이제 제일 큰 구멍이다.** 트레일 27개가 붙어서 **볼 건 생겼는데 들을 게 없다.**
> 노티 자리는 이미 잡혀 있다(`ANS_MeleeTrace` 위치 그대로) — 사운드 노티를 얹는 작업이다. SB도 `FootStepL/R` + `CheckPhyMat`(물리재질 연동)으로 발소리를 따로 관리한다.

> **★ 2026-08-13 MCP 전수 실측으로 그림이 바뀌었다 — 상세 = 메모리 `reference_project_sound_inventory`**
>
> | | 실제 |
> |---|---|
> **발소리** | ❌"전부 없다" → ✅ **`/Game/MotionMatchingAnimation/Audio` 에 286개 이미 있다.** 걷기30·달리기29·스트레이프29·착지20·점프17·구르기9 + `MSS_FoleySound_*` **동작별 래퍼 13개**. **없는 건 재료가 아니라 배선.** Sonniss 없이 지금 붙일 수 있다 |
> **검 소리** | 배선 완료(08-11). `SC_Sword_Hit`→`Metal_Hit_Flesh_1~20` / `SC_Sword_Swing`→`Metal_Light_Whoosh_1~12` / `_Heavy`→`Metal_Heavy_Whoosh_1~10`. **랜덤 풀은 넉넉하다** |
> **"빈 철봉" 정체** | `Metal_Light_Whoosh` = **공기 가르는 소리만 든 팩.** 금속 울림 성분이 없다. → `SlashTrailElemental/Resource/SW_Basic_Slash`·`SW_Distortion_Slash` 를 큐 안에서 Mixer 로 겹치면 붙는다(⚠️ 미시험) |
> **총성** | **0개.** Sonniss GDC 7.47GB 필요. `Gun_and_Sword` 팩엔 사운드가 하나도 없다 |
> **그 밖 없는 것** | 피격 보이스·신음 / UI / 발검·납검 금속음 = 전부 0. 앰비언트는 MM 샘플 딸림 4개뿐 |
>
> 우리가 만든 사운드 에셋은 `/Game/SB_Style_GameProject/Audio/Combat` **큐 3개가 전부**다. `/Game/Assets/Sfx` 2개는 길동 잔재.

### ⑥ 카메라 (2026-07-31 신규)
1단계 값 표는 §1-B에 **완성돼 있다**(BP 6 + 코드 1줄, 30분). 2단계 스플라인 돌리도 실현 가능 확인됨.
**판단 필요**: 카메라를 지금 하나, 사격 뒤로 미루나. 승환은 "시점이 중요하다"고 했고, 값 이식은 30분이라 사격 전에 끼워넣을 수 있다.

---

## 5. 세션 시작 시 읽을 것

**단일 진실 = `CLAUDE.md §0 세션 시작 프로토콜`.** 여기 복사본을 두면 또 갈리므로 포인터만 둔다.

⚠️ **에셋 값은 코드 기본값과 다를 수 있다.** MCP 로 실측할 것 — 2026-07-28 에 공격 GA 5개가 폐기된 길동 창 소켓(`Spear_Tip`)을 물고 있어 근접 판정이 죽어 있었다.

---

## 6. 오늘 배운 함정 3개 (재발 방지)

1. **`.h` 선언과 `.cpp` 정의는 짝이다.** `InAction`이 `.cpp`만 있어서 빌드가 깨졌다. 태그 추가 시 양쪽 확인
2. **구독과 해제는 같은 태그로.** `RegisterGameplayTagEvent(A).Add()` 하고 `RegisterGameplayTagEvent(B).Remove()` 하면 조용히 실패한다 — 핸들 리셋은 그대로 돌아서 코드가 깨끗해 보인다
3. **`search_assets`는 패턴을 무시하고 100개를 통째로 반환한다.** 목록·집계는 `execute_python`으로. 실측: 821개 중 100개가 돌아와 1만 토큰 낭비

### 2026-07-31 추가 4개

4. **`CurveFloat`의 `float_curve`는 Python에 노출 안 된다.** `get_editor_property('float_curve')` → `Failed to find property`. **커브 키 편집은 에디터에서만.** 읽기는 `get_float_value(x)` / `get_time_range()` / `get_value_range()`로 가능하니, 값 샘플링으로 키를 역추출한 뒤 사람이 편집하는 방식이 현실적
5. **UE의 `FieldOfView`는 가로 FOV다.** 16:9에서 75 → 세로는 46.7도(아래로 23.35도뿐). "왜 발이 안 보이지"의 계산 근거가 여기
6. **핸드오프의 "미완"을 믿지 말고 실측할 것.** BP 값 6개가 이미 다 들어가 있었다. `inspect_cdo`로 30초면 확인된다
7. **스플라인 점의 회전이 시선인지 판별하는 법** — 점들의 높이가 크게 다른데 회전값이 같거나 무관하면 **그건 시선이 아니다.** 머리 위 5m와 발치 아래에서 같은 각도로 같은 대상을 볼 수 없기 때문

