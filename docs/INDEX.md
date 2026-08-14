# docs/ 문서 인덱스

프로젝트 전체 문서 카탈로그. 새 문서 추가 시 여기에 한 줄 등록.

> ★ **[PROJECT_OVERVIEW](PROJECT_OVERVIEW.md)** — 프로젝트 전체 개관(현재 방향·코드 아키텍처·에셋 맵·진행상황). **새 세션 부트 문서, 여기부터 읽을 것.**
>
> ★ **코드 설명서 = 옵시디언 볼트 `ProjectKD/notes/코드구조/`** (10문서, 2026-08-08 갱신) — 클래스별 용도·함수 기능·동작 흐름·핵심 코드 발췌. `00_코드구조_MOC`부터. 이 저장소엔 두지 않음(개인 노트 체계).
>
> ★★ **로드맵 = 옵시디언 볼트 `ProjectKD/notes/Project_New/로드맵_GunSword_v1.md`** (2026-07-31) — M1~M6 마일스톤 + 포폴 제출 지점 + 리스크. 주 15~20h 기준 24주(버퍼 포함 31주). ⚠️볼트 `로드맵_마스터.md`는 버터 시절 **폐기**, `docs/reference/3-phases.md`는 길동+페어 시절 **stale**(형식만 계승).
>
> ★ **전투 수치표 = 옵시디언 볼트 `ProjectKD/notes/Project_New/GunSword_전투수치_v1.md`** (2026-07-30, **제안·미반영**) — 콤보 26노드 데미지 계수 + 히트스탑·셰이크·넉백·포이즈·적 HP 티어. SB 실측 + 팩 실측 기반. **계수를 넣을 코드 칸(`FComboNode.DamageMultiplier`)이 아직 없다 = §1 먼저 볼 것.**

> ★ **[2026-07-30 핸드오프 — 2세션 병행 체제](handoffs/2026-07-30-parallel-sessions.md)** — **현재 진행상황·다음 할 일·보류 목록·레인 구분.** 세션 시작 시 여기부터.

최신 dev-log: [2026-08-14 공중 콤보 캔슬 윈도우 정렬](dev-logs/2026-08-14-air-combo-cancel-window.md) · [2026-08-13 일반 공격 자동 조준 + 총격 사운드](dev-logs/2026-08-13-auto-aim-and-gun-sound.md) · [2026-08-12 입력 컴포넌트 분리 + GA 접근자 통일 + 총구 소켓 이전](dev-logs/2026-08-12-input-component-and-muzzle-socket.md) · [2026-08-11 검 사운드 + 웨폰 트레일](dev-logs/2026-08-11-weapon-sound-trail.md) · [2026-08-10 총 3단계 발사체 + 스폰 경로 통합](dev-logs/2026-08-10-gun-projectile.md) · [2026-08-08 총 2단계 사격 GA + 크로스헤어](dev-logs/2026-08-08-gun-fire-crosshair.md) · [2026-07-30 InAction 우산 태그 + 회피 캔슬 통합](dev-logs/2026-07-30-inaction-tag-system.md) · [2026-07-30 길동 몽타주 전면 탈출](dev-logs/2026-07-30-gildong-montage-migration.md) · [2026-07-29 근접 판정 부활 + 콤보 20개 노티](dev-logs/2026-07-29-melee-trace-static-mesh-fix.md) · [2026-07-28 타격 시퀀스 실측](dev-logs/2026-07-28-gunsword-hit-sequence.md)

- **design/** — 시스템 설계 (구현된 기능의 아키텍처 문서)
- **design/기획/** — 게임 기획 (도술/콤보/UI/밸런싱/QTE/분위기/전투진행/적)
- **reference/** — 외부 자료·컨벤션·팀 셋업
- **specs/** — 마일스톤 스펙 (답안지)
- **dev-logs/** — 기능 완료 시 개발 로그 (날짜-기능명)
- **handoffs/** — 세션/환경 인수인계 문서
- **archive/** — ⚠️ **폐기된 작업 라인** (버터/길동). 기본은 읽지 않음 → [archive/README](archive/README.md)

---

## design/ — 시스템 설계

| 문서                                                           | 설명                                                                  |
| -------------------------------------------------------------- | --------------------------------------------------------------------- |
| [README](design/README.md)                                     | design 폴더 안내                                                      |
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
| 2026-07-09 | [motion-matching-research](dev-logs/2026-07-09-motion-matching-research.md)       | 모션매칭 자료조사 (개념+에셋 구조+URL 모음)                                            |
| 2026-07-17 | [melee-trace-refactor](dev-logs/2026-07-17-melee-trace-refactor.md) | 트레이스 무기중립화 (WeaponTrace→MeleeTrace + ETraceMeshSource + 창단위 히트리셋) |
| 2026-07-22 | [gunsword-holster-socket](dev-logs/2026-07-22-gunsword-holster-socket.md) | 검+총 홀스터 소켓 셋업 + DA 교차오염 정정 (Holder 본 붕괴 우회, 발검/납검·PIE는 다음) |
| 2026-07-23 | [gunsword-equip-montage-loco-gating](dev-logs/2026-07-23-gunsword-equip-montage-loco-gating.md) | 발검/납검 속도대별 상체 몽타주 + 로코모션 Stop 입력 게이팅 |
| 2026-07-27 | [gunsword-combo-cancel-timing](dev-logs/2026-07-27-gunsword-combo-cancel-timing.md) | ★콤보 캔슬 타이밍 실측 (포즈 매칭으로 안무가 의도 역추출) + 루트모션 이동량 |
| 2026-07-28 | [gunsword-hit-sequence](dev-logs/2026-07-28-gunsword-hit-sequence.md) | ★타격 시퀀스 실측 (뼈 속도 봉우리) = ANS_MeleeTrace 배치표 + 총 전용 3타 발견 |
| 2026-07-29 | [melee-trace-static-mesh-fix](dev-logs/2026-07-29-melee-trace-static-mesh-fix.md) | ★근접 판정 부활 (무기가 StaticMesh라 코드가 배제) + 지상 콤보 20개 노티 일괄 배치 |
| 2026-07-30 | [gildong-montage-migration](dev-logs/2026-07-30-gildong-montage-migration.md) | ★길동 몽타주 전면 탈출 (단발·방어·회피·피격·공중) + GA 계층 리팩토링 + SB 공중 콤보 실측 |
| 2026-07-30 | [inaction-tag-system](dev-logs/2026-07-30-inaction-tag-system.md) | ★InAction 우산 태그 (가드·회피도 전투 진입+무기 부착) + 회피 캔슬 목록 통합 + ABP 슬롯 재배치 |
| 2026-08-03 | [camera-rail-look-rotation](dev-logs/2026-08-03-camera-rail-look-rotation.md) | ★카메라 레일 2단계 완료 — 조준을 랙 전 이상 위치로 (버그 3개가 한 뿌리) + 마우스 감도 노출. PIE 6항목 통과 |
| 2026-08-04 | [hitstop-component](dev-logs/2026-08-04-hitstop-component.md) | ★히트스톱을 액터 소유 컴포넌트로 — 정지가 노티를 끊어 데미지·판정이 중복되던 버그. CustomTimeDilation=0만 유효 |
| 2026-08-05 | [lockon-pitch-curve](dev-logs/2026-08-05-lockon-pitch-curve.md) | ★락온 시점 고정 버그 — 레일 눈금이 된 피치에 락온이 시선각도를 넣어 한 점으로 수렴. SB식 거리별 피치 커브 + 각도별 좌우 속도 커브 + 사거리 1700 |
| 2026-08-08 | [aim-offset](dev-logs/2026-08-08-aim-offset.md) | ★조준 상체 오프셋 — 레일 진행도를 -1~+1로 정규화(SB 실측), Mesh Space 판단 근거, 감도 배율이 필요했던 이유(134도 대 42도) |
| 2026-08-08 | [gun-fire-crosshair](dev-logs/2026-08-08-gun-fire-crosshair.md) | ★총 2단계 — 사격 GA(상속 대신 태그 조건표), 해제 몽타주는 태스크로 못 켠다, 첫 화면 HUD, `Set Visibility(self)`가 Tick을 죽인 함정, SB 발사 확산 커브 실측 |
| 2026-08-10 | [gun-projectile](dev-logs/2026-08-10-gun-projectile.md) | ★총 3단계 — 발사체 결함 2건(무성 통과·자기 총알 퍼펙트회피) + 스폰 경로를 `KDAbilityStatics` 하나로. `Muzzle Socket` 무성 실패, CDO가 비어도 미설정이 아닌 사례 |
| 2026-08-11 | [weapon-sound-trail](dev-logs/2026-08-11-weapon-sound-trail.md) | ★검 사운드 49칸 배선(타격음은 Cue로 일원화 — 빗나가면 소리 X) + 트레일 27개가 안 보이던 원인(`Distortion_Only`=굴절 전용) + `ANS_WeaponTrail` 파라미터 TMap화. `Trail Width` 공백 하나에 조용히 실패 |
| 2026-08-12 | [input-component-and-muzzle-socket](dev-logs/2026-08-12-input-component-and-muzzle-socket.md) | ★`AKDPlayerCharacter` 700→426줄(입력 컴포넌트 분리) + GA의 구체 Pawn 캐스팅 4곳 제거(접근자 통일) + 총구를 무기 메시 소켓으로 이전. 결함 2건(어빌리티 태그 충돌로 콘 히트스캔 공짜 발동 / `ANS_EnemyAttackWindow` 태그 미초기화로 퍼펙트 회피 no-op) |
| 2026-08-14 | [air-combo-cancel-window](dev-logs/2026-08-14-air-combo-cancel-window.md) | ★공중 콤보 캔슬 윈도우를 안무 기준 프레임으로 (포즈 매칭 + `_All` 교차검증 = 차이 0) + `Air_02`·`_04` 검 재확인. `Air_01`은 캔슬창이 아예 없었다(07-30 문서가 계획만 적힌 건) / fps를 `(nf-1)/L`로 재면 틀린다 → `get_time_at_frame` / `GameplayTag`는 파이썬 repr이 항상 비어 보인다(`tag_name`까지 꺼낼 것) |
| 2026-08-13 | [auto-aim-and-gun-sound](dev-logs/2026-08-13-auto-aim-and-gun-sound.md) | ★락온 없이도 자동 조준(규칙을 거리→각도 최소로, SB 실측 ±90°/500cm) + 총격 판정 원점을 총구→캡슐 + **사운드 전면 배선**(총성 21발·발소리 488개·점프착지·공중 총격). 뼈 좌표로 접지 프레임 자동 검출 / 본 속도비로 총·검 판별(Air_01 = 4.75) / 문서가 틀렸던 것 3건 정정 / SoundCue Mixer 파이썬 생성이 재생 순간 에디터를 죽인 사고 / Concurrency 의 Prevent New 가 발소리를 죽인다 |

## handoffs/ — 인수인계

| 문서                                                                                                     | 설명                                                                    |
| -------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| [2026-07-06-claudian-env-setup](handoffs/2026-07-06-claudian-env-setup.md)                               | Claudian 환경 셋업 인수인계                                             |
| [2026-07-21-pivot-gunsword](handoffs/2026-07-21-pivot-gunsword.md)                                       | ★프로젝트 피벗 — 버터 맨손 폐기 → 9CG Gun&Sword 팩 기반 SB 스타일 액션  |
| [2026-07-30-parallel-sessions](handoffs/2026-07-30-parallel-sessions.md)                                 | ★★**현재 핸드오프** — 2세션 병행(기획A/코드B) 레인 규칙 + 진행상황 + 보류 목록 + 설계 미결 |

## archive/ — ⚠️ 폐기 라인 (기본은 읽지 않음)

현재 프로젝트의 진실이 **아니다.** 설계 근거로 쓰지 말 것. 상세 = [archive/README](archive/README.md)

| 폴더 | 내용 |
| --- | --- |
| [butter/](archive/README.md) | 버터 라인 6개 — 2026-07-21 피벗으로 폐기. ⚠️`버터맵_작업기록_통합`은 **153KB/2381줄, 통째로 열지 말 것** |
| [kildong/](archive/README.md) | 길동 중단 백로그 2개 |
| team_setup_guide.md | 구 페어 시절 셋업 (P4 기술 stale) |
