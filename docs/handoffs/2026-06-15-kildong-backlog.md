# [길동 보류] 대기 작업 백로그

> 길동 → 버터 전환(2026-06-16)으로 보류된 미착수 작업 2건 + 폴리싱 1건 (2026-07-09 플랜 폴더에서 이관).
> 처형 Contextual Anim 도입(진행 중이던 것)은 별도 문서: [2026-06-15-kildong-execution-contextual-anim](2026-06-15-kildong-execution-contextual-anim.md)

## 1. 무리의 마지막 적 처치 → Time Dilation (설계 완료, 미착수)

**"마지막" 정의**: 맵 전체 생존 적 → 0, **단 처치 직전 생존 ≥2였을 때만** (너무 자주 터짐 방지 게이트).

**설계 (디커플링)**: 적 스폰 시 자기 등록 → `OnDeath` 시 해제 → 명단 0 되는 순간(직전 ≥2) GameplayCue/Message 1발 → 슬로우모션 BP. 직접 참조 0.

| 구분 | 경로 | 클래스 | 책임 |
|---|---|---|---|
| 신규 ★§0 | `Enemy/AI/EnemyRegistrySubsystem.{h,cpp}` | `UEnemyRegistrySubsystem : UWorldSubsystem` | 생존 적 명단 add/remove + 0 도달 시 큐/메시지 1발 |
| 수정 | `KDGameplayTags.{h,cpp}` | — | `GameplayCue.Combat.LastEnemyKill` 신규 |
| 수정 | `KDEnemyBaseCharacter.cpp` | — | BeginPlay 등록 1줄, `HandleDeath` 해제 1줄 |

> `EncounterSubsystem`과 분리 (토큰=공격 조율 / 레지스트리=생존 카운트). 슬로우 자체는 패링 BP 패턴 재탕(C++ 0).

**구현 전 결정 필요**: 슬로우 시간/배율(패링 0.3/0.2s 재사용?) / "마른 슬로우" 방지 레이어(PP 흑백/카메라/래그돌 푸시 어디까지) / ≥2 게이트 외 추가 조건(보스 제외 등).

**검증**: ①적 2명+ 마지막 처치 순간 슬로우 ②1명만 처치 시 안 뜸 ③화면효과 겹침 ④정상 속도 복귀.

## 2. Light 빠른 연타 휘젓기 폴리싱 (보류 — PIE 거슬릴 때)

- 지상 공격 몽타주 `ANS_CancelWindow` 시작을 `ANS_WeaponTrace` End +4~5프레임 뒤(회복 구간)로 이동 → 휘두름 항상 완주, 캔슬은 회복만 절단 (몽타주 = 사람 영역)
- 보조: 플레이어 BP `ComboComponent.MaxTempoMultiplier` 1.3 → 1.15~1.2

## 3. 새 기능 착수 시 공통 점검 체크리스트 (처형 작업에서 도출)

- **A 공간/정렬**: 벽·장애물 옆(워프 끼임) / 경사·계단(Z) / 캐릭터 키 차 / 적 이동 중 velocity 잔류
- **B 타이밍/상태**: 진행 중 외부 사망(연출 끊김 처리) / 이중 발화 가드 / 태그 제거 누락
- **C 카메라**: 시네 카메라 벽 뚫음 / 락온 카메라 충돌 / 연속 발동 큐 중첩
- **D 충돌**: 캡슐 끄는 동안 바닥 빠짐 / 제3의 적 통과
- **E 애님**: 듀엣 몽타주 길이 불일치 / 루트모션↔워프 충돌
- **F 멀티(나중)**: 서버-클라 동기
