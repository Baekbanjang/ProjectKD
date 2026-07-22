# docs/ 문서 인덱스

프로젝트 전체 문서 카탈로그. 새 문서 추가 시 여기에 한 줄 등록.

> ★ **[PROJECT_OVERVIEW](PROJECT_OVERVIEW.md)** — 프로젝트 전체 개관(현재 방향·코드 아키텍처·에셋 맵·진행상황). **새 세션 부트 문서, 여기부터 읽을 것.**

최신 dev-log: [2026-07-23 발검/납검 몽타주 + 로코 Stop 게이팅](dev-logs/2026-07-23-gunsword-equip-montage-loco-gating.md) · [2026-07-22 홀스터 소켓](dev-logs/2026-07-22-gunsword-holster-socket.md)

- **design/** — 시스템 설계 (구현된 기능의 아키텍처 문서)
- **design/기획/** — 게임 기획 (도술/콤보/UI/밸런싱/QTE/분위기/전투진행/적)
- **reference/** — 외부 자료·컨벤션·팀 셋업
- **specs/** — 마일스톤 스펙 (답안지)
- **dev-logs/** — 기능 완료 시 개발 로그 (날짜-기능명)
- **handoffs/** — 세션/환경 인수인계 문서

---

## design/ — 시스템 설계

| 문서                                                           | 설명                                                                  |
| -------------------------------------------------------------- | --------------------------------------------------------------------- |
| [README](design/README.md)                                     | design 폴더 안내                                                      |
| [butter-roadmap](design/butter-roadmap.md)                     | 버터 개발 로드맵 Phase 2~6 (락온 MM/스킬/보스 코미/아트/맵)           |
| [butter-skills](design/butter-skills.md)                       | 버터 전용 스킬 설계 (버서커/도끼 전환/닻 게이지/콤보트리)             |
| [butter-anim-pipeline](design/butter-anim-pipeline.md)         | 버터 애니 제작 파이프라인 (Mixamo/Cascadeur) + 락온 스트레이프 설계표 |
| [camera-curves-system](design/camera-curves-system.md)         | SB식 커브 기반 카메라 시스템 (FOV/피치/거리 커브)                     |
| [collision-channels](design/collision-channels.md)             | 프로젝트 콜리전 채널/프로파일 정의                                    |
| [damage-system](design/damage-system.md)                       | GAS ExecCalc 기반 데미지 계산 파이프라인                              |
| [lockon-system](design/lockon-system.md)                       | 락온 타겟팅 시스템 설계                                               |
| [player-locomotion-system](design/player-locomotion-system.md) | 플레이어 이동 시스템 (BlendSpace 기반, 구 킬동)                       |

## design/기획/ — 게임 기획

| 폴더        | 문서                                                                                                                                                                                                                                                                                 | 설명                                             |
| ----------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------ |
| —           | [README](design/기획/README.md)                                                                                                                                                                                                                                                      | 기획 폴더 구조 안내                              |
| 1\_도술     | [1\_도술](design/기획/1_도술/1_도술.md) · [도술](design/기획/1_도술/도술.md) · [후보풀](design/기획/1_도술/1_도술_후보풀.md) · [GPT프롬프트](design/기획/1_도술/도술_GPT프롬프트.md)                                                                                                 | 도술(스킬) 기획 + 후보 풀 + 이미지 프롬프트      |
| 2\_콤보     | [2\_콤보](design/기획/2_콤보/2_콤보.md) · [콤보](design/기획/2_콤보/콤보.md)                                                                                                                                                                                                         | 콤보 분기 기획 (Light/Heavy 시퀀스)              |
| 3_UI        | [3_UI](design/기획/3_UI/3_UI.md) · [UI](design/기획/3_UI/UI.md) · [HUD_mockup](design/기획/3_UI/HUD_mockup.md)                                                                                                                                                                       | HUD/UI 기획 + 목업 (SB/Wukong 레퍼런스)          |
| 4\_밸런싱   | [4\_밸런싱](design/기획/4_밸런싱/4_밸런싱.md)                                                                                                                                                                                                                                        | 수치 밸런싱 기획                                 |
| 5\_공중QTE  | [5\_공중QTE](design/기획/5_공중QTE/5_공중QTE.md) · [공중QTE](design/기획/5_공중QTE/공중QTE.md)                                                                                                                                                                                       | 공중 QTE 시퀀스 기획 (페이탈→띄움→연타→내리꽂기) |
| 6\_분위기   | [6\_분위기](design/기획/6_분위기/6_분위기.md) · [분위기](design/기획/6_분위기/분위기.md) · [GPT프롬프트](design/기획/6_분위기/분위기_GPT프롬프트.md) · [영상프롬프트](design/기획/6_분위기/분위기_영상프롬프트.md) · [절벽글록시퀀스](design/기획/6_분위기/분위기_절벽글록시퀀스.md) | 아트/톤 기획 + AI 프롬프트                       |
| 7\_전투진행 | [7\_전투진행](design/기획/7_전투진행/7_전투진행.md) · [전투진행](design/기획/7_전투진행/전투진행.md)                                                                                                                                                                                 | 전투 흐름/페이싱 기획                            |
| 8\_적       | [적-컨셉](design/기획/8_적/적-컨셉.md)                                                                                                                                                                                                                                               | 적 유닛 컨셉 (도적 4종 + 정예 + 보스)            |

## reference/ — 외부 자료·컨벤션

| 문서                                                                | 설명                                       |
| ------------------------------------------------------------------- | ------------------------------------------ |
| [README](reference/README.md)                                       | reference 폴더 안내                        |
| [3-phases](reference/3-phases.md)                                   | 프로젝트 3단계 로드맵                      |
| [UE5-GAS-Naming-Convention](reference/UE5-GAS-Naming-Convention.md) | UE5+GAS 네이밍 컨벤션 (GA*/GE*/AS*/AM* 등) |
| [references](reference/references.md)                               | 외부 참고 링크 모음                        |
| [team_setup_guide](reference/team_setup_guide.md)                   | 팀 환경 셋업 가이드 (구 페어 시절)         |

## specs/ — 마일스톤 스펙

| 문서                                                            | 설명                                     |
| --------------------------------------------------------------- | ---------------------------------------- |
| [README](specs/README.md)                                       | specs 폴더 안내                          |
| [deep-interview-abp-weapon](specs/deep-interview-abp-weapon.md) | ABP/무기 시스템 deep interview 결정 기록 |

## dev-logs/ — 개발 로그 (시간순)

| 날짜       | 문서                                                                              | 기능                                                                                   |
| ---------- | --------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| 2026-05-22 | [attribute-restructure](dev-logs/2026-05-22-attribute-restructure.md)             | AttributeSet 재구성                                                                    |
| 2026-05-22 | [combo-scope-change](dev-logs/2026-05-22-combo-scope-change.md)                   | 콤보 스코프 6→9분기 변경                                                               |
| 2026-05-22 | [gas-iframe-parry-research](dev-logs/2026-05-22-gas-iframe-parry-research.md)     | GAS i-frame/패링 자료조사                                                              |
| 2026-05-23 | [combo-system-day1](dev-logs/2026-05-23-combo-system-day1.md)                     | 콤보 Day1 — 구조 + ComboComponent                                                      |
| 2026-05-24 | [combo-system-day2](dev-logs/2026-05-24-combo-system-day2.md)                     | 콤보 Day2 — GA_Light/Heavy                                                             |
| 2026-05-25 | [combo-system-day3](dev-logs/2026-05-25-combo-system-day3.md)                     | 콤보 Day3 — 분기 완성 + PIE 검증                                                       |
| 2026-05-26 | [perfect-parry-cue-migration](dev-logs/2026-05-26-perfect-parry-cue-migration.md) | 퍼펙트 패링 GameplayCue 이관                                                           |
| 2026-05-27 | [movement-cancel-notify](dev-logs/2026-05-27-movement-cancel-notify.md)           | 이동 캔슬 노티파이                                                                     |
| 2026-05-28 | [camera-sb-curve-system](dev-logs/2026-05-28-camera-sb-curve-system.md)           | SB식 커브 카메라                                                                       |
| 2026-05-30 | [player-locomotion](dev-logs/2026-05-30-player-locomotion.md)                     | 플레이어 로코모션 (BlendSpace)                                                         |
| 2026-06-02 | [double-jump](dev-logs/2026-06-02-double-jump.md)                                 | 더블 점프                                                                              |
| 2026-06-02 | [jump-landing-pivot-turn](dev-logs/2026-06-02-jump-landing-pivot-turn.md)         | 점프 착지/피벗 턴                                                                      |
| 2026-06-02 | [run-to-stop](dev-logs/2026-06-02-run-to-stop.md)                                 | 런투스톱                                                                               |
| 2026-06-03 | [dodge-seam-rootmotion-mode](dev-logs/2026-06-03-dodge-seam-rootmotion-mode.md)   | 닷지 이음새/루트모션 모드                                                              |
| 2026-06-04 | [perfect-parry-juice](dev-logs/2026-06-04-perfect-parry-juice.md)                 | 퍼펙트 패링 juice 5겹                                                                  |
| 2026-06-08 | [lockon-fixes](dev-logs/2026-06-08-lockon-fixes.md)                               | 락온 버그 수정                                                                         |
| 2026-06-11 | [air-combo](dev-logs/2026-06-11-air-combo.md)                                     | 공중 콤보                                                                              |
| 2026-06-11 | [counter-camera-juice](dev-logs/2026-06-11-counter-camera-juice.md)               | 카운터 카메라 juice                                                                    |
| 2026-06-12 | [combat-tag-polish](dev-logs/2026-06-12-combat-tag-polish.md)                     | 전투 태그 폴리싱                                                                       |
| 2026-06-12 | [player-refactor](dev-logs/2026-06-12-player-refactor.md)                         | 플레이어 리팩토링 패스                                                                 |
| 2026-06-13 | [execution](dev-logs/2026-06-13-execution.md)                                     | 처형 시스템                                                                            |
| 2026-07-08 | [butter-mm-locomotion](dev-logs/2026-07-08-butter-mm-locomotion.md)               | 버터 MM 로코모션 재구축 Step 1~6 완료 (스블식 다이어트 + Interrupt Mode + 폴리싱 이월) |
| 2026-07-09 | [motion-matching-research](dev-logs/2026-07-09-motion-matching-research.md)       | 모션매칭 자료조사 (개념+에셋 구조+URL 모음)                                            |
| 2026-07-11 | [butter-lockon-strafe](dev-logs/2026-07-11-butter-lockon-strafe.md)               | 버터 락온 스트레이프 (Sword 리타게팅 + BS 8방향 + ABP 리페어런팅, Idle 오염 해결)      |
| 2026-06-25 | [버터맵 작업기록 통합](dev-logs/버터맵_작업기록_통합_2026-06-25.md)               | 버터 맵 배경 에셋 작업 기록 통합본 (바탕화면에서 이관)                                 |
| 2026-07-17 | [melee-trace-refactor](dev-logs/2026-07-17-melee-trace-refactor.md) | 트레이스 무기중립화 (WeaponTrace→MeleeTrace + ETraceMeshSource + 창단위 히트리셋) |
| 2026-07-22 | [gunsword-holster-socket](dev-logs/2026-07-22-gunsword-holster-socket.md) | 검+총 홀스터 소켓 셋업 + DA 교차오염 정정 (Holder 본 붕괴 우회, 발검/납검·PIE는 다음) |

## handoffs/ — 인수인계

| 문서                                                                                                     | 설명                                                                    |
| -------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| [2026-06-15-kildong-execution-contextual-anim](handoffs/2026-06-15-kildong-execution-contextual-anim.md) | [길동 보류] 처형 Contextual Anim 도입 — 스니펫 6건 적용 전 중단, 재개용 |
| [2026-06-15-kildong-backlog](handoffs/2026-06-15-kildong-backlog.md)                                     | [길동 보류] 대기 백로그 — 마지막 적 슬로우모션 + 휘젓기 폴리싱          |
| [2026-07-06-claudian-env-setup](handoffs/2026-07-06-claudian-env-setup.md)                               | Claudian 환경 셋업 인수인계                                             |
| [2026-07-21-pivot-gunsword](handoffs/2026-07-21-pivot-gunsword.md)                                       | ★프로젝트 피벗 — 버터 맨손 폐기 → 9CG Gun&Sword 팩 기반 SB 스타일 액션  |
