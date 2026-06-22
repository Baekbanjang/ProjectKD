# Damage System — Project_KD

> 단독/페어 결정 기록 (CLAUDE.md §5-5). 변경은 페어 합의 후.
> 정교한 액션 게임(스텔라 블레이드 레퍼런스) 기준. 로그라이크식 무거운 ExecCalc 중앙화는 채택 안 함.

## 핵심 원칙 — 깊이 축은 "숫자"가 아니라 "타이밍·공간·읽기"

로그라이크는 데미지 *숫자 연산*(크리·계수·스택·저항)이 깊이라 ExecCalc 중앙화가 맞았다.
이 프로젝트의 깊이는 타이밍/간격/전조 읽기 → 데미지 숫자는 보통 공격마다 거의 고정.
복잡도 예산은 **리액션·게이트·손맛**에 쓴다.

## 결정사항 (2026-05-22 페어 confirm)

1. **기본 데미지 = `SetByCaller`** (`SetByCaller_AttackPower`). 인라인/직접 Health 차감 ❌. ExecCalc ❌.
2. **ExecCalc는 단 하나의 경우만** — "타겟의 살아있는 어트리뷰트를 적중 시점에 캡처해 원자적으로 조합"이 필요할 때 (예: 속성 저항 경감). 그 시스템이 실제로 생길 때 도입(YAGNI). 지금은 없음.
3. **`IncomingDamage` 메타 어트리뷰트 = 필수 관문** — 배치 **`AS_Combat`** (AttackPower/Defense와 "전투 수식" 묶음). Transient·비복제·Init 안 함.
4. **`PostGameplayEffectExecute`에서 일괄 처리** — IncomingDamage 소진 → Defense 경감 → Health 차감/클램프 → (포이즈 차감) → Health 0이면 캐릭터의 Health 델리게이트가 사망 처리. AS는 데이터만 (§1-3).
5. **데미지 숫자 표기 X** — 플로팅 데미지 텍스트 없음. HUD가 Health/MaxHealth 어트리뷰트에 바인딩 → 체력바 감소로 피드백. "맞췄다"는 GameplayCue(임팩트) + HitFeedbackComponent(본 셰이크) + 히트스톱(차주).
6. **상태이상(감전 스턴/둔화/화상 표식) = 데미지와 분리** — 태그 부여 GE (`State.Debuff.*`) + 어빌리티 차단/무브먼트 모디파이어로 해결. 데미지 공식과 무관.

## 데미지 파이프라인 (소스 불문 단일 관문)

```
데미지 GE (SetByCaller 공격력)  →  IncomingDamage(메타, AS_Combat)
                                        ↓ PostGameplayEffectExecute
            LocalDamage = GetIncomingDamage(); SetIncomingDamage(0)
            mitigated   = 경감(Defense)
            SetHealth(Health - mitigated)  // PreAttributeChange가 0~Max 클램프
            (포이즈 차감 등 후처리)
                                        ↓ Health 0
            캐릭터 Health-change 델리게이트 → HandleDeath (AKDEnemyBaseCharacter)
```

- **즉발 타격** = Instant GE → 관문 1회
- **화염 도트 / 독장판** = Duration/Infinite + **Period** GE → 매 틱이 관문 재통과 (PostGEExec가 틱마다 호출). 장판 = 진입 시 GE 적용, 이탈 시 제거
- **원거리(활/총)** = 투사체 액터(예약된 `Projectile` 채널) 또는 히트스캔. 투사체가 GE 운반. 데미지는 같은 관문
- **데미지 타입(불/독/감전)** = GE Context의 태그(`Damage.Type.*`). 용도: GameplayCue 선택 / (미래)저항 매칭 / 면역. 별도 코드 경로 아님

## 탐지(detection)와 적용(application) 분리

`GA_WeaponTraceBase`는 "소켓 트레이스 근접" 전용(탐지). 데미지 적용부(GE 적용 + AttackPower + OnTargetHit)는 공통.
비-무기 적(맨손/광역/원거리)이 오면: 탐지 task만 교체(`OnHit(FHitResult)` 시그니처 동일 유지), 적용부 재사용. 2번째 비-무기 적 등장 시 적용부를 공통 헬퍼/상위 base로 추출(2회 규칙). 지금은 장검뿐 → 추출 안 함.

## 미결 / 차주

- `IncomingDamage` 메타 + PostGEExec 구현 (AS_Combat) — 결정 완료, 구현 대기
- JustDodge/패링 윈도우 소비 (적 공격 전조 → 플레이어 방어 판정)
- 속성 저항 시스템 (생기면 PostGEExec 또는 작은 ExecCalc에 타입태그 경감)
