# 전투 폴리싱 — 공중 공격 차단 + 휘젓기 + 어빌리티 태그 감사 — 2026-06-12

## 문제 1 — 공중에서 지상 공격 누수
공중 우클릭(Heavy)이 지상 강공으로 샘 (`TryHeavyAttack`엔 공중 가드 없었음).
- **결정 = 태그 방식**: `TryActivateAbilitiesByTag`가 ActivationBlockedTags를 자동 검사 → BP 태그로 차단, 코드 손검사 불필요.
- **신규 태그 `State.Movement.InAir`** + `KDPlayerCharacter::OnMovementModeChanged` 오버라이드에서 `IsFalling()` → AddLooseGameplayTag / 아니면 Remove. (UE는 falling을 게임플레이 태그로 자동 노출 안 함 → 코드가 토글해야 BP가 동작)
- **중복 코드 제거**: `TryLightAttack` 공중 분기에서 `AirComboLock` 손검사 삭제(`IsFalling()`만), `Tick` `bAirLocked` 게이트 삭제. `TryHeavyAttack`은 무변경(BP 태그가 차단 가져감).
- BP Class Defaults > ActivationBlockedTags: `GA_HeavyAttack`(★핵심)/`GA_SprintAttack`/`GA_LightAttack`에 `State.Movement.InAir`. `GA_AirLightAttack`은 InAir **Required**(공중에서만).

## 문제 2 — 빠른 연타 휘젓기 (어빌리티 문제로 판명)
진짜 원인 = Light↔Heavy가 서로 다른 어빌리티라 캔슬윈도우 안 거치고 즉발 전환.
- **해결 = `GA_LightAttack`/`GA_HeavyAttack` ActivationBlockedTags에 `State.Combat.Attacking` 추가** → 공격 중 새 공격 직접발동 차단 → 버퍼→캔슬윈도우 경로로만 전환. PIE "해결됐다잉".
- 닷지=이미 동일 패턴. 패링 방어캔슬 난이도 = 보류.
- ⚠️ **금지**: `State.Combat.Attacking`을 Light/Heavy에 넣되, AirCombo엔 넣지 말 것(콤보 연결이 캔슬-재발동이라 타이밍 깨짐). 공격끼리 차단은 캔슬 코드가 담당.

## 어빌리티 태그 감사 (Required / Blocked)
이번 실제 적용 = 공중 차단(InAir) 3종 + Required 2종(AirCombo=InAir / CounterThrust=CounterReady).
- **보류**: `State.Dead`(플레이어 사망 시스템 없음 → 나중 일괄), `State.Camera.Cinematic`(컷신 시스템이 태그 안 켬 → 나중).

## 남은 폴리싱 (보류)
- Light 빠른 연타 휘젓기 = 별개. 지상 공격 몽타주 `ANS_CancelWindow`를 `ANS_WeaponTrace` End +4~5프레임 뒤(회복 구간)로 이동 → 휘두름 완주, 캔슬은 회복만 절단. (몽타주=사람 영역, PIE 거슬릴 때)
- 보조: 플레이어 BP `ComboComponent.MaxTempoMultiplier` 1.3→1.15~1.2.

## 검증 (PIE) — 통과
공중 우/좌클릭 지상공격 안 나감 / 착지 후 정상 / 빠른 연타 휘두름 완주(휘젓기 없음).
