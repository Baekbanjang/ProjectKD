# 2026-08-28 — 패링 재설계 1·2·4① (창 / 반격 사슬 / 막기의 대가)

승환 지시 = **"패링이 구리다. 1부터 100까지 손봐야 한다"** + **"SB 레퍼런스니까 SB 대로 가고 싶다"**.
SB 덤프 전수조사와 우리 현행 실측을 대조해 5단계 계획을 세우고 그중 셋을 끝냈다.

계획 전문 = `docs/handoffs/CURRENT.md` §2026-08-28
SB 실측 전문 = 볼트 브릿지 `SB 노트 신규 등재 — 패링·막기 체계와 연출 전수`

## 진단 — 부품이 없는 게 아니라 사슬이 끊겨 있었다

승환이 꼽은 문제가 넷(입력·연출·보상·애니)이었는데, 파보니 **한 뿌리**였다.

```
애니   Block 5종 · Parry_L/R · Parry_Counter_Attack_L/R  전부 배선됨. 부족하지 않다
연출   GCN_PerfectParry(노드 21) · LCS_PerfectParry · GA_PerfectParryReaction(노드 13)
       전부 배선돼 있다. 슬로모 0.3배 0.15초 = SB(0.25배 0.15초)와 거의 같다
창     GE_PerfectParry Duration 0.5   <- SB 0.15 의 3.3배
보상   퍼펙트 패링 = 데미지 0 + Poise 4.0 이 전부. 파생기 없음
```

🔴 **핵심** — `State.Combat.CounterReady` 를 부여하는 곳이 **`GA_Dodge` 하나뿐**이었다(소스 grep).
즉 **반격이 퍼펙트 회피에만 붙어 있고 퍼펙트 패링에는 없었다.**
그런데 `GA_CounterThrust` 가 재생하는 몽타주 이름이 `AM_SB_Parry_Counter_Attack_L` 이다 —
**패링용 애니를 회피 반격이 쓰고 있었고, 짝인 `_R` 은 참조 0건으로 놀고 있었다.**

성공이 흔하고(창 0.5) 성공의 출구가 없었다(파생기 0). 세 축이 각각 고장난 게 아니었다.

## SB 대조 — 입력 구조는 이미 같았다

```
                SB                          우리
입력 구조        홀드 하나 + 내부 타이밍 분기    같다 (갈아엎을 필요 없음)
저스트 창        0.15                        0.5
막기의 대가      실드 소모 -> 가드 붕괴         없다. 고정 50% 감소만
저스트 보상      격파 -1 + 반격기(2.5~10.4)   데미지 0 + Poise 4.0
일반 막기 연출    전무                         전무 (구조 일치)
```

`SkillCommandTable / P_Eve_Sword_Normal_Guard1_1` 의 `Command = Guard · InputType = HoldKeep` 이
이브의 유일한 Guard 커맨드 행이다. **저스트는 별개 입력이 아니라 홀드 안의 타이밍 상위판정**이다.

## 1단계 — 창은 어디에 있나 (값은 아직 0.5)

`GE_PerfectParry` Duration 하나. **0.5 는 촬영용으로 올린 값이 안 돌아온 것**이었고,
08-24 에 "원복했다"고 적힌 건 아무도 안 읽는 죽은 노브(`PerfectParryWindowSec`) 쪽이었다.
그 노브는 08-27 에 삭제해서 이제 진실은 이 GE 하나뿐이다.

🔴 **정정 (2026-08-30 실측)** — 이 절이 종전에 "창 0.5 -> 0.15" 로 적혀 있었으나
**값은 바뀌지 않았다.** 에셋 실측 `GE_PerfectParry.DurationMagnitude = 0.5`.
승환이 "패링은 그대로 유지할거임, 계속 테스트할거라서" 라고 결정한 것을 문서가 못 따라간 것이다.
```
SB 실측     0.15
우리 현행   0.5      의도적 유지. 나머지 축(반격·실드·붕괴)을 다 붙인 뒤 감각을 보고 조인다
```
⚠️ 이 오기가 볼트 노트 5곳으로 번졌다(`04` 4곳 · `07` 1곳 · `01` 2곳). 브릿지로 정정 요청함.
**값을 정하는 것과 값을 어디서 바꾸는지 아는 것은 다른 일이다.** 이 절은 후자만 해결했다.

★ **슬로모는 무죄였다.** `GCN_PerfectParry` 의 Timeline 길이가 0.15초로 SB 와 같았다.
"늘어져서 밋밋하다" 는 가설을 세웠다가 실측으로 뒤집었다.
```
Set Global Time Dilation 0.3 -> TimeLine(0.15초) -> Set Global Time Dilation 1.0
Timeline 커브 (0,0) -> (0.05,1) -> (0.15,0) 은 시간이 아니라 MPC 화면 효과 강도를 그린다
```
⚠️ Timeline 커브는 파이썬에 안 열린다. 에디터에서 눈으로 봐야 한다.

## 2단계 — 반격 사슬

### 2-1 다리 놓기 (C++ 0줄)

```
GE_PerfectParry_Counter        신규 · HasDuration 1.0 · State.Combat.CounterReady
GA_PerfectParryReaction        ApplyGameplayEffectToOwner 노드 추가
   Send Gameplay Event to Actor -> [여기] -> Execute GameplayCue On Owner
```

**이후는 회피와 같은 경로를 그대로 탄다.** 입력 컴포넌트를 안 고쳐도 좌클릭에 반격이 나간다.

📌 반격 창을 SB(0.1초)가 아니라 **1.0초**로 잡았다. 우리 회피 쪽
`GE_PerfectDodge_CounterThrust` 가 이미 1.0이라 리듬을 맞추는 게 먼저라고 봤다.

### 2-2 좌/우 두 갈래

```
Ability.Player.CounterSlash          신규 태그 (.h/.cpp 짝)
UKDGameplayAbility_CounterSlash      부모 = CounterThrust. 생성자만
                                     AbilityTags.Reset() 후 새 태그
TryHeavyAttack()                     CounterReady 분기 추가
GA_CounterSlash BP                   AM_SB_Parry_Counter_Attack_R
```

🔴 **`AbilityTags.Reset()` 이 필수다.** 부모 생성자가 `CounterThrust` 태그를 박기 때문에
안 지우면 태그가 둘이 되어 약공에도 베기가 걸린다.

⚠️ **분기 위치가 캔슬 윈도우 판정보다 앞이어야 한다.** 뒤면 회피 캔슬이 먼저 걸려 반격이 안 나간다.

⚠️ `AM_SB_Parry_Counter_Attack_R` 은 **노티 0개**다. 재생만 되고 판정이 없다. 미해결.

### 왜 BP 가 아니라 C++ 층인가

BP 의 `AbilityTags` 만 갈아도 되지만, **이 프로젝트는 그걸로 이미 당했다** —
`GA_SprintAttack` 이 `AbilityTags` 가 빈 채로 달리기 공격이 통째로 죽어 있었고 08-26 에야 발견됐다.
발동을 태그로만 하니 에러도 로그도 없다.

> **판단 기준** — 없어지면 조용히 죽는 것(태그·발동 조건·상태 전이)은 C++ 에,
> 없어지면 눈에 띄는 것(몽타주·이펙트·수치)은 BP/에셋에.

## 4단계 ① — 막기의 대가

### 뒤집힌 것

```
종전   막으면 데미지 x 0.5 후 실드가 40% 흡수
       -> 막으면 실드가 오히려 천천히 닳는다 (0.4배 -> 0.2배). SB 와 정반대
현행   막으면 실드 80% 흡수 · 안 막으면 40%
       실드 0 이면 조기 반환으로 막아도 HP 로 전부
```

SB 근거 = `EffectTable / P_Eve_GuardShieldDamage_Step` 이 가드 스텝 동안
`FinalShieldDamageReduceRate` 를 **-50(StaticPercent)** 한다 = 가드 중 실드 데미지 증가.
곁들여 **적 공격 스킬 874개 전부**가 `AttackDamageRate` 와 `ShieldAttackDamageRate` 를 각각 갖는다
= HP 축과 실드 축이 따로 있고 공격이 둘 다 깎는다.

### Defense 를 비율로

```
없어짐   UKDCombatAttributeSet::Defense               뺄셈
생김     DamageReductionRate                          비율 (PreAttributeChange 0~1)
         ShieldAbsorbRate         0.4                 평소 흡수율
         BlockShieldAbsorbRate    0.8                 정면 방어 중
계산     Damage - Defense  ->  Damage x (1 - DamageReductionRate)
```

**SB 에 뺄셈식 방어력이 없다.** `CharacterTable` 의 감소 컬럼이 전부 `~Rate` 다
(`MeleeAttackDamageReductionRate` · `FinalHPDamageReduceRate` · `BaseDamageReductionByShield` …).
그리고 뺄셈은 약한 공격일수록 손해가 커서, 데미지 10 짜리를 만들면 `Defense 10` 엘리트에겐 0이 된다.

⚠️ **흡수율을 GE 하나로 덮는 방식은 안 된다.** `GE_Block` 은 뒤에서 맞아도 켜져 있어서
방향 판정(`bBlockedHit`)이 빠지면 등 뒤 공격도 막아진다. 그래서 어트리뷰트 2개 + 방향 판정으로 갔다.

### 기획자 튜닝을 위해 어트리뷰트로

처음엔 `constexpr` 상수로 썼는데 승환 지적으로 바꿨다 — **손맛 잡느라 열 번 바꿀 값을 재컴파일로 두면 안 된다.**
`CLAUDE.md §3` 의 "Editor 튜닝 우선" 이 이 경우의 기준이고, 내가 옆에 있는
"1곳에서만 쓰이면 코드 상수"(데이터 승격 기준)를 잘못 적용했다.
SB 도 상수가 아니라 스탯으로 두고 이펙트로 수정한다.

### 실드 리젠 차등 (SB 8배 비율)

```
GE_ShieldRegen             Ignore  State.Combat.InCombat · Shield +0.3  = 초당 3.0
GE_ShieldRegen_InCombat    Require State.Combat.InCombat · Shield +0.04 = 초당 0.4   ★신규
SB   ShieldRegenPerSecond 80 / WhenBattle 10 = 8배
```

종전엔 **전투 중 회복이 0** 이라 실드를 다 쓰면 그 전투 내내 막기가 없었다.

📌 전투 중 0.4/s 는 막기를 다시 채워주는 양이 아니다(16 채우는 데 40초).
**차등의 실효는 전투 밖 복구가 빠른 쪽에 있다.**

## 잡은 버그 2건

```
① return FMath::Max(Mitigated - AbsorbRate, 0.0f)    Absorbed 여야 했다
   그대로면 실드만 닳고 HP 는 거의 다 받는다 = 이중 손해
② DA 에 옛 Defense 값 10 이 그대로 넘어왔다
   ClampMax 1.0 은 에디터 입력만 막고 저장된 값엔 소급 안 된다
   -> 1 - 10 = -9 로 데미지가 음수 = 맞을수록 회복
   코드에 FMath::Clamp 를 넣어 값이 잘못돼도 음수가 안 되게 막았다
```

②가 특히 중요하다. **개명 리다이렉트는 값을 이어주지만 그 값이 새 의미에 맞는지는 안 본다.**

## 개명 절차 — 세 번째는 사고가 없었다

오늘 하루에 같은 사고를 두 번 겪은 뒤(DA 5개 · 몽타주 11개) 절차가 자리 잡았다.

```
1  개명 전에 PropertyRedirects 를 먼저 건다
2  에셋을 강제 저장          save_asset(path, only_if_is_dirty=False)
3  바이너리 검사로 옛 이름 0건 확인
4  그 다음에 리다이렉트를 지운다
```

⚠️ 2번 — **에디터 `Save` 는 dirty 가 아니면 아무 일도 안 한다.** 리다이렉트로 로드된 에셋은
메모리에선 새 이름을 물지만 "바뀐 것" 표시가 안 선다. 오늘 두 번 여기 걸렸다.

검증 = Content 930개 바이너리 스캔, 옛 이름 0건 → `ini` 2줄 삭제 → `git diff` 0.

## 검증

```
빌드      통과 (승환)
PIE       막으면 실드가 빨리 닳는 것 확인 (승환)
          패링/회피 후 좌=찌르기 우=베기, 평상시 강공 회귀 정상
          실드 리젠 전투 중/밖 차등 동작
정적      옛 이름(Defense · GetDefense) 소스 0건 / Content 바이너리 0건
```

## 남은 것

```
4단계 ②   가드 붕괴 — 실드 0 -> 무방비. AM_SB_Block_Hit_Break 가 이미 있다
          ⚠️ 감지 위치가 함정. PostGameplayEffectExecute 는 "사후 로직 금지" 자리다
3단계      연출 이식 (반격기에 몰아주기 · CustomTimeDilation 1.5 · 카메라 랙 동적 제어)
보류       패링 반격과 회피 반격이 아직 같은 태그를 공유한다
          SB 는 나눠져 있으나 반격 애니가 2개뿐이라 지금 나누면 회피가 손해만 본다
미해결     AM_SB_Parry_Counter_Attack_R 노티 0개
값         실드 총량 50 · 흡수율 0.8 · 회복 3/0.4 — ② 끝나고 셋을 같이 조인다
```

커밋 = 코드 `92f7ff4` · `11db1ff` / 에셋 `8822d13` · `9276fac`
