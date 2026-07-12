# 콤보 시스템 스코프 변경 — Obsidian/GitHub/P4 일괄 patch 참조용

**날짜**: 2026-05-22
**계기**: W1 우선순위 3-A 진행 중 콤보 시스템 본격 구현 결정 → GitHub 마일스톤 확인 결과 F4(콤보 분기 트리)·F5(방어·회피·패링)가 M2 Epic으로 이미 등록돼 있음을 발견 → M1 스코프 재조정.
**결정**: 옵션 B — M1엔 콤보 시스템 **구조**만, 분기 9 → 3으로 축소. 본격 9분기 + Dodge/Parry는 M2(F4·F5)에서 같은 구조 위에 데이터/기능 추가.

---

## 일괄 patch 대상

| 위치 | 파일 / 위치 | 변경 내용 |
|---|---|---|
| Obsidian | `docs/design/기획/2_콤보/콤보.md` | 콤보 시퀀스 6 → 9, GA 매핑에 UComboComponent 추가, hL 차지 약공 도입, 리셋 1.0s → 1.5s |
| Obsidian | `docs/design/기획/2_콤보/2_콤보.md` | 위 동일 |
| GitHub | 이슈 #10 [F4] 본문 | 6분기 → 9분기, 컴포넌트 협업 구조 명시, hL 차지 약공 명시 |
| GitHub | 이슈 #11 [F5] 본문 | 이미 만든 GE 3개(StaminaCost·DodgeInvincible·ParryWindow) 재사용 명시 |
| P4 | (해당 파일 submit) | Obsidian 변경분 동일 |

---

## 인터뷰 결정 사항 (라운드 1~4 + 옵션 B 재조정)

### 1. 콤보 트리 — 6 → 9분기 (M2 완성 기준)

**Before (기획서 §2_콤보, 2026-05-14 결정)** — 6시퀀스
1. `L → L → L → L` (순수 4타)
2. `L → L → H` (3타째 강공 마무리)
3. `L → L → L → H` (4타째 강공 마무리)
4. `L → H` (빠른 캔슬 강공)
5. `H` (단발)
6. `Hc` (차지)

**After (2026-05-22 결정)** — 9분기 (Stellar Blade 정합)
**Incursion 계열 (L 시작) 5종**
1. `L → H → H → H` (Incursion I)
2. `L → L → H → H` (Incursion II) — 마지막 H 돌진 타격
3. `L → H → L → H` (Incursion III) — 교차 광역
4. `L → L → L → H → H` (Incursion IV)
5. `L → L → L → L` (순수 약공 4타)

**Onslaught 계열 (H 시작) 4종** ⚠️ 기존 누락
6. `H → H → H` (Onslaught I)
7. `H → L → L → H` (Onslaught II)
8. `H → L → hL → H` (Onslaught III) — hL = 차지 약공
9. `H → L → H → L → H` (Onslaught IV, 5타)

**변경 사유**: 단순 인덱스(int) 콤보로는 `L→H` vs `L→L→H` 분기 표현 불가 → Tag 시퀀스 추적기 필수. Onslaught 계열 통째로 누락돼있어 SB식 콤보 불완전. 나중 확장성 위해 풀스펙 구조 + 데이터 분리.

---

### 2. 차지 약공 `hL` 신규 도입 (M2)

- 기존: 없음
- 신규: LMB 0.5s 홀드 = `hL` (charged Light). Onslaught III 분기에 필요.
- 입력 라우팅: IA_LightAttack에 `UInputTriggerHold(0.5s)` 추가. Started = L / Triggered = hL
- IMC 수정 1회 필요 (M2 F4)

---

### 3. GA / 컴포넌트 구조 변경

**Before** (기획서)
```
GA_LightAttack (콤보 인덱스 0→1→2 순환)
GA_HeavyAttack (단발 + 차지)
```

**After** (2026-05-22)
```
UComboTreeDataAsset (9분기 정의, BP에서 편집)
UComboComponent (InputHistory 시퀀스 추적 + 분기 매칭 + 리셋 Timer)
GA_LightAttack (LMB 진입점, 컴포넌트 협업)
GA_HeavyAttack (RMB 진입점, 컴포넌트 협업)
```

**협업 흐름**:
```
LMB → GA_LightAttack 활성화
  → ComboComponent->NotifyInput(Input.Combo.Light)
  → InputHistory 갱신 + 9분기 트리 탐색
  → 매칭 FComboBranch 반환 (Montage + DamageEffectClass)
  → GA가 부모 AttackMontage/DamageEffectClass 갈아끼움
  → Super::ActivateAbility (WeaponTraceBase가 나머지 처리)
```

**선정 근거**: GA 200줄 제한(CLAUDE.md §2 #1) + 분기 트리 데이터 BP 편집 가능성 + 단위 테스트 가능성. CLAUDE.md §1-1 "어디 둘지 모르면 컴포넌트 후보 먼저 검토" 직결.

---

### 4. 타이밍 수치

| 항목 | Before | After |
|---|---|---|
| 콤보 리셋 | 1.0s | **1.5s** (UPROPERTY 노출, BP 튜닝) |
| 캔슬 윈도우 | 0.25s 통일 | 0.25s 통일 유지 (M1). SB식 페이즈 구분은 M2 |
| 입력 버퍼 | 0.2s | 0.2s 유지 |
| 강공 차지 시간 | 0.5s | 0.5s 유지 |
| 패링 윈도우 | 0.2s | 0.2s 유지 |

---

### 5. M1 vs M2 분담 (옵션 B 재조정)

**M1 (5/21~6/3, 게이트 = 30초 슬라이스 즐거움)**
- 콤보 시스템 **구조** 완성 (UComboTreeDataAsset + UComboComponent + GA 2개 협업)
- 분기 **5종** 데이터 등록 (검증용, L/H 섞인 콤보 포함)
  - Incursion I: `L → H → H → H`
  - Incursion II: `L → L → H → H` (I와 L prefix 공유 → 분기 결정 로직 검증)
  - Onslaught I: `H → H → H`
  - Onslaught II: `H → L → L → H` (I과 H prefix 공유 + L/H 교차)
  - 순수 4타: `L → L → L → L`
- 태그 추가: `Input.Combo.Light`, `Input.Combo.Heavy`
- ✅ GE 3종(StaminaCost/DodgeInvincible/ParryWindow) 이미 작성 — M2 재사용 보관

**M2 Alpha (6/4~7/15)**
- **F4 (이슈 #10)**: 분기 6종 추가 (Incursion II/III/IV, Onslaught II/III/IV) + hL 차지 약공 도입 + IMC Hold 트리거 + SB식 캔슬 페이즈 구분 + 분기별 데미지 차등
- **F5 (이슈 #11)**: GA_Dodge + GA_Parry 구현 (M1에 만든 GE 3개 재사용) + Event.Parry.Success 태그 + GE_Damage_Physical Parrying reject 패치
- **F10 (이슈 #13)**: Posture/Stagger 시스템
- Launcher/GuardBreak/돌진 마무리 효과
- Beta Chain ↔ Dosul 매핑 (도술 GA 작업 세션)

---

### 6. 폐기되지 않은 기획서 항목 (그대로 유지)

- Stellar Blade형 약공-깊이 분기 트리 (B-2)
- LMB/RMB 마우스 입력만 (LMB+RMB 동시 입력 금지)
- 봉 특수 폐기 (도술 3종이 특수 슬롯 점유) — 그대로
- 공중 평타 도입 금지 — 그대로
- 보스전 강공 → 부양 → R 동에번쩍 연결 — 그대로

---

## 다음 액션

1. M1 작업: plan 파일(`C:\Users\asdasd\.claude\plans\velvety-drifting-wall.md`) 단계 3a~3h 진행
2. 페어 sync 시 필규에게 본 변경 통지
3. 일괄 patch 시점 (M2 진입 전 / W2 마무리 시점):
   - Obsidian `docs/design/기획/2_콤보/*.md` 갱신
   - GitHub 이슈 #10 #11 본문 patch
   - P4 submit (Obsidian 동기화분)

본 변경 로그는 페어 sync + 일괄 patch 작업 시 단일 진실 참조용.
