# 실드 어트리뷰트 + 좌하단 3줄 바이탈

`HP` 만 있던 자리를 `HP / ST / SH` 로 늘리고, 탄약을 크로스헤어 아래로 옮겼다.

**소스** — `AS_CharacterBase.h/.cpp` / `AS_Player.cpp` / `AS_Combat.h/.cpp`
**에셋** — `GE_ShieldRegen`(신규) / `WBP_PlayerInfo` / `WBP_Crosshair` / `WBP_EnemyStateBar` / `BP_PlayerState`
**앞 문서** — [2026-08-18-player-hud](2026-08-18-player-hud.md)

---

## 1. 실드 = 감소형. 상수 하나로 줄였다

SB 실측은 `DamageReductionPerShieldBock`(칸 수 비례 감소) + `ShieldAttackDamageRate`(실드 전용 피해 계수) **두 개**를 따로 둔다. 우리는 **"막아준 만큼 실드가 닳는다"** 로 묶어 하나로 만들었다.

```cpp
// AS_Combat.cpp — Defense 경감 직후
constexpr float ShieldDamageReduction = 0.4f;

float ToHealth = Mitigated;
const float Shield = ASC->GetNumericAttribute(UAS_CharacterBase::GetShieldAttribute());
if (Shield > 0.0f && Mitigated > 0.0f)
{
    const float Absorbed = FMath::Min(Shield, Mitigated * ShieldDamageReduction);
    ASC->SetNumericAttributeBase(UAS_CharacterBase::GetShieldAttribute(), Shield - Absorbed);
    ToHealth = Mitigated - Absorbed;
}
```

```
피해 20 · 실드 50   실드가 8 받음    실드 42 / 체력 -12
피해 20 · 실드 3    실드가 3만 받음  실드 0  / 체력 -17     남은 실드가 상한
피해 20 · 실드 0    경감 X          체력 -20
```

`FMath::Min` 이 없으면 **실드가 음수**가 된다.

### 두 번째 체력이 아니다

흡수형(실드가 먼저 다 깎이고 그다음 체력)과 헷갈리기 쉽다. 감소형은 **한 대 맞을 때마다 둘 다 줄고**, 실드가 있으면 체력이 덜 준다.

### `ToHealth` 를 안 쓰면 조용히 죽는다

`Mitigated` 로 실드를 깎아놓고 `NewHealth` 계산에 다시 `Mitigated` 를 쓰면, **실드는 닳는데 체력은 전액 맞는다.** 실드가 아무 일도 안 하는 상태가 되고 에러는 안 난다. 아래 세 줄이 전부 `ToHealth` 여야 한다.

```cpp
const float NewHealth = ... - ToHealth;
const bool bLethal = ToHealth > 0.0f && NewHealth <= 0.0f;
if (ToHealth <= 0.0f) { return; }
```

---

## 2. 어트리뷰트 위치와 초기값

```
UAS_CharacterBase   Shield / MaxShield 추가   4개 -> 6개 (한도 8 이내)
                    InitShield(0) / InitMaxShield(0)      적은 실드 X 로 시작
UAS_Player          InitShield(50) / InitMaxShield(50)    부모의 0 을 덮음
```

`UAS_CharacterBase` 에 둬서 **적도 같은 어트리뷰트를 갖는다.** 기본 0이라 지금은 아무 영향이 없고, `DA_EnemyDef_*` 에서 값을 주는 순간 적 상태 바의 빈 SH 자리가 살아난다.

---

## 3. 재생 — 새 태그 0개

```
GE_ShieldRegen   Infinite / Period 0.1 / Shield AddBase 0.3      초당 3, 0->50 = 17초
                 Ongoing > Ignore Tags = State.Combat.InCombat
BP_PlayerState.StartupEffects 에 등록
```

**전투 중에는 안 찬다.** `State.Combat.InCombat` 이 이미 있어서 새 태그도 새 코드도 필요 없었다. 상시 재생이면 실드가 전투 내내 무한이 된다.

`GE_StaminaRegen` 이 `State.Stamina.RegenBlocked` 를 같은 자리에 쓰는 것과 동일한 패턴.

> ⚠️ GE 를 만들기만 하고 `StartupEffects` 에 안 넣으면 아무 일도 안 일어난다. 실제로 한 번 빠뜨렸다.

---

## 4. ★ `ProgressBar` 는 색 노브가 두 개고 서로 곱해진다

세 바가 전부 시안으로 나왔다. 브러시 Tint 는 제대로 들어가 있었는데 화면이 파랬다.

```
Style > Fill Image > Tint    내가 넣은 색
Fill Color and Opacity       UMG 기본값 (0, 0.5, 1)   ← 이게 파랑
```

```
흰색           x (0, 0.5, 1) = (0, 0.5,  1  )   시안
(0.9,0.85,0.4) x (0, 0.5, 1) = (0, 0.43, 0.4)   어두운 청록
```

`SProgressBar` 가 `FillColorAndOpacity x 위젯 tint x 브러시 tint` 를 곱해 그린다.

→ **색은 `Fill Color and Opacity` 한 곳으로 모으고 브러시 Tint 는 흰색으로 둔다.** 그래프에서 바꿀 수 있는 쪽이 이쪽이라, 나중에 저체력 빨강 같은 걸 넣을 때 `Set Fill Color and Opacity` 하나면 된다.

**적 상태 바의 포이즈도 같은 이유로 노랑이 아니었다.** 노랑 x 기본 파랑 = 어두운 청록으로 나오고 있었다.

---

## 5. ★ `Height Override` 도 슬롯 정렬에 끌려간다

ST · SH 줄만 도트 두 번째 줄이 아래가 잘렸다.

```
SizeBox_ST   Height Override 10
그 슬롯의 Vertical Alignment = Fill
HorizontalBox 높이 = 가장 큰 자식 = 라벨 텍스트 약 16px
-> SizeBox 가 16 까지 늘어남 -> 16 / 10 = 1.6줄
```

HP 줄은 20 이라 라벨보다 커서 안 늘어났다. **그래서 ST · SH 만 잘렸다.**

→ 슬롯 `Vertical Alignment = Center`.

`Width Override` 가 부모 `Fill` 에 끌려가던 것(적 Poise 바)과 같은 함정이다. **Override 계열은 전부 "원하는 크기"일 뿐, 부모가 Fill 이면 무시된다.**

---

## 6. ★ UE5.3+ 에서 GE 태그 조건은 컴포넌트 안에 있다

`GE_AmmoRegen` 에 조건 칸이 아예 안 보였다.

```
디테일 패널 아래 [+ 컴포넌트 추가]
 └ 타깃 태그 요구 사항 (Target Tag Requirements)
      지속되는 태그 요구 사항 > 필수 태그 / 무시 태그
```

**컴포넌트를 안 붙이면 조건 칸 자체가 없다.** 구버전 속성(`UGameplayEffect::OngoingTagRequirements`)은 아직 읽히지만 에디터가 쓰는 곳은 `GEComponents` 안이다 — 파이썬으로 검증할 때 구버전 속성만 보면 **"비어 있다"고 오판한다.** 실제로 오판했다.

---

## 7. ★ `Wait` 의 `OldValue` 가 `MaxValue` 에 붙었다

세 갈래 전부 같은 오배선. `NewValue` 를 끌어올 때 바로 아래 `OldValue` 가 같이 붙은 것으로 보인다.

```
Wait(Health).OldValue ──> RefreshBar 의 MaxValue
```

```
체력 100 -> 80   80 / 100 = 0.80   맞아 보인다 (최대치가 100이라 우연히 일치)
체력  80 -> 60   60 /  80 = 0.75   실제로는 0.60
```

**첫 타격만 맞고 그다음부터 바가 덜 줄어든다.** 처음이 정상으로 보여서 놓치기 쉽다.

---

## 8. `RefreshBar` — 바를 인자로 받게

바가 셋이 되면서 같은 4노드 묶음이 3벌 생길 뻔했다.

```
RefreshBar ( Target: Progress Bar, NewValue: float, MaxValue: float )
  Branch ( MaxValue > 0 ) -> Target > Set Percent ( NewValue / MaxValue )
```

`Construct` 는 `Sequence` 로 `HP / ST / SH` 세 갈래로 나눴다. **한 갈래 안에서 "최대값 캐시 -> 첫 값 표시 -> 구독"이 다 끝나므로** 갈래끼리는 순서가 상관없고, 실드를 빼거나 자원을 추가할 때 갈래 하나만 건드리면 된다.

---

## 9. 탄약을 크로스헤어 안으로

좌하단 상시 표시를 버리고 **조준 중에만 크로스헤어 아래**에 현재 탄약만 띄운다(`MAX` 없음).

`WBP_Crosshair` 의 `RootCanvas` 는 이미 자기 Tick 에서 조준 태그를 읽어 표시를 정한다. **그 안에 넣으면 켜고 끄는 로직을 하나도 안 짜도 된다.** SB 가 조준 UI 를 `Canvas_TPS` 로 통째 분리한 것과 같은 취지.

```
Tick 체인 끝
  Get Player Pawn -> Get Float Attribute (Ammo) -> To Text (Float) -> Set Text
                                                -> (Ammo <= 0) -> Select -> Set Color and Opacity
```

**여기만 구독이 아니라 Tick 이다.** 이 위젯은 이미 매 프레임 ASC 를 읽고 있어서, 구독을 새로 만들어 `EndAction` 수명까지 관리하는 것보다 한 줄 얹는 게 싸다.

### `Branch` 대신 `Select`

처음엔 `Branch` 로 짰다가 `else` 가 비어서 **한 번 빨개지면 안 돌아왔다.** `Select` 로 바꾸면 양쪽 색이 항상 같은 노드에서 결정돼 그 실수가 구조적으로 불가능해진다.

---

## 10. 수치

```
ShieldDamageReduction   0.4          AS_Combat.cpp
Player Shield / Max     50 / 50
GE_ShieldRegen          0.1s 당 0.3  초당 3 · 0->50 = 17초 · 전투 중 X

SizeBox_HP  400 x 20    도트 2줄
SizeBox_ST  400 x 10    1줄
SizeBox_SH  400 x 10    1줄
라벨        Min Desired Width 32 / Right / Padding Right 8 / VAlign Center

Fill Color and Opacity
  HP  (1, 1, 1)              흰색
  ST  (0.95, 0.78, 0.20)     금색
  SH  (0.30, 0.75, 0.65)     청록
  적 포이즈 (0.937, 1, 0.455)  SB 실측 #EFFF74
```

---

## 11. 검증 (PIE)

```
좌하단 HP / ST / SH 3줄 + 라벨          통과
회피 -> ST 감소                         통과
피격 -> SH 먼저 줄고 HP 는 덜 줆        통과
전투 이탈 -> SH 만 서서히 회복          통과
적 포이즈 칸이 노랑                     통과
조준 -> 크로스헤어 아래 탄약            통과
30발 소진 -> 사격 차단 + 숫자 빨강      통과
만충 -> 흰색 복귀 + 사격 재개           통과
```

---

## 12. 적 실드까지 같이 넣었다

```
EnemyDefinitionDataAsset.h    float MaxShield = 0.f
KDEnemyBaseCharacter.cpp      MaxShield / Shield 두 줄, Defense 아래
DA_Arrow 0 / DA_Sword 0 / DA_Parry 30 / DA_Axe_Elite 60
WBP_EnemyStateBar             SizeBox_Shield 200 x 10, HP 와 Poise 사이
                              MaxShield 0 이면 Collapsed 로 줄 자체를 없앰
```

**전부 주면 안 된다.** 실드가 있는 적과 없는 적이 갈려야 "이놈은 단단하다"가 읽힌다.

적 실드는 **재생 안 한다.** `EnemyDefinition` 에 `StartupEffects` 배열이 없어 `GE_ShieldRegen` 을 받을 통로가 없다.

`Collapsed` 여야 자리까지 사라진다. `Hidden` 이면 빈 줄이 남아 검사·궁수 바가 위아래로 벌어진다.

### 여기서도 값이 100배로 들어갔다

`Fill Color and Opacity` 에 `(3.0, 75.0, 65.0)` 이 들어가 HDR 로 타서 하얗게 번졌다. **소수점이 빠진 것.** 색은 0~1 범위다.

---

## 13. ★ 오늘 두 번 오진했다 — 기록

```
"콤보 데미지 계수가 전부 0이라 HP 바가 안 움직인다"
   틀림. 데미지는 처음부터 정상. 계수라는 개념 자체가 없었다
   실제 = 모든 타격이 AttackPower(20) 고정. "0" 이 아니라 "전부 똑같다"

"GA_LightCombo 의 DamageEffectClass 가 비어 있다"
   틀림. 그건 런타임 작업용 변수라 CDO 가 비는 게 정상
   GA_PlayerAttackBase.cpp:21 이 매 활성화마다 DefaultDamageEffectClass 로 덮는다
   실제 값은 DefaultDamageEffectClass 에 처음부터 들어 있었다
```

**CDO 값만 보고 "안 물려 있다"고 단정한 게 원인이다.** 런타임에 대입되는 변수는 CDO 가 비어 있는 게 정상 상태다. 그 변수를 누가 채우는지 `.cpp` 를 먼저 봐야 한다.

---

## 14. 남은 것

```
타격별 계수        FComboNode 에 DamageMultiplier(float) 추가 -> 노드 26칸 채우기
                  SB 실측 계수표가 볼트에 있다. 전투 밸런싱 작업으로 별도 진행
실드 0 리액션      SB 의 ReactionWhenShieldZero. 지금은 실드 0이어도 아무 일 없음
전투 중 회복       SB 는 ShieldRegenPerSecondWhenBattle 를 따로 둔다. 우리는 전투 중 0
튜닝              ShieldDamageReduction 0.4 / MaxShield 50 / 회복 0.3 전부 체감 미검증
```
