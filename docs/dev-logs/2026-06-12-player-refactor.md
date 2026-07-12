# 플레이어 코드 리팩토링 패스 — SprintComponent 추출 + 입력 헬퍼 + 디버그 정리 — 2026-06-12

## Context
타격감/전투 폴리싱 일단락 후, 다음 큰 기능(처형) 전에 플레이어 코드 정리. `KDPlayerCharacter.cpp` 419줄 책임 과밀.

## ① SprintComponent 추출 (신규)
| 클래스 | 경로 | 상속 | 책임 |
|---|---|---|---|
| `USprintComponent` | `Movement/SprintComponent.{h,cpp}` | `UActorComponent` | 스프린트/워크 상태 + 속도 보간(타이머) + `OnMaxWalkSpeedChanged` 발화 |

- **§1-3 정합 = 델리게이트**: 컴포넌트는 "원하는 속도"만 계산(이동 컴포넌트 접근 X) → `OnMaxWalkSpeedChanged(float)` 멀티캐스트 → **Pawn이 바인드 → 자기 `GetCharacterMovement()->MaxWalkSpeed`에 적용**(Pawn→자기 이동 = 합법). 컴포넌트는 `CurrentSpeed`를 진실원으로 들고 `FInterpTo` 보간, 매 스텝 broadcast.
- **튜닝 값**: WalkSpeed=**250** / JogSpeed=**500** / SprintSpeed=**700** / FullSprintSpeed=**800** / FullSprintTriggerSec=**4.0** / SprintInterpSpeed=**6.0**. CharacterMovement MaxWalkSpeed=500(=Jog 정합). GravityScale=1.25.
- ⚠️ **CurrentSpeed 시드 버그**: 기본값이 옛 300이면 첫 스프린트 때 속도 뚝 떨어졌다 회복(끊김). → 컴포넌트 `BeginPlay`에서 `CurrentSpeed = JogSpeed` 시드(하드코딩 X, 동적값 정합).
- Pawn: 생성자 `CreateDefaultSubobject` + `OnMaxWalkSpeedChanged.AddDynamic(..., ApplyMaxWalkSpeed)`. StartSprint/StopSprint/ToggleWalk → SprintComp 위임. 스프린트 멤버/타이머/속도 파라미터 전부 Pawn에서 삭제.

## ② 입력 디스패치 헬퍼 (중복제거)
Pawn에 private 헬퍼 2개로 `FGameplayTagContainer`+`AddTag`+`Try/Cancel` 반복(~10곳) 교체:
```cpp
bool ActivateByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;
void CancelByTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag) const;
```
동작 변경 0 — 순수 가독성/줄수 정리. `TryConsumeAndActivate`의 다중 태그 CancelTags는 그대로.

## ③ 디버그 코드 삭제
"Player Health" / "Full Sprint ENTERED!" GEngine 메시지, `[KD-HitConfirm]` UE_LOG 등 제거. "정상" 경고 UE_LOG(설정 누락)는 유지.

## 제외 (오탐)
`GA가 GetComboComponent()` 부르는 곳 = §1-3 위반 아님. 콤보 입력 데이터 조회는 코드베이스 표준 패턴(architect 검증). §1-3의 "GA→Component 금지"는 카메라/사운드/UI 부수효과 얘기. 그대로 둠.

## 검증 (PIE) — 통과
걷기/조깅/스프린트/풀스프린트 속도 전환 보간 기존 동일 / ToggleWalk 정상 / 점프·낙하·착지 후 속도 정상 / 좌우클릭·닷지·패링 회귀 없음 / 디버그 메시지 안 뜸.
