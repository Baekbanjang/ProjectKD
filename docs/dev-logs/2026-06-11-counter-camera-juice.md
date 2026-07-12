# 타격 연출 묶음 — 카메라 셰이크 / 히트스탑 / 카운터 슬로우+FOV펀치 / 카메라 트레일 — 2026-06-11~12

## 1. 카메라 셰이크 + 히트스탑 (2026-06-11) — C++ 0
- **히트스탑 = 이미 구현됨** (`GA_WeaponTraceBase::ApplyHitStop` — Montage_Pause→타이머→Montage_Resume). BP 기본값 공격자 **0.08s** / 피격자 **0.12s**.
- **카메라 셰이크 통로 = 이미 깔림**: `HitConfirmProfile.CameraShakeClass` 필드. `OnTargetHit`이 착탄마다 `GameplayCue.Combat.PlayerHitConfirm` 큐 발화(`SourceObject=HitConfirmProfile`).
- BP 작업만: `GCN_PlayerHitConfirm`에서 SourceObject→Cast→CameraShakeClass→ClientStartCameraShake. → VFX+CameraShake+Sound 배선 확인 완료/검증.
- ⚠️ 0.01s 타임스톱은 서브프레임(1프레임=0.0167s)이라 인지 불가 → 안 만듦.

## 2. 퍼펙트 닷지 카운터 연출 — 슬로우 + FOV펀치 (2026-06-11)
레퍼런스 = SB 역조(Reflection). 동작은 이미 `GA_CounterThrust`로 구현됨 → 이번엔 juice만.
- **트리거 = AnimNotify** (발동 순간 X, "거의 닿을 때" O): 카운터 몽타주 착탄 직전 프레임에 `AN_PlayerCue` 노티 → `CueTag=GameplayCue.Combat.CounterThrust` → `GCN_CounterThrust`(액터형, Delay 기반 슬로우).
- **슬로우** = `GlobalTimeDilation 0.3` → Delay(Real Time) → 1.0 (퍼펙트 패링 패턴 재탕, C++ 0).
- **FOV 펀치** = `KDPlayerCameraManager`에 `FovPunchCurve` + `TriggerFovPunch(Scale)` + `UpdateViewTarget`에서 전이 델타 가산.
  - 커브 = 시간(X)→FOV 변화량(Y), 음수=줌인. `TriggerFovPunch`가 스톱워치 켜기(`FovPunchElapsed=0`), 매 프레임 커브 평가→델타 가산→MaxT 지나면 OFF.
  - **왜 셰이크 FOV 아닌 매니저 내부 가산**: 매니저가 `Super`(셰이크) 뒤에 FOV를 거리커브로 덮어써서 셰이크 FOV 채널이 묻힘.
  - **커브 다중화 결정 = 단일 커브 유지**(YAGNI, 연출 1종). 둘째 연출 생기면 `TriggerFovPunch(UCurveFloat*, Scale)`로 B안 리팩터.
- 신규 클래스: `UAN_PlayerCue`(`AbilitySystem/AnimNotifies/`, `UAnimNotify` 상속) — CueTag를 owner ASC에 ExecuteGameplayCue. **범용 재사용**.
- 태그: `GameplayCue.Combat.CounterThrust`.
- 검증: PIE 작동 ("아 된다잉"). 슬로우+FOV펀치 자연스러움 확인.

## 3. 카메라 지연+따라잡기 트레일 (A안, 2026-06-12) — C++ 태그 1개
SB 역조식: 플레이어 먼저 대시 → 카메라 뒤처짐(트레일) → 따라잡기 → 평소 거리 복귀.
- **메커니즘**: SpringArm `CameraBoom`의 `CameraLag`을 대시 시작에 켬(LagSpeed 낮춤 3~4) → 카운터 끝에 끔. 스프링 물리가 트레일+catch-up 자동 처리, 위치 계산 불필요.
- **트리거**: 카운터 몽타주 대시 시작 프레임에 `AN_PlayerCue` 노티 → 새 큐 `GameplayCue.Camera.DashTrail`(착탄 FOV펀치와 다른 타이밍) → `GCN_CounterTrail`(액터형).
- BP: GCN 커스텀 이벤트로 Delay 분리 (GameplayCueNotify_Actor 핸들러는 bool 반환 함수라 Delay 불가 → 커스텀 이벤트로 우회). Camera Boom 핀에서 `Set Enable Camera Lag`/`Set Camera Lag Speed`.
- 태그: `GameplayCue.Camera.DashTrail`. C++ = 태그 선언/정의 1줄씩만.
- 복귀 스냅 거슬리면 → B안(`KDPlayerCharacter::TriggerDashCameraTrail` C++ 타이머 보간)으로 승격 후보.
