# Project_KD 데모 — team-setup 진입점

> **2026-05-15 작성 (필규)**. 내일 페어 sync 안건. 정독은 `3-phases.md` + `references.md` 두 개만.

---

## 어디부터 읽을지

| 문서 | 무엇 | 언제 |
|---|---|---|
| **본 README.md** (← 지금) | 한 페이지 요약 + 내일 논의 안건 | **승환 첫 30분** |
| `3-phases.md` | 일정·분담·Phase 본문 (Phase 1/2/3/4 = W1~W13+) | **승환 본 sync 전 정독** |
| `references.md` | 결정 24개 + 공통 클래스 owner 표 + 자산 파이프라인 + 데이터 | 작업 중 reference |
| `prototype-architecture.md` | prototype HKD 코드 구조 (참고용 raw) | 마이그·fix 작업 중 |
| `CLAUDE.md.draft` | 2인 협업 룰 (Perforce CL·페어 리뷰·코드 권한) | 합의 후 레포 루트로 이동 |

> 정보 많아 보여도 핵심 정독 = **3-phases.md (분담·시간순 표) + references.md (결정 #1~#24)** 두 곳.

---

## 핵심 1줄

**3달 12주, 페어 2명. Project_KD = 25분 액션 RPG 데모.** UE5.6 GAS + prototype 검증된 패턴 6종 재사용.

---

## 분담 — **필규 제안, 승환 confirm 필요**

| 영역 | 필규 | 승환 | 페어 |
|---|---|---|---|
| **GAS 인프라** | ✅ Tags / AS / GE / `ATTRIBUTE_ACCESSORS` 매크로 (Day 0) | — | — |
| **prototype 6종 마이그·fix** | ✅ **fix까지 책임** (Day 0 끝까지) | — (raw 그대로 사용) | — |
| **Player 전부** | — | ✅ Pawn 베이스 신규 + 4 GA + 도술 3 + AirCombo + 인벤 + 락온 | — |
| **적·보스·Stagger** | ✅ 잡몹 4 + 호위 엘리트 + 보스 BT/Phase swap + Stagger 시스템 | — | — |
| **UI** | ✅ HUD + 적 측 UI 4 WBP | ✅ Player UI 3 WBP (도술/인벤/QTE) | 시스템 메뉴 |
| **3D 자산** | — | — | ✅ Meshy 6 모델 |
| **레벨·연출** | — | — | ✅ 5구역 / 체크포인트 5 / 환경 / BGM / 채도 / 카메라 |
| **기획** | ✅ 적·보스 | ✅ 도술·콤보·UI·QTE·분위기·전투진행 | — |
| **AWS·Perforce 운영** | ✅ | — | — |
| **프로젝트 골격 (YAGNI)** | Collision/NavMesh (W1) | IMC+IA / GM 셸 (W1+ 필요 시) | SaveGame (W9~10) |

### 이전 안 대비 변경점 (논의 포인트)

1. **prototype fix 책임 = 필규** (이전: 승환 → 필규) — 필규가 sed rename + 결함 5개 fix까지 끝낸 raw 상태로 넘김. 승환은 fix 부담 X.
2. **프로젝트 골격 = YAGNI** (Day 0 미리 작업 X) — UE5 디폴트 GameMode/PC/GI 그대로 사용. IMC+IA·Collision·NavMesh·SaveGame은 닿는 사람이 W1+ 필요 시점에 자연 추가.
3. **Player Pawn 베이스 신규 = 승환** — prototype `HKDPlayerCharacter` 재설계 포함.

---

## 12주 timeline (한눈)

```
W0  Day 0     [필규 1일] GAS 인프라 + prototype 6종 마이그·fix + Perforce 첫 CL    [승환] 합의 1시간만
W1            [필규] 단검 잡몹 + Collision/NavMesh         [승환] Player Pawn 신규 + IMC+IA 7 + 4 GA + 콤보 6
W2  ★ Gate    페어 통합: 손맛 3층 + HUD + 사망/재시작 → "30초 슬라이스 즐거운가?"
W3~5          [필규] 잡몹 3 + Stagger + 적측 UI + HUD     [승환] 도술 3 + Doul + 인벤 GA + UI (IA 추가)
W6~8 ★ Gate   [필규] 호위·보스 BT/Phase swap + UI         [승환] AirCombo + F 처형 + 락온 → "풀세트 작동?"
W9~10         [페어] 레벨 5 + SaveGame/Subsystem + 체크포인트 + BGM   [필규] 보스 P2 [승환] 카메라 2차
W11~12 ★ Gate 폴리싱 + 시스템 메뉴 사이클 + Shipping 빌드 1회 → "60 FPS / 25분 클리어"
W13+          [필규] 스토브 인디지원 [페어] Steam 페이지 + Next Fest (8~9월)
```

---

## 내일 sync 안건 (논의 필요)

1. **분담 confirm** — 위 변경점 3개 OK인가? (특히 프로젝트 골격 = YAGNI / Player Pawn 신규 = 승환)
2. **Phase 1 시작일** — Day 0 시작 = 언제?
3. **Perforce 워크플로우** — `CLAUDE.md.draft` §5-2 룰 합의 (Workspace 네이밍 / CL 단위 / Exclusive lock 풀기 / 빌드 바이너리 정책)
4. **데일리 sync 시간** — 30분 / 어느 시각
5. **마켓플레이스 팩 평가·구매** — 휴머노이드 잡몹팩 + 보스 봉술팩 + BGM 국악팩 + SFX + VFX. **Phase 1 시작 전 또는 W3까지 결정** (예산 = ?)
6. **AI 코드 작성 룰** — `CLAUDE.md.draft` §0 작성자 의도 1~2줄 → AI .h 합의 OK?

---

## 즉시 다음 액션

1. **승환**: 본 README + `3-phases.md` + `references.md` 정독 → 내일 sync 안건 6개 응답 준비
2. **필규**: Day 0 자기 트랙 (GAS 인프라 + prototype 마이그·fix) 즉시 착수 가능
3. **페어**: 마켓플레이스 팩 후보 1~2개씩 찾아오기 (sync 30분)
