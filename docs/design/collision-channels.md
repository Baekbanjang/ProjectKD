# Collision Channels — Project_KD

> F2 #3 셋업. 단독 결정 기록 (CLAUDE.md §5-5) + 페어 리뷰용. 변경은 페어 합의 후.
> 정의 위치: `Config/DefaultEngine.ini` → `[/Script/Engine.CollisionProfile]`

## 핵심 원칙 — 콜리전은 "무엇을 맞히나"만

공격 *출처*(무기/스킬/적/보스)는 콜리전 채널이 **아니다**. 콜리전이 답하는 질문은
"이 쿼리가 무엇에 맞는가"지 "누가 때렸나"가 아니다.

| 관심사 | 담당 | 채널? |
|---|---|---|
| 무엇을 맞히나 (Pawn / 항아리) | ObjectType 쿼리 | ✅ |
| 아군 / 적 판정 | GAS 팀 affiliation | ❌ |
| 데미지 출처 (무기/스킬/보스) | GE Context | ❌ |
| 벽 차단 (LoS) | 엔진 기본 WorldStatic | ❌ (커스텀 아님) |

## 정의한 커스텀 채널 (2개)

엔진에 없던 것만. `Pawn`, `WorldStatic`(=벽/Environment)은 엔진 기본 → 정의 안 함.

| Slot | Name | Type | 용도 |
|---|---|---|---|
| `ECC_GameTraceChannel1` | **Projectile** | Object | 스킬/원거리 투사체. **예약** — 스킬 들어올 때까지 미사용 |
| `ECC_GameTraceChannel2` | **Destructible** | Object | 항아리 등 파괴 오브젝트 |

프로파일 2개(`Destructible`, `Projectile`)도 함께 정의 — BP 드롭다운 원클릭 셋업용.

## "WeaponTrace"는 채널이 아니다 ★

원래 작업목록엔 "WeaponTrace ObjectType"로 적혔으나, 무기 트레이스는 *월드에 존재하는 것*이
아니라 *쿼리*다. `AT_WeaponTrace.cpp`는 `*ByObjectType` 쿼리를 쓴다(트레이스 채널 response가
아니라 ObjectType으로 직접 판정). 따라서 별도 WeaponTrace 채널을 만들지 않고,
**무기 트레이스가 ObjectType `{Pawn, Destructible}`를 직접 쿼리**한다.

## 결정사항 (페어 confirm 완료)

1. **채널 철학**: 최소 ObjectType + GAS 팀필터 (출처별 분리 채널 ❌)
2. **스킬/투사체**: Projectile 채널 미리 정의(예약), 사용은 스킬 마일스톤에서
3. **벽 차단**: 근접 trace가 벽(WorldStatic)에 막힘 — LoS 체크 필요

## 코드 후속 작업 (.cpp만, .h 시그니처 불변)

채널 정의만으로는 동작 안 함 → 와이어링 완료 (2026-05-22, 빌드 exit 0):

- [x] **Destructible 쿼리 추가** — `AT_WeaponTrace.cpp:81` `ObjectParams.AddObjectTypesToQuery(ECC_GameTraceChannel2)`
- [x] **LoS 벽 차단** — `AT_WeaponTrace.cpp:85` `IsWallBlocking` 람다(`LineTraceTestByChannel(ECC_Visibility)`),
      양 브랜치(TipLine/Sweep) per-actor hit에서 벽 막히면 skip
- [x] **TipLine→Sweep** — `GA_WeaponTraceBase.cpp:17` 생성자 기본값 Sweep (점착 whiff 해결)

> `ECC_GameTraceChannel2` 하드코딩은 취약 → 무기 종류 늘면 named 상수/`#define`로 감쌀 것.
> `GA_WeaponTraceBase.h` 초기화자는 아직 `= TipLine` (생성자가 Sweep로 덮음). 단일 진실 원하면
> 오너가 .h 초기화자도 Sweep로 동기화 (1줄, 오너 판단).

## 검증 (F2 #3 게이트)

- [ ] 에디터 Project Settings → Collision에 Projectile/Destructible 채널 + 프로파일 노출 확인
- [ ] 항아리 BP에 Destructible 프로파일 적용 → 장검 GA로 타격 시 OnHit 발동
- [ ] 벽 뒤 적은 장검에 안 맞음 (LoS)
- [ ] **캐릭터 캡슐 Visibility 응답 = Ignore 확인** — LoS 체크가 `ECC_Visibility`를 쓰므로,
      캡슐이 Visibility=Block이면 *벽 없이도* 정상 타격이 false-skip됨. 표준 "Pawn"
      프로파일은 Visibility=Ignore라 기본 OK (모션매칭/GASP 폐기로 커스텀 캡슐 리스크 해소).
      문제 시 LoS 트레이스에 타겟 액터를 ignore 추가하는 hardening 옵션 있음.
