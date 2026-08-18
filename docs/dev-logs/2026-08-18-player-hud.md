# 플레이어 HUD — 좌하단 HP + 탄약

화면에 고정으로 뜨는 첫 HUD. 크로스헤어 하나만 떠 있던 자리에 컨테이너를 세웠다.

**소스** — `AS_Player.h/.cpp` / `GE_AmmoCost.h/.cpp`(신규) / `GA_Shoot.h/.cpp` / `KDGameplayTags.h/.cpp`
**에셋** — `WBP_PlayerInfo` `WBP_MainHUD` `GE_AmmoRegen` `GE_GunRegenBlock`(신규) / `BP_SBPlayer` `BP_PlayerState` `GA_Shoot`
**설계 근거** — 볼트 `notes/Reference/StellarBlade_UI_HUD_구조.md`

---

## 1. 탄약을 GAS 어트리뷰트로

### `Dosul` 을 지우고 그 자리에

```
전    Stamina / MaxStamina / Dosul / MaxDosul     도술 = 길동 컨셉 잔재
후    Stamina / MaxStamina / Ammo  / MaxAmmo      30 / 30
```

`Dosul` 은 **에셋 참조가 0개**였다. GE 도 BP 도 아무도 안 읽고 있어서 그냥 지웠다.

어트리뷰트 개수는 4개 그대로다(§1-1 한도 8).

### 초기값이 들어가는 곳

```
AKDPlayerState 생성
 └ CreateDefaultSubobject<UAS_Player>("PlayerAttributes")     KDPlayerState.cpp:10
      └ UAS_Player 생성자 -> InitAmmo(30) / InitMaxAmmo(30)
```

적은 `DA_EnemyDef_*` 가 `PossessedBy` 때 스탯을 덮지만 **플레이어는 그런 게 없다.** 생성자 값이 곧 최종값.

총이 하나뿐이라 코드 상수로 둔다. 무기마다 장탄이 갈리면 그때 DA 로 승격(§3).

---

## 2. 소모 = Cost GE. 고정 `-1` 로

```cpp
// GE_AmmoCost.cpp
DurationPolicy = EGameplayEffectDurationType::Instant;
AmmoMod.Attribute = UAS_Player::GetAmmoAttribute();
AmmoMod.ModifierOp = EGameplayModOp::Additive;
AmmoMod.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-1.0f));
```

```cpp
// GA_Shoot.cpp:22
CostGameplayEffectClass = UGE_AmmoCost::StaticClass();
```

### SetByCaller 를 안 쓴 이유 — 동작이 다르다

기존 `GE_StaminaCost` 는 SetByCaller 방식인데 그걸 따라가지 않았다.

```
고정 -1        CheckCost 가 "Ammo - 1 >= 0" 을 계산할 수 있음  ->  0발이면 발동 자체가 막힘
SetByCaller    스펙 생성 시점에 값이 없으면 0으로 계산         ->  0발이어도 그냥 나감
```

`GA_Shoot.cpp:27` 의 `CommitAbility` 가 `CheckCost` + `ApplyCost` 를 같이 한다. **조건문을 우리가 안 써도 0발이면 몽타주조차 안 나온다.**

> ⚠️ `GE_StaminaCost` 는 C++ 어디에서도 안 쓰이고 BP 에만 물려 있다. **BP 로 Cost GE 를 물리면 SetByCaller 를 채울 자리가 없다.** 스태미나 0에서 회피가 나가는지 나중에 확인할 것.

### `GA_ShotBlast` 는 탄약을 안 먹는다

콤보 3타·5타에 섞인 총격이다. 여기서 탄약이 마르면 **근접 콤보가 중간에 끊긴다.** 연출로 보고 소모 대상에서 뺐다.

---

## 3. 구조 — 컨테이너 하나, 캔버스 하나

```
WBP_MainHUD                    뷰포트에 붙는 유일한 위젯
 └ Canvas_HUD
    ├ WBP_Crosshair            전체 화면 (Anchor 0,0 - 1,1 / Offset 0)      ZOrder 0
    └ WBP_PlayerInfo           좌하단                                       ZOrder 1
         └ VerticalBox
            ├ SizeBox_HP (400 x 20) > ProgressBar_HP
            └ HorizontalBox > TextBlock_Ammo / TextBlock_MaxAmmo
```

`BP_SBPlayer` BeginPlay 의 `Create Widget` **Class 핀 하나만** `WBP_Crosshair` -> `WBP_MainHUD` 로 바꿨다. `Add to Viewport` 가 받는 타입이 `UserWidget` 이라 선은 안 끊긴다.

### 크로스헤어는 옮겨도 그대로 돈다

`WBP_Crosshair` 는 Tick 에서 `Get Player Pawn` -> ASC 태그를 직접 읽어 표시·확산을 정한다. **누가 부모냐에 의존하지 않는다.** 중첩만 하면 끝.

### SB 는 캔버스가 3개인데 우리는 1개

띄울 게 두 개뿐이라 층을 나눌 게 없다. 화면 효과도 우리는 위젯이 아니라 포스트프로세스 머티리얼(`M_PP_ScreenEffect`)이다. 층이 실제로 필요해질 때 추가한다.

---

## 4. 값 전달 — 적 바와 같은 패턴

```
첫 값      GetFloatAttribute(Actor, Attribute)         델리게이트는 바뀔 때만 온다
값 변화    WaitForAttributeChanged(Actor, Attribute, OnlyTriggerOnce=false)
해제       EndAction()                                 Destruct 에서
```

대상은 **`Get Owning Player Pawn` 을 그대로 넘긴다.** `ABaseCharacter` 가 `IAbilitySystemInterface` 를 구현하고 `PossessedBy` 에서 PlayerState 의 ASC 를 캐시해 두기 때문(`KDPlayerCharacter.cpp:134`). PlayerState 를 따로 꺼낼 필요가 없다.

```
RefreshHealth   Branch(MaxHealthCache > 0) -> Set Percent ( NewValue / MaxHealthCache )
RefreshAmmo     NewValue -> To Text (Float) -> Set Text
```

`To Text (Float)` 는 `Minimum Fractional Digits = 0` 이라 `30.0` 이 `30` 으로 찍힌다. `Truncate` 를 따로 안 거쳐도 된다.

---

## 5. ★ 함정 — `Changed` 와 `Then` 은 다른 핀이다

이번에 실제로 물린 버그. `Wait for Attribute Changed` 의 **`Changed` 를 실행 흐름으로 썼다.**

```
Then       구독을 걸자마자 = 그 즉시 1회. 다음 노드로 넘어가는 선
Changed    값이 바뀔 때마다 = 나중에 여러 번. 갱신 작업을 다는 선
```

### 잘못된 배선이 만든 것

```
Wait(Health).Changed ──> Set AsyncHealth ──then──> Wait(Ammo)
Wait(Ammo).Changed   ──> Set AsyncAmmo
```

```
1  Construct 가 끝나도 AsyncHealth 가 null       Changed 가 와야 대입되니까
2  Wait(Ammo) 가 아예 생성 안 됨                 Health 가 먼저 변해야 그 줄에 도달
   -> 총을 쏴도 탄약이 안 바뀜
3  Refresh 함수가 변화 때 호출 안 됨              -> 바가 첫 값에서 굳음
4  Destruct 의 End Action 이 null 을 호출
```

**에러도 로그도 안 난다.** 첫 값은 Construct 에서 따로 읽으니 화면엔 정상으로 보인다.

### 옳은 배선

```
Wait.Then     ──> Set Async<X>        구독 객체 보관
Wait.Changed  ──> Refresh<X>          + NewValue 를 그 함수 인자로
```

`NewValue` 선을 빠뜨리면 갱신은 도는데 **0이 넘어간다.** 한 대 맞는 순간 HP 바가 0으로 떨어진다.

---

## 6. 실측 치수

```
SizeBox_HP        400 x 20            도트 10px 기준 40칸 x 2줄
브러시            T_Dot10 / Image Size 10x10 / Tiling Both / Draw As Image
Tint 배경         (0.08, 0.08, 0.08, 0.85)
Tint 채움         흰색
TextBlock_Ammo    폰트 21   흰색
TextBlock_MaxAmmo 폰트 12   (0.5,0.5,0.5)   VAlign Bottom / Padding Left 4
PlayerInfo 배치   Anchor (0,1) / Alignment (0,1) / Position (60, -60) / Size To Content
```

`Alignment Y = 1` 이 있어야 `Position Y -60` 이 "화면 바닥에서 60px 위"가 된다.

**슬롯 `Horizontal Alignment = Left`** — `VerticalBox` 기본값이 `Fill` 이라 그냥 두면 `Width Override` 가 무시되고 부모 폭까지 늘어난다(적 Poise 바에서 도트가 20개 나왔던 그 함정).

---

## 7. 검증 (PIE)

```
좌하단에 HP 도트 바 + "30 / 30"        통과
우클릭 조준 -> 크로스헤어               통과 (기존 동작 유지)
사격 -> 29 / 28 ...                    통과
적에게 맞음 -> HP 바 감소               통과
```

콤보 데미지 계수가 0이라 **내 공격으로는 HP 를 못 깎는다.** 적이 나를 때리는 쪽으로 검증했다.

---

## 8. 재장전 — 애니가 없어서 상태로만 만들었다

### 팩에 재장전 모션이 0개다

```
Gun_and_Sword AnimSequence 993개 중  reload / magazine / ammo / insert / cock / rack  = 0
프로젝트 전체(/Game) 검색                                                              = 0
```

`Equip` 8개는 걷기·달리기 중 무기 뽑기/집어넣기 전환이다. 사격 세트가 `Start / Loop / Shoot / Run_Shoot / Walk_Shoot / End` 6개뿐인 걸 보면 **팩이 총을 탄약 없는 에너지 무기로 상정하고 만들었다.**

동작을 못 만드니 **상태만으로** 재장전을 만든다.

### 부품 4개

```
State.Gun.Reloading      Ammo 0 에서 켜짐 / 만충에서 꺼짐        AS_Player::PostGameplayEffectExecute
                         GA_Shoot 의 ActivationBlockedTags       0발이면 발동 자체를 막음
State.Gun.RegenBlocked   GE_GunRegenBlock (HasDuration 2.0)      GA_Shoot 이 사격마다 적용
GE_AmmoRegen             Infinite / Period 0.1 / Ammo +1         Ignore Tags = State.Gun.RegenBlocked
```

```
25발에서 멈춤   ->  2초 뒤 회복 시작  ->  0.5초만에 30발
0발 도달        ->  사격 차단 + 2초 대기  ->  3초 회복  ->  만충에서 차단 해제
```

**회복 조건과 사격 차단이 서로 다른 태그다.** 처음엔 `Require Tags = State.Gun.Reloading`(0일 때만 회복)으로 짰다가, "쏘다 멈추면 차게" 로 바꾸면서 `Ignore Tags = State.Gun.RegenBlocked` 로 뒤집었다. 차단은 그대로 두면 돼서 GE 하나로 두 동작이 다 나온다.

### 노브가 하나로 묶여 있다

`GE_AmmoRegen` 의 `Scalable Float Magnitude` 가 **재장전 속도와 유휴 회복 속도를 동시에** 정한다.

```
1.0 (초당 10발)   0->30 = 3초.  유휴 회복이 후한 편
0.5 (초당 5발)    0->30 = 6초.  유휴는 적당하나 재장전이 길다
```

나누고 싶으면 GE 를 둘로(빠른 재장전용 / 느린 유휴용) 쪼개야 한다. 지금은 안 쪼갠다.

### `AddLooseGameplayTag` 말고 `SetLooseGameplayTagCount`

앞엣것은 부를 때마다 카운트가 쌓인다. 같은 조건이 두 번 걸리면 태그가 안 지워진다. 뒤엣것은 몇 번을 불러도 결과가 같다(`AbilitySystemComponent.h:674`).

### ★ UE5.3+ 에서 GE 태그 조건은 컴포넌트로 옮겨갔다

`GE_AmmoRegen` 에 조건을 아무리 찾아도 칸이 없었다. **컴포넌트를 안 붙이면 조건 칸 자체가 없다.**

```
디테일 패널 아래 [+ 컴포넌트 추가]
 └ 타깃 태그 요구 사항 (Target Tag Requirements)
      지속되는 태그 요구 사항 > 필수 태그 / 무시 태그
```

구버전 속성(`UGameplayEffect::OngoingTagRequirements`)은 아직 읽히지만 **에디터가 쓰는 곳은 `GEComponents` 안이다.** 파이썬으로 검증할 때 구버전 속성만 보면 "비어 있다"고 오판한다.

### ★ BP 가 C++ 기본값을 안 물려받는 경우가 있다

```
C++ CDO   ActivationBlockedTags = State.Gun.Reloading
BP  CDO   ActivationBlockedTags = (비어 있음)
```

같은 생성자에서 지정한 `CostGameplayEffectClass` 는 정상으로 내려왔는데 **`FGameplayTagContainer` 만 BP 에 빈 값이 박혀 있었다.** 컨테이너 계열은 BP 컴파일 시점에 통째로 직렬화돼서 이후 C++ 기본값 변경이 안 따라온다.

증상이 조용하다 — 0발에서 "완전 차단" 대신 "2.1초마다 한 발"이 된다. 에러도 로그도 없다.

→ BP 디테일에서 해당 속성의 **되돌리기 화살표(↺)** 를 눌러 상속을 복구한다.

---

## 9. 남은 것

```
스태미나 표시 X    실제로 소모되는데(회피·강공·풀스프린트) 화면에 없다. 3줄로 늘릴 때 같이
Shield (SH)       좌하단 3층 중 맨 아래. 전투 시스템 작업이라 나중
콤보 데미지 계수 0  HP 바가 내 공격에 안 움직이는 진짜 이유
튜닝              MaxAmmo 30 / Regen Magnitude 1.0 / RegenBlock Duration 2.0 은 전부 체감 미검증
잔가지            WBP_PlayerInfo 루트 이름 오타(VeticalBox) / 빈 Event Tick 노드 / Percent 0
```
