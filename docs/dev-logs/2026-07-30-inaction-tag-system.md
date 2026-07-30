# 2026-07-30 — `State.Combat.InAction` 우산 태그 + 회피 캔슬 목록 통합

## 왜 했나

증상 3개를 따로 고치려다 뿌리가 하나임을 발견했다.

- 평시에 가드를 올리면 **무기 없이 패링 자세**
- 패링 후 **납검이 안 됨** (영구 발검 상태)
- 발검/납검 몽타주 중 공격이 들어오면 **빈손 공격**

원인: `CombatStateComponent`와 `WeaponComponent`가 **둘 다 `State.Combat.Attacking` 하나만** 보고 있었다. 공격 GA만 그 태그를 부여하니, **가드·회피는 전투 진입도 무기 부착도 트리거하지 못했다.** 납검은 "InCombat이 켜진 적이 없으니 꺼지는 이벤트도 없다"는 형태로 죽어 있었다.

## 설계 — 우산 태그 (B안)

새 태그 `State.Combat.InAction` 하나를 **각 GA의 `ActivationOwnedTags`에 기존 태그와 함께** 부여하고, 두 컴포넌트가 그걸 구독한다.

```
GA 활성화 → State.Combat.InAction 부여 (GAS가 자동)
                    │
      ┌─────────────┴─────────────┐
      ↓                           ↓
CombatStateComponent        WeaponComponent
InCombat ON + 4초 타이머      AttachWeaponToHand() 즉시
```

`InAction`은 **입력**이고, 전투 상황 진입과 무기 부착은 각각 독립된 **출력**이다.

### `InAction` vs `InCombat` — 중복이 아니다

```
                     공격 시작     공격 끝        +4초
InAction   ▁▁▁▁▁▁▁▁▁███████████████▁▁▁▁▁▁▁▁▁▁▁▁▁▁   약 0.8초 (몽타주 길이)
InCombat   ▁▁▁▁▁▁▁▁▁█████████████████████████▁▁▁▁   약 4.8초 (행동 끝 + 4초)
```

| | `InAction` | `InCombat` |
|---|---|---|
뜻 | "지금 동작 중" | "지금 전투 상황" |
켜는 주체 | GA가 직접 (`ActivationOwnedTags`) | `CombatStateComponent`가 판단 |
끄는 시점 | 행동 끝나면 즉시 | 4초 지연(`CombatExitDelay`) |
하는 일 | 무기 **지금 당장** 손에 + 발검 모션 생략 | 무기 든 상태 **유지** + 전투 자세 로코모션 |

**왜 하나로 못 합치나**
- `InCombat`만 → 4초 유지는 되지만 "지금 뭘 하는 중"을 몰라 **발검 몽타주가 회피를 씹는다**
- `InAction`만 → 공격 끝나는 순간 납검 → **콤보 1타·납검·2타·발검** 반복

**방향은 한쪽뿐**: `InAction`은 `InCombat`을 켜기만 하고 끄지 않는다.

### 왜 태그 배열(A안)이 아닌가

배열을 두 컴포넌트에 각각 노출하면 같은 목록을 **두 군데서 관리**한다 → 한쪽만 추가하면 "가끔 발검이 회피를 씹는다"처럼 원인 찾기 힘든 형태로 어긋난다. 우산 태그는 단일 진실이 **GA 쪽**에 있고, 새 행동 추가 시 **C++ 0줄**(BP에서 태그 한 줄).

## 구현

### C++
| 파일 | 변경 |
|---|---|
`KDGameplayTags.h/.cpp` | `State_Combat_InAction` 선언 + 정의 |
`CombatStateComponent.h/.cpp` | `Attacking` → `InAction` 구독 전환. `AttackingTagHandle`→`InActionTagHandle`, `RegisterAttackingTagListener`→`RegisterInActionTagListener`, `OnAttackingTagChanged`→`OnInActionTagChanged`. **멤버 수 변화 없음** |
`WeaponComponent.h/.cpp` | 멤버 3개 추가(핸들 + 함수 2개). `:97`에서 "InAction 중이면 발검 몽타주 생략 후 즉시 부착", `:142` 핸들러가 강제 부착 |
`Config/DefaultEngine.ini` | `OnAttackingTagChanged`→`OnInActionTagChanged` 함수 리다이렉트(Rider 리네임 자동 생성) |

```cpp
// WeaponComponent.cpp:97 — 발검 몽타주 생략 조건
if (bDraw && ASC->HasMatchingGameplayTag(GameplayTags::State_Combat_InAction))
{
    AttachWeaponToHand();
    return;
}

// :142 — 강제 부착
void UWeaponComponent::OnInActionTagChanged(const FGameplayTag Tag, int32 NewCount)
{
    if (NewCount <= 0) return;
    AttachWeaponToHand();
}
```

### BP — `ActivationOwnedTags` (실측 확인)

| GA | Owned | Blocked |
|---|---|---|
`GA_LightCombo` | Attacking, **InAction** | InAir, Dead, Attacking |
`GA_HeavyCombo` | Attacking, **InAction** | InAir, Dead, Attacking |
`GA_AirLightAttack` | Attacking, **InAction** | AirComboLock, Dead |
`GA_SprintAttack` | Attacking, **InAction** | InAir, Attacking |
`GA_CounterThrust` | Attacking, **InAction** | InAir, Dead (+Required: CounterReady) |
`GA_Dodge` | Dodging, **InAction** | Attacking, Dead |
`GA_Parry` | Parrying, **InAction** | Dead, InAir |
`GA_HitReact` | (없음) | (없음) |

**의도적 제외**: `GA_HitReact` — 피격은 "전투 행동"이 아니다. 넣으면 맞을 때마다 4초 타이머가 갱신돼 영영 납검을 못 한다. `GA_PlayerTurn`은 BP가 없다(C++ 전용).

`GA_SprintAttack`·`GA_CounterThrust`는 원래 `Attacking`도 없었다 → 이 둘이 도는 중엔 `GA_Dodge`가 안 막히고, 버퍼 소비 게이트도 안 걸리고, 적 `BTService_TryParry`가 패링 대상으로 보지 않았다. 같이 추가.

## 회피 몽타주에 `AN_WeaponAttach`는 불필요 (결론)

`ActivationOwnedTags`는 `ActivateAbility`보다 **먼저** 붙는다 → 태그 경로가 몽타주 0프레임보다 이르다. 노티는 늦고 하는 일도 같다(`AttachWeaponToHand` 동일 호출).

**노티가 여전히 필요한 곳 = 발검/납검 몽타주.** GA 없이 `WeaponComponent`가 직접 재생하고, 부착 프레임이 "손이 자루를 잡는 순간"이라 애니마다 다르다. 태그로는 표현할 수 없는 타이밍이다.

(`AM_SB_Block_Start`의 노티는 InAction 도입 전에 넣은 것 → 지금은 중복이지만 idempotent해서 무해)

## 곁가지 — 회피 캔슬 목록 통합

같은 취소 목록을 **세 곳에서 손으로 관리**해서 셋 다 다르게 어긋나 있었다.

| 위치 | 기존 목록 |
|---|---|
`TryConsumeAndActivate:93` | Light, Heavy, Dodge, AirCombo |
`TryDodge:286` | **Light만** |
`Tick:379` (회피 버퍼) | **Light만** |

`Attacking`은 `GA_Dodge`를 막고(`ActivationBlockedTags`), 캔슬 경로는 Light만 끊는다 → **강공격 캔슬 윈도우에서 Shift를 눌러도 태그가 안 풀려 회피가 안 나갔다.** 게다가 `Tick`의 `TryConsume`이 입력을 먼저 지우고 실패하므로 **입력이 조용히 먹혔다**(다시 눌러야 함).

`Ability.Mugong` 아래에 Dodge/Parry/Execution이 같이 있어 부모 태그로 못 묶는다 → 명시 목록 하나로 통합.

```cpp
namespace
{
	// 캔슬 윈도우에서 끊을 수 있는 공격 GA 목록
	const FGameplayTagContainer& GetCancelableAttackTags()
	{
		static const FGameplayTagContainer Tags = []
		{
			FGameplayTagContainer C;
			C.AddTag(GameplayTags::Ability_Mugong_Light);
			C.AddTag(GameplayTags::Ability_Mugong_Heavy);
			C.AddTag(GameplayTags::Ability_Mugong_SprintAttack);
			C.AddTag(GameplayTags::Ability_Mugong_CounterThrust);
			return C;
		}();
		return Tags;
	}
}
```

**함수 안 `static`인 이유**: 파일 스코프 `static`으로 두면 네이티브 태그 등록 **전에** 초기화돼서 빈 태그가 박힌다.

공격 경로는 여기에 `Dodge`/`AirCombo`를 더해 기존 동작 유지. **회피 경로는 `AirCombo`를 뺐다** — `DodgeMontages` 4개가 전부 지상 클립이라 공중에서 자세가 어긋난다(공중 회피 도입은 별도 판단).

**`bCanCancel` 조건은 세 곳 다 손대지 않았다.** 타이밍이 아니라 "무엇을 끊나"만 바뀐다.

### 칼 휘두르는 중 회피는 여전히 불가능 (실측)

| 몽타주 | 길이 | 판정 | 캔슬 윈도우 | 공백 |
|---|---|---|---|---|
`Combo_01_01` | 100f | 11~21f | 51~100f | 30f |
`Combo_01_02` | 100f | 16~23f | 44~100f | 21f |
`Combo_01_03` | 95f | 10~26f | 45~95f | 19f |
`Combo_01_04` | 120f | 16~22f | **없음** | — |
`Combo_02_01` | 100f | 20~23f | 35~100f | 12f |
`Combo_02_02` | 85f | 14~19f | 62~85f | 43f |
`Combo_02_03` | 163f | 19~27f | 74~163f | 47f |
`Combo_02_04` | 119f | 13~21f | **없음** | — |

판정 종료 후 **12~47프레임(0.2~0.8초) 뒤에야** 캔슬 윈도우가 열린다. 4타 마무리(`*_04`)는 캔슬 윈도우가 **아예 없다** = 확정타.

## 함께 고친 것

| 항목 | 내용 |
|---|---|
`ABP_SB` 슬롯 재배치 | `Slot 'DefaultSlot'`이 `포즈히스토리 → LocoBase` 사이에 있어 **Grounded 상태만** 몽타주를 받았다. 점프 상태들은 슬롯을 안 거쳐 공중 공격이 보이지 않았다 → 슬롯을 Locomotion SM **뒤**로 이동. 옛 노드를 안 지워 한때 중복(엔진 경고 `'DefaultSlot'는 이미 존재합니다`)이 났고, 지운 뒤 경고 소멸 확인 |
`AM_SB_Combo_Air_01` 노티 복원 | 노티 0개였다. 이게 `DA_AirComboTree`의 **진입 노드**라 `CancelWindow`가 없으면 입력을 소비할 창이 없어 `Air_1 → Air_2` 링크를 탈 수 없다 = 공중 콤보가 1타에서 막힌다. `WeaponAttach@0f` + `CancelWindow 28~55f` 배치 |
퍼펙트 패링 큐 발동 | `GCN_PerfectParry`(Timeline + Start Parameter 0.7,0.7,0.7,1.0)와 `LCS_PerfectParry`가 **만들어져 있는데 아무도 안 불렀다.** `GA_PerfectParryReaction`에 `Execute GameplayCue On Owner`(태그 `GameplayCue.Combat.PerfectParry.SlowMo`) 추가 |

## 검증

| 항목 | 결과 |
|---|---|
빌드 | 통과 |
BP 태그 7개 | CDO 조회로 실측 확인 |
ABP 슬롯 | 노드 13→12, `LocoBase ← 포즈히스토리` 직결 확인, 엔진 경고 소멸 |
공중 콤보 4타 연결 | PIE 로그 08:17 — `Air_01→02→03→04` 순차 INTERRUPTED 확인 |
PIE 전반 | 승환 확인 "이상 없었음" |

**미해결로 남은 것**: 퍼펙트 패링/닷지 이펙트가 **여전히 안 나온다.** 큐 노드는 붙었으나 `GameplayCueNotifyPaths`(`Config/DefaultGame.ini:17-19`)가 죽은 폴더만 가리켜 큐 자체가 등록되지 않는다. `GameplayCue.PrintGameplayCueNotifyMap` 실측 = **연결 3 / unmapped 9**. → `docs/handoffs/2026-07-30-parallel-sessions.md §2` 참조

## 커밋

```
소스   4a44cfd [GAS]    InAction 우산 태그
       474bd52 [Combat] 회피 캔슬 목록 확대
Content 33b9957 [BP]    InAction 태그 7개 + 퍼펙트패링 큐 + ABP 슬롯
       72017a2 [Anim]   Air_01 노티 + 스프린트/반격 Attacking
```

## 배운 함정

1. **`.h` 선언과 `.cpp` 정의는 짝이다.** `InAction`이 `.cpp`에만 있어 빌드가 깨졌다
2. **구독과 해제는 같은 태그로.** `RegisterGameplayTagEvent(InAction).Add()` 후 `RegisterGameplayTagEvent(Attacking).Remove()` 하면 조용히 실패한다 — 핸들 `Reset()`은 그대로 돌아서 코드가 깨끗해 보이고 빌드도 통과한다
3. **`search_assets`는 패턴을 무시하고 100개를 반환한다.** 목록·집계는 `execute_python`으로 (실측: 821개 중 100개, 1만 토큰 낭비)
4. **`AnimMontage`의 `Notifies` 프로퍼티는 protected라 Python에서 못 읽는다.** `unreal.AnimationLibrary.get_animation_notify_events()` 사용
