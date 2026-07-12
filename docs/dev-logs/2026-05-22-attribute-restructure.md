# 2026-05-22 — Attribute Restructure (W1 우선순위 2)

## 목적
W1 어트리뷰트 슬롯 작업. 도술 자원 추가 + 어트리뷰트 분포 재정의 + 네이밍 통일.

## 결과 어트리뷰트 분포

```
UAS_CharacterBase    (전 캐릭터 — 플레이어/적/보스 공통)
├─ Health / MaxHealth      100 / 100
└─ Poise / MaxPoise        100 / 100   ← 신규 (균형 게이지, Stagger 시스템)

UAS_Combat           (전 캐릭터 — 플레이어/적/보스 공통)
├─ AttackPower             20
└─ Defense                  0           ← 신규 (방어력, 데미지 차감)

UAS_Player           (플레이어 전용, UAS_CharacterBase 상속)
├─ Stamina / MaxStamina    100 / 100   ← base에서 이동 (회피·강공 자원)
└─ Dosul / MaxDosul          0 / 100   ← 신규 (도술 자원, 이전명 Doul)
```

PlayerState 보유:
- `PlayerAttributes` (`UAS_Player`) — 상속을 통해 Health/Poise도 포함
- `CombatAttributes` (`UAS_Combat`) — AttackPower/Defense

EnemyState (향후):
- `CharacterAttributes` (`UAS_CharacterBase`) — Health/Poise
- `CombatAttributes` (`UAS_Combat`) — AttackPower/Defense

## 핵심 결정

| 결정 | 사유 |
|------|------|
| **Stamina를 base → UAS_Player로 이동** | 기획서상 적은 회피 게이지 없음 (고정 모션/쿨다운). base는 진짜 공통만. |
| **Poise/MaxPoise를 base에 추가** | 적/보스 Stagger 시스템 공통, 플레이어도 보유 (광폭화 등 영향). |
| **Defense를 Combat에 추가** | 데미지 차감용. 적도 방어력 보유. 음수 방지 클램프만. |
| **Doul → Dosul 네이밍** | 도술 발음에 더 맞음. 한국어 "도력"은 유지. |
| **상속 유지 + PlayerState는 PlayerAttributes만 등록** | 옵션 A. 적/보스 EnemyState만 CharacterAttributes 직접 사용. PlayerState에서 중복 등록 방지. |
| **Dosul 초기값 0, Max 100** | spec 1_도술 §자원: 첫 교전 후 활성화. 0~100% 게이지. |
| **Defense 초기값 0** | 4_밸런싱 수치 미정 — GE_Init 또는 데이터로 차등 예정. |

## 변경 파일

| 파일 | 변경 |
|------|------|
| `AS_CharacterBase.h/.cpp` | Stamina/MaxStamina 제거 → Poise/MaxPoise 추가. PreAttributeChange Poise 클램프. |
| `AS_Combat.h/.cpp` | Defense 추가 + 음수 방지 클램프. PreAttributeChange 오버라이드 신규. |
| `AS_Player.h/.cpp` | 신규 클래스. Stamina/MaxStamina/Dosul/MaxDosul. UAS_CharacterBase 상속. |
| `KDPlayerState.h/.cpp` | `CharacterAttributes` 멤버 제거, `PlayerAttributes`/`CombatAttributes`만 유지. |
| `KDGameplayTags.h/.cpp` | `SetByCaller_Dosul` 태그 신규. |

## 클램프 로직

| 어트리뷰트 | 위치 | 범위 |
|-----------|------|------|
| Health | `UAS_CharacterBase::PreAttributeChange` | 0 ~ MaxHealth |
| Poise | `UAS_CharacterBase::PreAttributeChange` | 0 ~ MaxPoise |
| Defense | `UAS_Combat::PreAttributeChange` | ≥ 0 (음수 방지만) |
| Stamina | `UAS_Player::PreAttributeChange` | 0 ~ MaxStamina |
| Dosul | `UAS_Player::PreAttributeChange` | 0 ~ MaxDosul |

## 검증

✅ 빌드 통과 (UE5.6, Live Coding 후 풀 빌드)
✅ PIE `showdebug abilitysystem` — 8개 어트리뷰트 모두 한 줄씩 정상 출력 (중복 없음)
  - Health 100 / MaxHealth 100 / Poise 100 / MaxPoise 100
  - AttackPower 20 / Defense 0
  - Stamina 100 / MaxStamina 100 / Dosul 0 / MaxDosul 100
✅ ASC 자동 등록 (CreateDefaultSubobject → InitAbilityActorInfo)

## 미적용 (별도 작업)

- **Replication** (`UPROPERTY(Replicated)` + `OnRep_*` + `GAMEPLAYATTRIBUTE_REPNOTIFY`) — 멀티 확정 시점
- **D1 충전 GE** — 패링/적중 시 Dosul +N. 도술 GA 작업 단계
- **소모 GE** (`GE_Dosul_Cost`) — 화부/풍보/정승 GA 작업 시
- **4_밸런싱 수치** (Dosul 충전·소모량, Defense 값) — 밸런싱 문서 후
- **UI 슬롯 빨강 깜빡** (U-3 / Dosul 부족) — Player UI 작업 시
- **기획서 patch** (`Doul` → `Dosul`, AttributeSet 분포 표기) — 승환이 직접 진행 예정

## 페어 동기화

- `UAS_CharacterBase`, `UAS_Combat`은 base 클래스라 적/보스 영역 영향 가능
- 변경 사항(Poise 추가, Defense 추가, Stamina 제거) 데일리 싱크 시 필규 알림 필요

## 워크플로우 변화

이번 세션에서 메모리에 박은 새 룰:
- **AI는 코드 출력만**, 실제 .h/.cpp 수정은 승환이 직접 적용
- **P4 공유 MD 파일**은 명시적 요청 없이 절대 수정 X (기획/스펙 patch는 승환이 직접)
- 개인 영역(글로벌 CLAUDE.md, `.omc/dev-logs/` 등)만 AI가 자유롭게 수정
