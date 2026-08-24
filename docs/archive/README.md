# archive/ — 폐기된 작업 라인 문서

> ⚠️ **여기 있는 문서는 현재 프로젝트의 진실이 아니다.** 설계 근거로 쓰지 말 것.
> 지우지 않는 이유는 **파이프라인·노하우에 재활용 가치**가 있어서다(리타게팅 절차, 에셋 임포트 세팅, MM 튜닝 노브 등).

읽을 때는 항상 **"이건 폐기된 라인"** 을 전제로 볼 것. 여기 적힌 클래스명·에셋 경로·수치는 현재 코드와 다르다.

## butter/ — 버터(트릭컬 팬게임) 라인

2026-06-16 길동 중단 → 버터 신규 기획 → **2026-07-21 피벗으로 폐기.**
현재는 Gun&Sword 팩 기반 SB 스타일로 전환됨. 경위 = [handoffs/2026-07-21-pivot-gunsword](../handoffs/2026-07-21-pivot-gunsword.md)

| 문서 | 재활용 가치 |
|---|---|
| ~~`버터맵_작업기록_통합_2026-06-25.md`~~ | 🗑️ **2026-08-24 삭제** (153KB/2381줄). 전문이 다른 데 보존돼 있어 중복이었다 — PART 2 노트 4종은 볼트 `언리얼/맵제작/`·`언리얼/에셋파이프라인/`(북유럽 워크플로우는 볼트가 418행으로 더 길다), PART 1 대화 기록은 메모리 `reference_butter_map_pipeline`(핵심 경로 5줄 + UE 임포트 세팅 + 14단계 워크플로우까지 더 상세) |
| `2026-07-08-butter-mm-locomotion.md` | MM DB 다이어트·Bias·Interrupt Mode 튜닝 노브 (SB 재구축에도 같은 원리 적용됨) |
| `2026-07-11-butter-lockon-strafe.md` | 락온 스트레이프 BS 8방향 + Idle 오염 해결법 |
| `butter-roadmap.md` / `butter-skills.md` / `butter-anim-pipeline.md` | 버터 전용 기획 — 재활용 거의 없음 |

## kildong/ — 길동 라인 (중단된 백로그)

2026-06-16 중단. 재개 가능성은 낮지만 미완 작업 목록으로 남김.

## team_setup_guide.md

2026-06-22 Perforce→Git 솔로 전환으로 무효. **P4 기술은 전부 stale.**

---

## 🗑️ `design/기획/` — 2026-08-24 삭제 (아카이브 아님)

`design/기획/` 하위(도술·콤보·UI·밸런싱·공중QTE·분위기·전투진행·적) 19문서 + 이미지 22개는 **여기로 옮기지 않고 삭제했다.** 볼트 `ProjectKD/notes/ProjectKD/기획/`에 **더 최신 판본이 이미 있어서**다 (21쌍 diff 실측: 19쌍에서 볼트가 최신 또는 동일).

⚠️ **이 문단의 종전 서술을 정정한다.** 전에는 "`Dosul` 어트리뷰트가 코드에 살아 있어 격리하지 않았다"고 적혀 있었으나, 2026-08-24 소스 실측 결과 `Dosul`은 **주석 한 줄(`Source/Project_KD/Player/KDPlayerState.h:33`)에만** 남아 있다. 실제 어트리뷰트도, GA도, Config 태그도 없다.

```
현행 AttributeSet 실측 (2026-08-24)
  AS_CharacterBase   Health / MaxHealth / Poise / MaxPoise / Shield / MaxShield
  AS_Combat          AttackPower / Defense / IncomingDamage
  AS_Player          Stamina / MaxStamina / Ammo / MaxAmmo     ← Dosul 없음, Ammo = Gun&Sword
```

**스토리·세계관 서술은 무효, 시스템 설계 문법(SB 정합 콤보트리·색상 신호 규약)만 참고 가치** — 볼트 쪽에서 읽을 것.
복구는 **필요 없다** — 볼트 `ProjectKD/notes/ProjectKD/기획/` 이 최신본이자 진실 기준이다.
⚠️ 종전에 적혀 있던 `git checkout HEAD~1 -- docs/design/기획` 은 **작동하지 않는다**(2026-08-24 실행 확인). `HEAD~1` 이 이미 삭제 이후 커밋을 가리킨다.
