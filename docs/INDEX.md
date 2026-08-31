# docs/ 문서 인덱스

프로젝트 전체 문서 카탈로그. 새 문서 추가 시 여기에 한 줄 등록.

> ★ **[PROJECT_OVERVIEW](PROJECT_OVERVIEW.md)** — 프로젝트 전체 개관(현재 방향·코드 아키텍처·에셋 맵·진행상황). **새 세션 부트 문서, 여기부터 읽을 것.**
>
> ★ **코드 설명서 = 옵시디언 볼트 `ProjectKD/notes/코드구조/`** (10문서, **2026-08-23 갱신** — 종전 "08-08" 표기는 stale이라 2026-08-24 정정) — 클래스별 용도·함수 기능·동작 흐름·핵심 코드 발췌. `00_코드구조_MOC`부터. 이 저장소엔 두지 않음(개인 노트 체계).
>
> 🟠 **`docs/design/` = 참고용 보존. 현행 아님** — 2026-05에서 멈췄고 **지금은 거짓인 문장이 섞여 있다**(없는 클래스를 인용 / 쓰이는 채널을 "미사용"이라 서술 / 끝난 구현을 "대기"라 표기 / 데미지 파이프라인에 Shield 층 누락). **설계 문서의 현행 진실은 볼트 `notes/코드구조/`다.** 여기서 읽을 것은 "왜 그렇게 골랐나"뿐이고, **파일:줄 인용과 구현 상태는 믿지 말 것.** 거짓 목록 = 아래 §design.
>
> ★★ **로드맵 = 옵시디언 볼트 `ProjectKD/notes/Project_New/로드맵_GunSword_v1.md`** (2026-07-31) — M1~M6 마일스톤 + 포폴 제출 지점 + 리스크. 주 15~20h 기준 24주(버퍼 포함 31주). ⚠️볼트 `로드맵_마스터.md`는 버터 시절 **폐기**, `docs/reference/3-phases.md`는 **2026-08-24 삭제**(길동+페어 시절, 볼트에 최신본).
>
> ★ **전투 수치표 = 옵시디언 볼트 `ProjectKD/notes/Project_New/GunSword_전투수치_v1.md`** (2026-07-30, **제안·미반영**) — 콤보 26노드 데미지 계수 + 히트스탑·셰이크·넉백·포이즈·적 HP 티어. SB 실측 + 팩 실측 기반. **계수를 넣을 코드 칸(`FComboNode.DamageMultiplier`)이 아직 없다 = §1 먼저 볼 것.**

> ★ **[handoffs/CURRENT.md](handoffs/CURRENT.md)** — **현재 상태 · 다음 할 일 · 보류 목록 · 설계 미결.** 세션 시작 시 여기부터.

최신 dev-log: [2026-08-31 카메라 랙 버스트 + 반격 워프 잔류 수정](dev-logs/2026-08-31-camera-lag-burst.md) · [2026-08-31 슬로모 시계 정정 + 반격 TimeScale 배선](dev-logs/2026-08-31-slowmo-timescale-fix.md) · [2026-08-31 반격 히트 프로필 — 무음 해소](dev-logs/2026-08-31-counter-hitconfirm-profile.md) · [2026-08-31 SB 언리얼페스트2024 발표 전수 판독](dev-logs/2026-08-31-SB언리얼페스트2024-전수판독.md) · [2026-08-30 연출 구조 감사 + 슬로모 Subsystem 신설](dev-logs/2026-08-30-presentation-audit.md) · [2026-08-30 퍼펙트 패링 반격(우) 노티 배치 + 표준 트랙 구성](dev-logs/2026-08-30-counter-slash-notify.md) · [2026-08-30 가드 로코모션 + 가드 붕괴 + 실드 회복 지연](dev-logs/2026-08-30-guard-locomotion-break.md) · [2026-08-28 패링 재설계 1·2·4① — 창/반격 사슬/막기의 대가](dev-logs/2026-08-28-parry-redesign.md) · [2026-08-28 적 몽타주 노티 11개 복구 + 바이너리 전수 검사](dev-logs/2026-08-28-enemy-montage-notify-recovery.md) · [2026-08-27 총격 조준 필터 데이터화 + 죽은 노브 제거](dev-logs/2026-08-27-shotblast-filter-and-dead-knob.md) · [2026-08-27 DA 5개 로드 불능 복구 — 개명 재저장 누락](dev-logs/2026-08-27-dataasset-load-failure.md) · [2026-08-27 적 상태 바 — 락온 외 피격에서도 표시](dev-logs/2026-08-27-enemy-statebar-visibility.md) · [2026-08-27 대상 탐색 필터 구조 + 자동 조준 결함 6건 해소](dev-logs/2026-08-27-target-filter-autoaim.md) · [2026-08-26 개명 에디터 검증 + GCN 큐 맵 함정 + 전수 재저장](dev-logs/2026-08-26-rename-editor-verification.md) · [2026-08-26 클래스 네이밍 규약 정렬 68개 개명](dev-logs/2026-08-26-class-naming-convention.md) · [2026-08-26 Poise 2단계 — 커스텀 EffectContext 통로](dev-logs/2026-08-26-poise-context-channel.md) · [2026-08-26 C3·C4·C5 리팩토링 + 달리기 공격 태그 복구](dev-logs/2026-08-26-refactor-c3-c4-c5.md) · [2026-08-25 C2 넉백을 거리(cm) 기반 컴포넌트로](dev-logs/2026-08-25-knockback-component.md) · [2026-08-25 적 상태 바 재락온 후 갱신 정지 — Construct/Destruct 비대칭](dev-logs/2026-08-25-enemy-statebar-resubscribe.md) · [2026-08-25 B2 OnHitReceived 4분해](dev-logs/2026-08-25-refactor-b2-onhitreceived.md) · [2026-08-25 리팩토링 A·E·B1 — 중복 통합 + 죽은 코드 정리](dev-logs/2026-08-25-refactor-dedup-deadcode.md) · [2026-08-22 디버그 콘솔화 + 플레이어 초기값 GE + 마무리 워프 회전](dev-logs/2026-08-22-debug-console-player-init.md) · [2026-08-21 조준 정확도 + 넉백 실효화 + 공중 클립 카메라](dev-logs/2026-08-21-aim-knockback-camera.md) · [2026-08-20 콤보 진입기 접근 워프](dev-logs/2026-08-20-approach-warp.md) · [2026-08-19 콤보 데미지 계수 + 총격 조준 제한 + 더미](dev-logs/2026-08-19-combo-damage-scaling.md) · [2026-08-19 적 근접 판정 부활 — 무기 메시 태그 불일치](dev-logs/2026-08-19-enemy-melee-trace-tag.md) · [2026-08-18 적 상태 바 HUD 구현](dev-logs/2026-08-18-enemy-state-bar.md) · [2026-08-17 적 상태 바 HUD 설계 + 총구 이펙트](dev-logs/2026-08-17-hud-enemy-state-bar-spec.md) · [2026-08-16 근접 판정 반경 + SB 캐릭터 정합](dev-logs/2026-08-16-melee-radius-and-character-fit.md) · [2026-08-14 공중 콤보 캔슬 윈도우 정렬](dev-logs/2026-08-14-air-combo-cancel-window.md) · [2026-08-13 일반 공격 자동 조준 + 총격 사운드](dev-logs/2026-08-13-auto-aim-and-gun-sound.md) · [2026-08-12 입력 컴포넌트 분리 + GA 접근자 통일 + 총구 소켓 이전](dev-logs/2026-08-12-input-component-and-muzzle-socket.md) · [2026-08-11 검 사운드 + 웨폰 트레일](dev-logs/2026-08-11-weapon-sound-trail.md) · [2026-08-10 총 3단계 발사체 + 스폰 경로 통합](dev-logs/2026-08-10-gun-projectile.md) · [2026-08-08 총 2단계 사격 GA + 크로스헤어](dev-logs/2026-08-08-gun-fire-crosshair.md) · [2026-07-30 InAction 우산 태그 + 회피 캔슬 통합](dev-logs/2026-07-30-inaction-tag-system.md) · [2026-07-30 길동 몽타주 전면 탈출](dev-logs/2026-07-30-gildong-montage-migration.md) · [2026-07-29 근접 판정 부활 + 콤보 20개 노티](dev-logs/2026-07-29-melee-trace-static-mesh-fix.md) · [2026-07-28 타격 시퀀스 실측](dev-logs/2026-07-28-gunsword-hit-sequence.md)

- 🟠 **design/** — **참고용 보존(2026-05 정지).** 현행 진실은 볼트 `notes/코드구조/`. **파일:줄 인용과 구현 상태는 믿지 말 것** (§design 참조)
- ~~**design/기획/**~~ — 🗑️ **2026-08-24 삭제.** 길동 시대 기획. 볼트 `ProjectKD/notes/ProjectKD/기획/`에 최신본 보존 (§삭제 기록 참조)
- **reference/** — 외부 자료·컨벤션
- **specs/** — 마일스톤 스펙 (답안지)
- **dev-logs/** — 기능 완료 시 개발 로그 (날짜-기능명)
- **handoffs/** — 세션/환경 인수인계 문서
- **archive/** — ⚠️ **폐기된 작업 라인** (버터/길동). 기본은 읽지 않음 → [archive/README](archive/README.md)

---

## 🟠 design/ — 참고용 보존 (2026-05 정지, 현행 아님)

한 번 삭제했다가(`f31bf02`) **참고용으로 되살렸다.** 5문서 모두 2026-05-22~30 이후 갱신이 없다.

**⚠️ 이 폴더에서 읽을 것은 "왜 그렇게 골랐나"뿐이다.** 코드 인용과 구현 상태는 **아래처럼 이미 거짓이다** — 2026-08-24 코드 대조 실측.

| 문서가 말하는 것 | 실제 |
| --- | --- |
| `AT_WeaponTrace.cpp:81` · `GA_WeaponTraceBase.cpp:17` 인용 | **두 클래스 다 없다.** `AT_MeleeTrace` · `GA_MeleeTraceBase` 로 개명됨 |
| `ECC_GameTraceChannel1`(Projectile) = "예약 — 미사용" | **쓰인다.** `GA_Dodge.cpp:188` 퍼펙트 회피가 발사체를 잡는 채널 |
| §미결/차주 = `IncomingDamage`+`PostGEExec` / 패링 윈도우 | **둘 다 구현 완료** |
| 데미지 파이프라인 = `Defense → Health` | **Shield 층이 빠졌다**(08-18 신설). 이 그림대로 설계하면 실드를 빠뜨린다 |

**설계 문서의 현행 진실 = 볼트 `notes/코드구조/`** (10문서). 주제별 대응은 아래.

```
camera-curves      → 볼트 02_플레이어 · 08_총_조준_스탠스
collision-channels → 볼트 05_애님노티파이_큐 · 01_전투_GAS_어빌리티
damage-system      → 볼트 04_어트리뷰트_이펙트 · 01_전투_GAS_어빌리티
lockon-system      → 볼트 03_전투_컴포넌트 · 01_전투_GAS_어빌리티
player-locomotion  → 볼트 02_플레이어            (08-24 먼저 삭제됨)
```

**여기서 건져낸 함정 2건** — 볼트에도 이 저장소에도 없던 것이라 트러블슈팅으로 옮겼다. **그쪽이 원본이다**(이 폴더가 다시 낡아도 함정은 살아 있다).

| 건진 것 | 이관처 |
| --- | --- |
| `Can Be Targeted` 디폴트 False — BP 가 C++ `_Implementation` 을 이겨 락온이 조용히 죽는다 | 볼트 `트러블슈팅/GAS-BlueprintNativeEvent_BP디폴트가_C++를_이긴다` |
| `OverlapMultiByObjectType` 이 같은 액터를 **콜리전 컴포넌트 수만큼** 중복 반환 | 볼트 `트러블슈팅/콜리전-Overlap이_같은액터를_컴포넌트수만큼_중복반환` |

> ℹ️ `Visibility` 함정과 `ExecCalc` 미채택 근거는 **이미 다른 데 있어** 안 건졌다 — 각각 볼트 `트러블슈팅/콜리전-Pawn이_Visibility를_무시한다`(08-21) 와 `CLAUDE.md §1-2`.
> 🔴 **이 폴더에서 유일하게 아직 유효한 지적** — `GA_MeleeTraceBase.h:60` 이 `TipLine` 인데 `.cpp:21` 생성자가 `Sweep` 로 덮는다. `collision-channels.md` 말미에 "헤더도 맞춰라"고 적혔고 **3개월째 그대로다. 헤더만 읽으면 오해한다.**

## 🗑️ design/기획/ — 2026-08-24 삭제됨

길동 시대(2026-05~06) 게임 기획 19개 + 이미지 22개. **볼트에 최신본이 있어 이쪽을 정리했다.**

| 항목 | 내용 |
|---|---|
| 보존 위치 | 볼트 `ProjectKD/notes/ProjectKD/기획/` (8폴더 + `자료/` 이미지 22개 전부) |
| 판정 근거 | 21쌍 diff 실측 — 19쌍에서 볼트가 최신이거나 동일 |
| 증거 | KD판 `[1] 분신 [2] 축지 [3] 스턴` = 구 도술명 / 볼트판 `[1] 화부 [2] 풍보 [3] 정승` = 신 도술명. KD판 `AGdPlayerCharacter` = 사망한 `Gd` 접두 |
| 예외 | `적-컨셉.md`만 KD판이 최신(2026-06-01 색상 규약)이라 **볼트로 이관 후** 삭제. 종전 볼트판은 `적-컨셉_구버전_2026-06-15.md`로 보존 |
| 복구 | **필요 없다.** 볼트에 최신본이 있고 그쪽이 진실 기준. ⚠️종전에 적혀 있던 `git checkout HEAD~1 -- docs/design/기획` 은 **작동하지 않는다**(2026-08-24 실행 확인 — `HEAD~1` 이 이미 삭제 이후 커밋). 정말 필요하면 삭제 직전 커밋을 SHA 로 짚을 것 |

⚠️ 종전 `archive/README.md`는 "`Dosul` 어트리뷰트가 코드에 살아 있어 격리하지 않았다"고 적혀 있었으나, **2026-08-24 실측 결과 `Dosul`은 주석 한 줄(`KDPlayerState.h:33`)에만 남아 있고 실제 어트리뷰트는 없다.** 현행 `AS_Player` = `Stamina` · `Ammo`.

## reference/ — 외부 자료·컨벤션

| 문서                                                                | 설명                                       |
| ------------------------------------------------------------------- | ------------------------------------------ |
| [README](reference/README.md)                                       | reference 폴더 안내                        |
| [UE5-GAS-Naming-Convention](reference/UE5-GAS-Naming-Convention.md) | UE5+GAS 네이밍 컨벤션 (GA*/GE*/AS*/AM* 등) |

🗑️ `3-phases.md` · `references.md` — 2026-08-24 삭제. 길동+페어 시절 12주 일정·클래스 분담(`Gd` 접두)이라 **이 폴더의 정의(외부 자료 보관)에도 안 맞았다.** 볼트 `notes/ProjectKD/0_팀셋업/`에 최신본 있음.

## specs/ — 마일스톤 스펙

| 문서                                                            | 설명                                     |
| --------------------------------------------------------------- | ---------------------------------------- |
| [README](specs/README.md)                                       | specs 폴더 안내                          |

🗄️ `deep-interview-abp-weapon.md` — **2026-08-24 `archive/kildong/`으로 이동.** 길동 스켈레톤 + 창(Spear) 전제라 폐기. 단 **`WeaponComponent` + `weapon_r` 소켓 결정은 현행 코드에 살아 있다**(`Combat/WeaponComponent.h:45`, `Combat/Data/WeaponDataAsset.h:46`) → 설계 근거로 보존.

## dev-logs/ - 개발 로그 (최신 순)

총 **50개**. 설명은 원문 그대로 둔다 - 여기서 훑어보고 필요한 것만 여는 용도라 요약을 줄이면 그 값이 사라진다.

### 최근 15개

| 날짜 | 문서 | 내용 |
| --- | --- | --- |
| 2026-08-31 | [camera-lag-burst](dev-logs/2026-08-31-camera-lag-burst.md) | ★핸드오프 B1(`GCN_CounterTrail` 배선)을 처리하려다 **결함 셋**이 나왔다. ①08-30 조사의 *"발신자 0건"* 은 **틀렸다** — `AM_..._L` PlayerCue 노티에 태그가 이미 박혀 있었고(몽타주 스캔이 노티 안 `FGameplayTag` 값을 안 봤다), 안 나온 진짜 이유는 **수신 쪽**(GCN에 `OnExecute` 훅 자체가 없음 + `Cast To KDPlayerCharacter` §1-3 위반 + 복귀값 `20` 하드코딩, 실제 19). 버리고 `GCN_CameraLagBurst`(**Static** — `ExecuteGameplayCue`↔`OnExecute` 짝)로 신설. ②소유권을 `UKDSpringArmComponent` 로 — `RequestLagBurst(Scale/MaxDist/Duration/BlendSpeed)` 는 **주문서만 놓고** 값은 안 건드리고, Tick의 `UpdateLagBurst` 가 민다. **복구를 요청으로 만들지 않았다** — `Remaining` 이 0을 지나면 목표가 스스로 `DefaultLagSpeed`(BeginPlay 원본)로 갈아탄다. 시계는 슬로모와 **반대로 게임시간**이 맞다(엔진 랙 자체가 DeltaTime 보간). ③★★**워프 타겟 잔류** — 원거리 적 패링 후 이동해 재패링하면 **처음 패링한 좌표로 강제 워프**. `Dist > MaxDashRange` 조기 return이 갱신을 건너뛰는데 낡은 `CounterTarget` 이 월드 좌표로 살아 있었다(주석의 *"대시 생략"* 이 실제론 *"옛 좌표로 대시"*). 발동 시작에 `RemoveWarpTarget` — `PlayerMelee`·`EnemyRushAttack` 은 이미 하고 있었고 `CounterThrust`·`PlayerExecution` 만 빠져 있었다. ★SB 실측 = `SBShowControlCamLagSpeedKey` 275개 실재(`TargetSpeed 0.5`/`MaxDistance 100`/`Duration 0.5`), **약공만 랙·강공은 쉐이크 2배**라 우리 `_L`만 워프인 구조가 원래 맞았다 → `GA_CounterSlash` 슬로모 0.2→**0.1** |
| 2026-08-31 | [slowmo-timescale-fix](dev-logs/2026-08-31-slowmo-timescale-fix.md) | ★★`UKDSlowMotionSubsystem` 이 **시계 두 개를 섞어 써서** 슬로모가 요청 길이의 **`1/Scale` 배**로 지속되고 있었다 — 만료는 실시간(`GetRealTimeSeconds`, `World.h:1879` "IS NOT dilated"), 알람은 게임 시계 타이머(`LevelTick.cpp:1577` 이 dilated 델타를 먹인다). 실측 = 퍼펙트 패링 `0.15 -> 실시간 0.50초` / 퍼펙트 회피 `1.5 -> 2.14초`. 08-30 기록의 *"한 프레임 늦다"* 는 **배율의 역수배**가 맞다. 같은 함정이 `KDHitStopComponent` 재개 타이머에도 있어 **슬로모를 얹는 순간 반격 정지가 실시간 0.75초**가 될 뻔했다. 수정 = 양쪽 다 `실시간 초 × 현재 배율 = 게임 시계 초` 환산(각 1~2줄). ★그 위에 **GA 슬로모 칸 3개**(`HitSlowMoScale/Duration/Priority`)를 `UKDGameplayAbility_PlayerMelee` 에 신설 — 기본값 `1.f/0.f` 라 **"평타엔 슬로모 X"가 구조로 강제**된다. 호출은 `bIgnoreHitStop` 조기 반환보다 **위**(별개 레버). ★**정정** — 세션 중 "SB 는 나와 적이 같이 멈춘다"고 했으나 **틀렸다**, 발표 원문 = **공격자만** 0.07초. 그래서 어제 올린 히트스톱 `0.15 -> 0.08` 복귀(묵직함의 출처는 TimeScale 이었다). 겹침(패링·반격 둘 다 P10)은 **버그를 고치자 함께 해소** — 패링이 0.15초로 짧아져 반격 판정(f4) 전에 끝난다. 곁가지 = 브릿지 대기 2건 실측(입력 버퍼 **0.8/2** · 락온 **DA 가 필터를 담는 그릇**, Radius 1000) |
| 2026-08-31 | [counter-hitconfirm-profile](dev-logs/2026-08-31-counter-hitconfirm-profile.md) | ★반격 슬래시가 **소리도 이펙트도 안 났다** — `GA_CounterSlash.HitConfirmProfile = None`. `GCN_PlayerHitConfirm` 은 **`Cast To KDHitConfirmProfile` 이 유일한 관문**이라 프로필이 없으면 소리·VFX·쉐이크가 **한꺼번에** 끊긴다(개별 null 은 엔진이 막아줘서 안전 — `PlayerCameraManager.cpp:1249` · `GameplayStatics.cpp:1698`). SB식 분리 채택 — **평타는 쉐이크 X**(Show 데이터 511파일 중 평타에 `SBShowCamShakeKey` 0개), 반격만 **FOV 렌즈 펀치**(`Freq 1.5` = 진동이 아니라 한 번 훅 들어갔다 나옴). ★**DA 에 필드를 추가하지 않았다** — ShakeScale=`HitConfirmMagnitude`, TimeScale=`AttackerHitStopDuration` 이 이미 GA 에 있고 `RawMagnitude`→Scale 핀 배선까지 돼 있다. ⚠️`InitialOffset` 기본값이 `Random` 이라 느린 파형은 **줌인/줌아웃이 매번 뒤집힌다** → `Zero` 필수(`LegacyCameraShake.cpp:54`). PIE = 반격 소리·이펙트 부활 ✅ / FOV -20·히트스톱 0.15 는 체감 X → **레버가 틀렸다. SB 묵직함은 히트스톱이 아니라 TimeScale 0.2배 슬로모**(반격 전용, 판정보다 0.07초 먼저 시작). 다음 = `RequestSlowMo(0.2, 0.15, P10)` |
| 2026-08-31 | [SB언리얼페스트2024-전수판독](dev-logs/2026-08-31-SB언리얼페스트2024-전수판독.md) | ★34분 발표 **전 구간 프레임 전수 판독**(1080p 345장 + whisper large-v3 338세그). 기존 미확인 3건 전부 해결 — **Balanced Mode 2160p(FSR2) 50~60FPS** / 보스명 **엘더** / 본 분리 = **BreakConstraint + AddImpulse**. ★**히트스톱 0.07초·공격자만** (우리 0.08과 거의 동일) / **어제 결론 정정** — 평타 히트스톱은 있다, 단 Show 데이터가 아니라 코드에(TimeScale과 별개 2층 구조) / 판정 = **Triangle-Hitbox Intersection**(얇은 무기) vs **Hitbox Sweep Trace**(큰 무기, 정확도 낮음 — 우리가 이쪽) / 보간 = **UAnimSequence.RawAnimationData** + LookAt IK 부작용 2차 보정 / **최적화 목차 11개**(발표는 4개만) / ShowMaker UI 필드 전수 — **Available Guard·Super Parry** 는 우리가 모르던 항목 / Vertex Shake = 메테리얼 **WorldPositionOffset** |
| 2026-08-22 | [debug-console-player-init](dev-logs/2026-08-22-debug-console-player-init.md) | ★켜고 끄는 자리가 코드에 박혀 있던 것 4건. ① 온스크린 디버그 4종(**데미지·회피·접근·넉백**)이 `if (GEngine)` 만으로 항상 켜져 있었다 → **콘솔 변수 `KD.ShowDamage`/`ShowDodge`/`ShowApproach`/`ShowKnock`** (`ECVF_Cheat`). 체크박스로 안 뺀 이유 = 데미지·넉백은 GA 가 아니라 **어트리뷰트셋·Pawn 에서 찍는다**. ⚠️무기 궤적·조준선은 여전히 BP 체크박스. ② 플레이어 어트리뷰트 초기값이 **C++ 생성자에만** 있어 탄약 하나 바꾸는 데 재컴파일 → `GE_InitPlayerStats` 신설, `BP_PlayerState.StartupEffects` 에 등록(적용 루프는 `KDPlayerState.cpp:40` 기존 것). 적은 이미 DA 로 데이터화돼 있었는데 플레이어만 코드였다. ③ 마무리 5타로 끝내면 **적 등 뒤에 착지** — 워프 노티 `RotationType = Facing` 인데 `05_04` 는 적을 지나쳐 달리는 클립이라 반대로 돈다 → 마무리 5타 `warp_rotation` 해제(회전은 `OnActivated` 의 `SetActorRotation` 담당). ⚠️**`AM_SB_Parry_Counter_Attack_L` 하나만 `DEFAULT` 로 남음**(19개 중 유일, 미확인). ④ `BufferTimeWindow` 상한 0.5→0.8(기본값 0.2 유지). **촬영용 임시값 원복 목록 있음** — 밴딧 Poise 1→3 / 퍼펙트 패링 창 0.5→0.2. ⚠️노출을 `AEM_MANUAL` 로 바꾸면 화면이 검어진다(실사 주광 기준 계산) |
| 2026-08-21 | [aim-knockback-camera](dev-logs/2026-08-21-aim-knockback-camera.md) | ★무음 결함 3건 — 전부 **캡슐 기준으로 계산하는데 캡슐이 진실이 아닌** 문제. ① 조준 트레이스가 `ECC_Visibility` 하나뿐인데 **`Pawn`·`CharacterMesh` 프로파일이 그 채널을 무시**해서 조준점이 적 뒤 벽에 찍혔다(먼 벽은 맞고 가까운 적만 빗나감 = 시차) → Pawn 오브젝트 트레이스를 벽까지만 덧댐. ② 넉백 배수를 바꿔도 거리가 그대로 — **AI 경로추종이 `LaunchCharacter` 속도를 매 틱 덮어씀**(`StopMovement` 는 현재 요청만 취소). 넉백 구간만 `PauseLogic`, ⚠️`ResumeLogic` 은 이유별로 안 세므로 경직·사망 가드 필수. ③ 공중 콤보에서 캐릭터가 화면 위로 벗어남 — **루트 Z 가 0**(점프가 골반에만 그려짐). 캡슐을 띄우면 높이가 두 번 더해지므로 **스프링암이 골반 높이를 직접 읽어** 카메라 위치와 LookAt 을 같이 올림(둘 다 올려야 각도 유지). 문턱 180 = **클립 993개 실측**(해당 8개 / 공중 회피 최고 169 사이가 빔). ⚠️좌표계 = `RTS_Component`(액터 기준은 메시 오프셋 88 만큼 어긋남) / **파이썬은 bool 의 `b` 접두사를 뗀다**(`use_muzzle_origin`) / PIE 중 `load_asset` 은 조용히 `None` |
| 2026-08-20 | [approach-warp](dev-logs/2026-08-20-approach-warp.md) | ★콤보 진입기 접근 워프 — 반경 안 적에게 붙어서 콤보 시작. **모션 워핑은 위치를 만들지 않고 원래 루트모션을 늘린다**(워프창 이동량 20클립 실측) / 무음 결함 = **등록한 워프 좌표가 조기 리턴에 살아남아** 시체·옛 자리로 끌려감(`RemoveWarpTarget` 이 `if (!Target) return` 아래 있었다) / 위치 스냅샷 → `bFollowComponent` 추적으로 전환(멈출 거리는 `VectorFromTargetToOwner` 오프셋) — **그러면 회전 기준이 적 컴포넌트로 바뀌므로 노티를 `Facing` 으로 같이 바꿔야 한다** / **각도 필터가 둘인데 기준이 다르다**(콘=카메라 / 135도 게이트=몸) → 콘만 360으로 열어도 안 먹어서 게이트를 삭제 / **배율 상한을 수치로 잡으려던 게 틀렸다**(6배에서도 체감 멀쩡, 기준은 PIE) / `AutoAimRange` 가 `MaxApproachRange` 의 상한 / `export_text` 의 `LinkValue` 는 두 번 나온다(앞=끝, 뒤=시작) |
| 2026-08-19 | [combo-damage-scaling](dev-logs/2026-08-19-combo-damage-scaling.md) | ★타별 데미지 계수(`FComboNode.DamageMultiplier`, 곱하는 자리는 `GA_MeleeTraceBase.cpp:210` 하나) + 총격 조준 클램프 + 타격감 더미. **1타를 0.7로 낮추는 게 핵심**(마무리만 올리면 대비가 안 생김) / 공중은 별도 경로라 같은 쌍을 두 번 / **라이브 코딩 직후 파이썬으로 `USTRUCT` 배열을 쓰면 전 필드가 에러 없이 밀린다**(`DA_ComboTree` 22노드 파괴 → git 복구) / `EditDefaultsOnly` 는 파이썬 쓰기 차단 / **더미는 `AIControllerClass` 를 비우면 `PossessedBy` 가 안 불려 초기화가 통째로 안 된다** — `BehaviorTreeAsset` 만 비울 것 |
| 2026-08-19 | [enemy-melee-trace-tag](dev-logs/2026-08-19-enemy-melee-trace-tag.md) | ★적 근접 판정 부활 + 퍼펙트 패링 반복 발동 — **무음 결함 2건.** ① 무기 메시를 **컴포넌트 태그로** 찾는데 적은 `Weapon` / GA 는 `Sword` 였다(플레이어는 무기 2개라 `Sword`/`Gun` 으로 갈라야 해서 기본값이 그것). 태스크가 생성조차 안 돼 **디버그 궤적도 같이 죽었다** — 궤적이 "없는" 것은 반경 고장이 아니다. 적 베이스 생성자 1줄로 BP 7개 해결(CDO 재조회로 확인). 무음 게이트 3곳(`UE_LOG` 주석)이 진짜 비용. ② **`End Ability` 없는 BP GA 는 두 번째 활성화가 조용히 거부된다**(`InstancedPerActor`) — 퍼펙트 패링이 첫 한 번만 포이즈를 깎았다, 연출이 여러 겹이라 증상이 가려짐. 조회법 3개 = `GameplayTagLibrary.get_tag_name` / SCS 태그는 `<BP>.<클래스>:<이름>_GEN_VARIABLE` 직접 로드 / PIE 어트리뷰트는 `get_all_attributes`+`get_gameplay_attribute_value` |
| 2026-08-18 | [enemy-state-bar](dev-logs/2026-08-18-enemy-state-bar.md) | ★적 상태 바 구현 — **우리 첫 월드 스페이스 UI**. SB 스샷 보고 설계를 2번 뒤집음(상단 고정 → 머리 위 / HP 채움 빨강 → 흰색, 상단 중앙은 보스 전용) + Poise 를 카운트로(MaxPoise=칸 수, 코드 0줄) + 값 전달 = `WaitForAttributeChanged`(SB의 Master 폴링보다 한 단계 위). 함정 4개 — **BP 함수엔 Async 노드 못 놓음**(그래서 노드가 안 보였다) / **빈 `Event On Targeted` 가 C++ `_Implementation` 을 대체**해 바가 안 뜸 / 리페어런트 이름 충돌 / `Width Override` 가 부모 슬롯 `Fill` 에 끌려 늘어남 |
| 2026-08-18 | [shield-and-vitals](dev-logs/2026-08-18-shield-and-vitals.md) | ★실드 어트리뷰트(감소형, 상수 1개) + 좌하단 HP/ST/SH 3줄 + 탄약을 크로스헤어 아래로. 재생은 `State.Combat.InCombat` 을 Ignore 로 써서 **새 태그 0개**. 함정 4개 — **`ProgressBar` 는 색 노브가 2개고 곱해진다**(기본 파랑이 모든 색을 덮음, 적 포이즈도 노랑이 아니었다) / **`Height Override` 도 슬롯 `Fill` 에 끌려 늘어난다**(도트 두 번째 줄이 잘림) / UE5.3+ **GE 태그 조건은 `GEComponents` 안**(구버전 속성만 보면 오판) / `Wait` 의 `OldValue` 가 `MaxValue` 에 붙어 **첫 타격만 맞고 이후 바가 덜 준다** |
| 2026-08-18 | [player-hud](dev-logs/2026-08-18-player-hud.md) | ★플레이어 HUD 1단계 — 좌하단 HP 도트 바 + 탄약. `Dosul` 폐기하고 `Ammo/MaxAmmo` 로(참조 0개 확인) + 소모는 Cost GE 고정 `-1`(**SetByCaller 면 0발에도 발사된다** — `CheckCost` 가 값을 못 구함) + `WBP_MainHUD` 컨테이너에 크로스헤어 이사. 함정 = **`Wait for Attribute Changed` 의 `Changed` 를 실행 흐름으로 쓰면** 뒤 구독이 아예 생성 안 되고 갱신도 안 돈다(에러·로그 X). 재장전은 애니가 0개(993개 전수)라 **상태 태그로만** 구현 — 0발 차단 + 사격 후 2초 회복정지. UE5.3+ **GE 태그 조건은 컴포넌트에 있다**(안 붙이면 칸 자체가 없음) / **BP 가 C++ 의 `FGameplayTagContainer` 기본값을 안 물려받는다** |
| 2026-08-17 | [hud-enemy-state-bar-spec](dev-logs/2026-08-17-hud-enemy-state-bar-spec.md) | ★적 상태 바 HUD 설계(SB `WB_MainHUD_EnemyStateBar` 원본 복원 — 계층·치수·색) + 총구 이펙트 배선 25발. **GC 로는 총 메시 소켓 못 가리킴**(엔진 337행) → `ANS_WeaponTrail` 경로. Niagara 파라미터를 uasset `grep -a` 로 캐는 법. UI 소재 결론 = 텍스처 0장으로 시작 |
| 2026-08-16 | [melee-radius-and-character-fit](dev-logs/2026-08-16-melee-radius-and-character-fit.md) | ★근접 판정 반경 3→20 (28개 창 전수 측정 = 모션별 차등 근거 없음) + 발차기 축 0 결함 + **SB 본편 Eve 콜리전 원본 확보**(덤프에 없던 게 아니라 추출 누락) + 플레이어 메시가 지면에 3cm 파묻혀 있던 것 |
| 2026-08-14 | [air-combo-cancel-window](dev-logs/2026-08-14-air-combo-cancel-window.md) | ★공중 콤보 캔슬 윈도우를 안무 기준 프레임으로 (포즈 매칭 + `_All` 교차검증 = 차이 0) + `Air_02`·`_04` 검 재확인. `Air_01`은 캔슬창이 아예 없었다(07-30 문서가 계획만 적힌 건) / fps를 `(nf-1)/L`로 재면 틀린다 → `get_time_at_frame` / `GameplayTag`는 파이썬 repr이 항상 비어 보인다(`tag_name`까지 꺼낼 것) |
| 2026-08-13 | [auto-aim-and-gun-sound](dev-logs/2026-08-13-auto-aim-and-gun-sound.md) | ★락온 없이도 자동 조준(규칙을 거리→각도 최소로, SB 실측 ±90°/500cm) + 총격 판정 원점을 총구→캡슐 + **사운드 전면 배선**(총성 21발·발소리 488개·점프착지·공중 총격). 뼈 좌표로 접지 프레임 자동 검출 / 본 속도비로 총·검 판별(Air_01 = 4.75) / 문서가 틀렸던 것 3건 정정 / SoundCue Mixer 파이썬 생성이 재생 순간 에디터를 죽인 사고 / Concurrency 의 Prevent New 가 발소리를 죽인다 |
| 2026-08-12 | [input-component-and-muzzle-socket](dev-logs/2026-08-12-input-component-and-muzzle-socket.md) | ★`AKDPlayerCharacter` 700→426줄(입력 컴포넌트 분리) + GA의 구체 Pawn 캐스팅 4곳 제거(접근자 통일) + 총구를 무기 메시 소켓으로 이전. 결함 2건(어빌리티 태그 충돌로 콘 히트스캔 공짜 발동 / `ANS_EnemyAttackWindow` 태그 미초기화로 퍼펙트 회피 no-op) |
| 2026-08-11 | [weapon-sound-trail](dev-logs/2026-08-11-weapon-sound-trail.md) | ★검 사운드 49칸 배선(타격음은 Cue로 일원화 — 빗나가면 소리 X) + 트레일 27개가 안 보이던 원인(`Distortion_Only`=굴절 전용) + `ANS_WeaponTrail` 파라미터 TMap화. `Trail Width` 공백 하나에 조용히 실패 |
| 2026-08-10 | [gun-projectile](dev-logs/2026-08-10-gun-projectile.md) | ★총 3단계 — 발사체 결함 2건(무성 통과·자기 총알 퍼펙트회피) + 스폰 경로를 `KDAbilityStatics` 하나로. `Muzzle Socket` 무성 실패, CDO가 비어도 미설정이 아닌 사례 |

<details>
<summary><b>2026-07-30 이전 35개 펼치기</b> - 길동/버터 시절 포함. 그날의 기록이라 현행과 다를 수 있다</summary>

| 날짜 | 문서 | 내용 |
| --- | --- | --- |
| 2026-08-08 | [aim-offset](dev-logs/2026-08-08-aim-offset.md) | ★조준 상체 오프셋 — 레일 진행도를 -1~+1로 정규화(SB 실측), Mesh Space 판단 근거, 감도 배율이 필요했던 이유(134도 대 42도) |
| 2026-08-08 | [gun-fire-crosshair](dev-logs/2026-08-08-gun-fire-crosshair.md) | ★총 2단계 — 사격 GA(상속 대신 태그 조건표), 해제 몽타주는 태스크로 못 켠다, 첫 화면 HUD, `Set Visibility(self)`가 Tick을 죽인 함정, SB 발사 확산 커브 실측 |
| 2026-08-05 | [lockon-pitch-curve](dev-logs/2026-08-05-lockon-pitch-curve.md) | ★락온 시점 고정 버그 — 레일 눈금이 된 피치에 락온이 시선각도를 넣어 한 점으로 수렴. SB식 거리별 피치 커브 + 각도별 좌우 속도 커브 + 사거리 1700 |
| 2026-08-04 | [hitstop-component](dev-logs/2026-08-04-hitstop-component.md) | ★히트스톱을 액터 소유 컴포넌트로 — 정지가 노티를 끊어 데미지·판정이 중복되던 버그. CustomTimeDilation=0만 유효 |
| 2026-08-03 | [camera-rail-look-rotation](dev-logs/2026-08-03-camera-rail-look-rotation.md) | ★카메라 레일 2단계 완료 — 조준을 랙 전 이상 위치로 (버그 3개가 한 뿌리) + 마우스 감도 노출. PIE 6항목 통과 |
| 2026-07-30 | [gildong-montage-migration](dev-logs/2026-07-30-gildong-montage-migration.md) | ★길동 몽타주 전면 탈출 (단발·방어·회피·피격·공중) + GA 계층 리팩토링 + SB 공중 콤보 실측 |
| 2026-07-30 | [inaction-tag-system](dev-logs/2026-07-30-inaction-tag-system.md) | ★InAction 우산 태그 (가드·회피도 전투 진입+무기 부착) + 회피 캔슬 목록 통합 + ABP 슬롯 재배치 |
| 2026-07-29 | [melee-trace-static-mesh-fix](dev-logs/2026-07-29-melee-trace-static-mesh-fix.md) | ★근접 판정 부활 (무기가 StaticMesh라 코드가 배제) + 지상 콤보 20개 노티 일괄 배치 |
| 2026-07-28 | [gunsword-hit-sequence](dev-logs/2026-07-28-gunsword-hit-sequence.md) | ★타격 시퀀스 실측 (뼈 속도 봉우리) = ANS_MeleeTrace 배치표 + 총 전용 3타 발견 |
| 2026-07-27 | [gunsword-combo-cancel-timing](dev-logs/2026-07-27-gunsword-combo-cancel-timing.md) | ★콤보 캔슬 타이밍 실측 (포즈 매칭으로 안무가 의도 역추출) + 루트모션 이동량 |
| 2026-07-23 | [gunsword-equip-montage-loco-gating](dev-logs/2026-07-23-gunsword-equip-montage-loco-gating.md) | 발검/납검 속도대별 상체 몽타주 + 로코모션 Stop 입력 게이팅 |
| 2026-07-22 | [gunsword-holster-socket](dev-logs/2026-07-22-gunsword-holster-socket.md) | 검+총 홀스터 소켓 셋업 + DA 교차오염 정정 (Holder 본 붕괴 우회, 발검/납검·PIE는 다음) |
| 2026-07-17 | [melee-trace-refactor](dev-logs/2026-07-17-melee-trace-refactor.md) | 트레이스 무기중립화 (WeaponTrace→MeleeTrace + ETraceMeshSource + 창단위 히트리셋) |
| 2026-07-09 | [motion-matching-research](dev-logs/2026-07-09-motion-matching-research.md) | 모션매칭 자료조사 (개념+에셋 구조+URL 모음) |
| 2026-06-13 | [execution](dev-logs/2026-06-13-execution.md) | 처형 시스템 |
| 2026-06-12 | [combat-tag-polish](dev-logs/2026-06-12-combat-tag-polish.md) | 전투 태그 폴리싱 |
| 2026-06-12 | [player-refactor](dev-logs/2026-06-12-player-refactor.md) | 플레이어 리팩토링 패스 |
| 2026-06-11 | [air-combo](dev-logs/2026-06-11-air-combo.md) | 공중 콤보 |
| 2026-06-11 | [counter-camera-juice](dev-logs/2026-06-11-counter-camera-juice.md) | 카운터 카메라 juice |
| 2026-06-08 | [lockon-fixes](dev-logs/2026-06-08-lockon-fixes.md) | 락온 버그 수정 |
| 2026-06-04 | [perfect-parry-juice](dev-logs/2026-06-04-perfect-parry-juice.md) | 퍼펙트 패링 juice 5겹 |
| 2026-06-03 | [dodge-seam-rootmotion-mode](dev-logs/2026-06-03-dodge-seam-rootmotion-mode.md) | 닷지 이음새/루트모션 모드 |
| 2026-06-02 | [double-jump](dev-logs/2026-06-02-double-jump.md) | 더블 점프 |
| 2026-06-02 | [jump-landing-pivot-turn](dev-logs/2026-06-02-jump-landing-pivot-turn.md) | 점프 착지/피벗 턴 |
| 2026-06-02 | [run-to-stop](dev-logs/2026-06-02-run-to-stop.md) | 런투스톱 |
| 2026-05-30 | [player-locomotion](dev-logs/2026-05-30-player-locomotion.md) | 플레이어 로코모션 (BlendSpace) |
| 2026-05-28 | [camera-sb-curve-system](dev-logs/2026-05-28-camera-sb-curve-system.md) | SB식 커브 카메라 |
| 2026-05-27 | [movement-cancel-notify](dev-logs/2026-05-27-movement-cancel-notify.md) | 이동 캔슬 노티파이 |
| 2026-05-26 | [perfect-parry-cue-migration](dev-logs/2026-05-26-perfect-parry-cue-migration.md) | 퍼펙트 패링 GameplayCue 이관 |
| 2026-05-25 | [combo-system-day3](dev-logs/2026-05-25-combo-system-day3.md) | 콤보 Day3 — 분기 완성 + PIE 검증 |
| 2026-05-24 | [combo-system-day2](dev-logs/2026-05-24-combo-system-day2.md) | 콤보 Day2 — GA_Light/Heavy |
| 2026-05-23 | [combo-system-day1](dev-logs/2026-05-23-combo-system-day1.md) | 콤보 Day1 — 구조 + ComboComponent |
| 2026-05-22 | [attribute-restructure](dev-logs/2026-05-22-attribute-restructure.md) | AttributeSet 재구성 |
| 2026-05-22 | [combo-scope-change](dev-logs/2026-05-22-combo-scope-change.md) | 콤보 스코프 6→9분기 변경 |
| 2026-05-22 | [gas-iframe-parry-research](dev-logs/2026-05-22-gas-iframe-parry-research.md) | GAS i-frame/패링 자료조사 |

</details>

## handoffs/ — 인수인계

| 문서                                                                                                     | 설명                                                                    |
| -------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------- |
| [**CURRENT**](handoffs/CURRENT.md) | ★★ **현재 상태 (233행).** 세션 시작 시 여기부터. 현행만 두고, 절이 쌓이면 월 단위로 `archive/` 로 덜어낸다 |
| [2026-07-21-pivot-gunsword](handoffs/2026-07-21-pivot-gunsword.md)                                       | ★프로젝트 피벗 — 버터 맨손 폐기 → 9CG Gun&Sword 팩 기반 SB 스타일 액션  |
| [archive/2026-07~08-past-sessions](handoffs/archive/2026-07~08-past-sessions.md) | 🗄️ 지난 세션 기록 834행 — 2026-08-24 에 `2026-07-30-parallel-sessions.md`(1,052행)에서 분리. **여기 적힌 "다음 할 일"은 전부 끝났거나 뒤집혔다.** 설계 근거로만 읽을 것 |
| [archive/2026-07-06-claudian-env-setup](handoffs/archive/2026-07-06-claudian-env-setup.md) | 🗄️ Claudian 환경 셋업 인수인계 |

## archive/ — ⚠️ 폐기 라인 (기본은 읽지 않음)

현재 프로젝트의 진실이 **아니다.** 설계 근거로 쓰지 말 것. 상세 = [archive/README](archive/README.md)

| 폴더 | 내용 |
| --- | --- |
| [butter/](archive/README.md) | 버터 라인 5개 — 2026-07-21 피벗으로 폐기. 🗑️`버터맵_작업기록_통합`(153KB/2381줄)은 **2026-08-24 삭제** — 볼트·메모리에 전문 보존 |
| [kildong/](archive/README.md) | 길동 중단 백로그 2개 |
| team_setup_guide.md | 구 페어 시절 셋업 (P4 기술 stale) |
