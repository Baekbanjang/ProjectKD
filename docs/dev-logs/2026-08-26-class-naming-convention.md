# 2026-08-26 — 클래스 네이밍 규약 정렬 (68개 개명)

C++ 클래스명을 UE/GAS 표준에 맞췄다. **문서가 옳고 코드가 비표준이었다.**

**커밋** — `456fc3d`(E) · `a510235`(D) · `56ff9a4`(C)
**직전 작업** = `2026-08-26-poise-context-channel.md`

---

## 1. 판단 근거 — 엔진 실측

```
엔진 GAS      UAbilitySystemComponent · UGameplayAbility · UAttributeSet · UGameplayEffect
              타입 접두가 하나도 없다
엔진 AI       UBTTask_MakeNoise · UBTService_DefaultFocus · UEnvQueryContext_Querier
              이 접두는 엔진 자신이 쓴다
엔진 애니      UAnimNotifyState_Trail · UAbilityTask_ApplyRootMotionMoveToForce
              풀네임을 쓴다
```

`GA_` `GE_` `AS_` `GCN_` 은 **에셋 이름 규칙**(`GA_LightCombo.uasset`)이지 C++ 클래스 규칙이 아니다. 우리 `CLAUDE.md §9` 표도 "에셋" 기준인데 코드가 그걸 따라가 있었다.

> ⚠️ 2026-08-25 에 내가 "문서를 현실에 맞추자"고 했던 건 *일관성* 근거였지 *표준* 근거가 아니었다. 엔진을 열어보니 문서 쪽이 맞았다. 그 판단을 뒤집는다.

## 2. 5그룹으로 나눠 처리

| 그룹 | 개수 | 처리 | 위험 |
| --- | --- | --- | --- |
| A 이미 준수 | 17 | 변경 X | — |
| B 엔진 관례 | 9 | **변경 X** — 엔진과 동일하므로 위반이 아니다 | — |
| E 접두 없음 | 21 | `UWeaponComponent` -> `UKDWeaponComponent` | 🟢 |
| D GAS 에셋 접두 | 36 | `UAS_`·`UGA_`·`UGE_`·`UGCN_` -> 표준 | 🟠 BP 19개가 부모 |
| C 엔진 관례 축약 | 12 | `UANS_`·`UAN_`·`UAT_` -> 풀네임 | 🔴 몽타주에 인스턴스 |

**위험도 낮은 순으로 E -> D -> C.** 그룹마다 커밋을 끊어 무엇이 깨졌는지 추적 가능하게 했다.

### 주요 개명

```
UGA_ActionBase              ->  UKDGameplayAbility            모든 GA 의 뿌리
UGA_MeleeTraceBase          ->  UKDGameplayAbility_MeleeTrace
UGA_PlayerMeleeAttackBase   ->  UKDGameplayAbility_PlayerMelee
UGA_PlayerAttackBase        ->  UKDGameplayAbility_PlayerCombo
UGA_PlayerAirAttackBase     ->  UKDGameplayAbility_PlayerAirCombo
UGA_EnemyWeaponTraceBase    ->  UKDGameplayAbility_EnemyMeleeTrace
UAS_Combat                  ->  UKDCombatAttributeSet
UAT_MeleeTrace              ->  UKDAbilityTask_MeleeTrace
UANS_MeleeTrace             ->  UKDAnimNotifyState_MeleeTrace
```

## 3. 정적 검사 3종 — 개명 작업의 핵심 도구

Rider 리네임이 놓치는 자리가 있어 매 그룹 후 이 셋을 돌렸다.

```
① 깨진 프로젝트 include     #include "AbilitySystem/..." 를 뽑아 파일 존재 대조
② generated.h 불일치       각 .h 의 파일명과 include 한 generated.h 이름 비교
③ 구 이름 잔존             주석 · 로그 문자열 포함 전수 grep
```

### ① 이 실제로 잡은 것

`KDGameplayAbility_MeleeTrace.h:4` 가 옛 `GA_ActionBase.h` 를 물고 있었다.

```
부모를 못 찾음 -> UKDGameplayAbility_MeleeTrace 미정의
              -> UKDGameplayAbility_PlayerMelee 미정의
              -> UKDGameplayAbility_PlayerAirCombo 에 에러
```

**Rider 가 빨간 줄을 여러 파일에 뿌릴 때 대부분 뿌리는 하나다.** 상속·include 사슬의 제일 위부터 고치면 아래가 한꺼번에 사라진다.

### ③ 이 잡은 것 — 12곳

Rider 리네임은 **주석과 문자열을 안 건드린다.** `UE_LOG` 안의 `"[KD] UComboComponent:"` 같은 것이 남는다. 컴파일은 되지만 나중에 그 로그로 클래스를 찾으면 없다.

## 4. CoreRedirects

Rider 가 `DefaultEngine.ini` 에 **68줄을 자동 생성**했다(32 -> 68). 옛 리다이렉트(`GA_WeaponTraceBase` 등)도 새 이름을 가리키도록 연쇄 갱신됐다.

```
ClassRedirects (이번 것)       C++ 클래스 개명 -> 에셋이 저장한 옛 경로를 이어준다
에셋 리다이렉터 (별개)          .uasset 을 옮기거나 개명할 때 생긴다. 이번엔 X
```

> 🔴 **리다이렉트는 임시 다리다.** 에셋을 재저장해야 새 경로가 저장되어 다리가 필요 없어진다. **재저장 전에 ini 줄을 지우면 에러 없이 죽는다** — 2026-08-25 처형 큐 태그 사건과 같은 구조.

## 5. 검증 상태

```
빌드                통과 (E·D·C 한 번에)
정적 검사 3종        전부 0건
```

> 🔴 **에디터 검증 미완.** 새 세션에서 이어서 할 것 — `CURRENT.md` 참조.
> BP 19개 부모·태그 / 몽타주 노티 / DA 값 / PIE / 몽타주 전수 재저장.

## 6. 남긴 것

```
B그룹 9개     UBTTask_ · UBTService_ · UEnvQueryContext_
              엔진이 똑같이 쓰므로 의도적으로 유지
```

## 7. 문서 파급 — 아직 안 함

```
docs/PROJECT_OVERVIEW.md   §2 시스템 맵 · 상속 트리에 옛 클래스명이 전부 남아 있다
볼트 notes/코드구조/ 10문서  🔴 클래스명 68개가 전부 바뀌었다. 브릿지로 요청함
docs/reference/네이밍 규약   이제 코드가 문서를 따라갔다. Op 예시만 KD 로 (🔒 승환)
```

⚠️ 볼트 코드구조가 옛 이름으로 남으면 **다음 설계 때 없는 클래스를 근거로 삼게 된다.** `docs/design/` 이 거짓이 됐던 것과 같은 경로다.
