---
title: UE5 GAS Action Game — Naming Convention
version: 1.1
last_updated: 2026-05-14
scope: Unreal Engine 5 + GameplayAbilitySystem 기반 싱글플레이 액션 게임
status: Stable
tags: [convention, ue5, gas, naming]
---

# UE5 GAS Action Game — Naming Convention

> 본 문서는 UE5 + GameplayAbilitySystem(GAS) 기반 **싱글플레이 액션 게임**에서 **에셋·C++ 클래스·변수·GameplayTag·폴더 구조**의 명명 규칙을 정의한다.
> Epic 공식 표준 기반.

---

## 0. 핵심 원칙

| # | 원칙 | 이유 |
|---|------|------|
| 1 | **Epic 표준 접두사 엄수** (`U/A/F/E/I/T`) | GC 안정성·리플렉션 호환성. |
| 2 | **에셋명만으로 타입 식별 가능** | 콘텐츠 브라우저 검색·필터링 효율. |
| 3 | **계층은 카테고리 → 디테일** | `Ability.Combat.Attack.Light` 식. 자동완성 친화. |
| 4 | **GameplayTag·상수는 중앙 집중** | `NativeGameplayTags.h` 1개 파일에서 관리. |
| 5 | **표시명 영어, 주석 영어, 식별자 영어** | i18n·외부 협업 대비. |

---

## 1. 에셋 파일 접두사

### 1.1 Mesh & Material
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `T_` | Texture | `T_Sword_Albedo` |
| `SM_` | StaticMesh | `SM_Crate_01` |
| `SK_` | SkeletalMesh | `SK_Hero_Default` |
| `M_` | Master Material | `M_Character_Master` |
| `MI_` | MaterialInstance | `MI_Hero_Costume_01` |
| `MF_` | MaterialFunction | `MF_DissolveEffect` |
| `PM_` | PhysicalMaterial | `PM_Metal_Heavy` |

### 1.2 Blueprint & UI
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `BP_` | Blueprint (Actor 파생) | `BP_Hero` |
| `WBP_` | Widget Blueprint (UMG) | `WBP_HUD_Main` |
| `PC_` | PlayerController BP | `PC_Hero` |
| `GM_` | GameMode BP | `GM_Combat` |
| `GS_` | GameState BP | `GS_Combat` |
| `PS_` | PlayerState BP | `PS_Hero` |

### 1.3 Animation
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `ABP_` | AnimBlueprint | `ABP_Hero` |
| `AM_` | AnimMontage | `AM_Attack_Light_01` |
| `AS_` | AnimSequence | `AS_Idle_Loop` |
| `BS_` | BlendSpace | `BS_Locomotion` |
| `AN_` | AnimNotify (커스텀) | `AN_FootStep` |

> ⚠️ 에셋 `AS_` = **AnimSequence**.
> C++ AttributeSet 클래스는 풀네임 사용 (`U{Prefix}CombatAttributeSet`) → 충돌 회피.

### 1.4 GAS (Gameplay Ability System) ⭐
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `GA_` | GameplayAbility | `GA_Attack_Light` |
| `GE_` | GameplayEffect | `GE_Damage_Slash` |
| `GC_` | GameplayCueNotify | `GC_Hit_Sword` |

### 1.5 AI
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `BT_` | BehaviorTree | `BT_Grunt` |
| `BB_` | Blackboard | `BB_Humanoid` |
| `BTT_` | BTTask | `BTT_StrafeAroundTarget` |
| `BTS_` | BTService | `BTS_UpdateThreatLevel` |
| `BTD_` | BTDecorator | `BTD_HasLineOfSight` |
| `EQS_` | EnvQuery | `EQS_FindCoverPoint` |

### 1.6 Data
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `DA_` | DataAsset | `DA_Weapon_LongSword` |
| `PD_` | PrimaryDataAsset | `PD_HeroLoadout` |
| `DT_` | DataTable | `DT_AbilityCooldowns` |
| `CT_` | CurveTable | `CT_LevelXP` |

### 1.7 Input / FX / Audio / Cinema
| 접두사 | 타입 | 예시 |
|--------|------|------|
| `IA_` | InputAction (Enhanced Input) | `IA_Attack` |
| `IMC_` | InputMappingContext | `IMC_DefaultGameplay` |
| `NS_` | NiagaraSystem | `NS_BloodSpray` |
| `NE_` | NiagaraEmitter | `NE_Spark` |
| `SCue_` | SoundCue | `SCue_Sword_Swing` |
| `SC_` | SoundClass | `SC_SFX_Combat` |
| `LS_` | LevelSequence | `LS_Intro_Boss01` |

---

## 2. C++ 클래스 네이밍

### 2.1 Unreal 표준 접두사 (필수)
| 접두사 | 대상 | 예시 |
|--------|------|------|
| `U` | UObject 파생 | `UCombatComponent` |
| `A` | AActor 파생 | `AHeroCharacter` |
| `F` | 구조체·값 타입 | `FDamageContext` |
| `E` | Enum | `EAttackType` |
| `I` | Interface | `IInteractable` |
| `T` | Template | `TRingBuffer<T>` |

### 2.2 프로젝트 Prefix
- 모든 게임 모듈 클래스는 **프로젝트 prefix(2–3자 PascalCase)** 를 Unreal 접두사 다음에 부착.
- 예: `Op` (Omega Protocol), `If` (IF Action), `Pj` (PlayerName 등).
- 형식: `{UnrealPrefix}{ProjectPrefix}{ClassName}`

```cpp
// 예: 프로젝트 prefix = Op
class UOpAbilitySystemComponent : public UAbilitySystemComponent { ... };
class AOpHeroCharacter         : public ACharacter { ... };
struct FOpDamageContext { ... };
enum class EOpAttackType : uint8 { Light, Heavy, Charged };
```

### 2.3 GAS 클래스 명명 표준
| 책임 | 명명 패턴 | 예시 |
|------|-----------|------|
| ASC 확장 | `U{Prefix}AbilitySystemComponent` | `UOpAbilitySystemComponent` |
| AttributeSet 베이스 | `U{Prefix}AttributeSet` | `UOpAttributeSet` |
| AttributeSet 도메인별 | `U{Prefix}{Domain}AttributeSet` | `UOpCombatAttributeSet` |
| GameplayAbility 베이스 | `U{Prefix}GameplayAbility` | `UOpGameplayAbility` |
| GE Context 확장 | `F{Prefix}GameplayEffectContext` | `FOpGameplayEffectContext` |
| AbilityTask 커스텀 | `U{Prefix}AbilityTask_{Action}` | `UOpAbilityTask_WaitTargetData` |

---

## 3. 변수 & 함수

### 3.1 변수
- **bool**: `b` 접두사 → `bIsInCombat`, `bCanDodge`
- **멤버 변수**: PascalCase → `CurrentHealth`, `OwningAbility`
- 모든 UObject 포인터 멤버는 `UPROPERTY()` 필수 (GC 안전성)
- 매직 넘버 금지 → `UPROPERTY(EditDefaultsOnly)` 또는 DataAsset

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Combat")
float BaseAttackDamage = 10.0f;

UPROPERTY(VisibleAnywhere, Category = "State")
bool bIsInCombat = false;
```

### 3.2 함수
| 접두사 | 용도 | 예시 |
|--------|------|------|
| `Get` | 순수 반환 | `GetCurrentHealth()` |
| `Set` | 값 변경 | `SetMaxHealth(float)` |
| `Is` / `Has` / `Can` | bool 반환 | `IsAlive()`, `CanDodge()` |
| `Try` | 실패 가능 작업 | `TryActivateAbility()` |
| `Handle` | 이벤트 응답 | `HandleAbilityCommitted()` |
| `On` | Delegate 콜백 | `OnHealthChanged(...)` |

### 3.3 Delegate
- 시그니처 타입: `FOn{Event}Signature`
- 인스턴스: `On{Event}`

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnHealthChangedSignature, float, OldHealth, float, NewHealth);

UPROPERTY(BlueprintAssignable)
FOnHealthChangedSignature OnHealthChanged;
```

### 3.4 GAS 특수 함수 규약
| 함수 | 책임 | 비고 |
|------|------|------|
| `ATTRIBUTE_ACCESSORS` 매크로 | Getter/Setter 자동 생성 | 모든 attribute 필수 |
| `PreAttributeChange` | 클램프 전용 | 비즈니스 로직 금지 |
| `PostGameplayEffectExecute` | 후처리 (사망·알림 등) | 최종 값 변경 후 실행 |

---

## 4. GameplayTag 계층

### 4.1 카테고리 표준
```
Ability.{Domain}.{Action}.{Variant}
    Ability.Combat.Attack.Light
    Ability.Combat.Attack.Heavy
    Ability.Combat.Dodge
    Ability.Movement.DoubleJump
    Ability.Parkour.WallRun

State.{Domain}.{Condition}
    State.Combat.InAction
    State.Combat.Invulnerable
    State.Movement.Sprinting
    State.Death

Effect.{Type}.{Source}
    Effect.Damage.Slash
    Effect.Damage.Fire
    Effect.Buff.AttackUp
    Effect.Debuff.Bleed

Event.{Phase}.{Action}
    Event.Montage.SendEvent.Hit
    Event.Montage.End
    Event.Combat.EnemyKilled

Input.{Action}
    Input.Move
    Input.Attack.Light
    Input.Attack.Heavy

Cue.{Type}.{Specific}
    Cue.Combat.Hit.Sword
    Cue.Combat.Block.Metal

Data.{Domain}.{Type}
    Data.Damage.Type.Physical
    Data.Damage.Type.Fire
```

### 4.2 선언 규칙
- **단일 진실의 원천**: `Source/{Project}/{Project}GameplayTags.h` 또는 `NativeGameplayTags.h`에 중앙 선언.
- 헤더별 분산 선언 금지.
- 어빌리티 차단용 태그(`BlockedTags`)는 `State.*` 계층 사용.

```cpp
// Source/Op/OpGameplayTags.h
namespace OpGameplayTags
{
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Ability_Combat_Attack_Light)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(State_Combat_InAction)
    UE_DECLARE_GAMEPLAY_TAG_EXTERN(Event_Montage_End)
}
```

---

## 5. 폴더 구조 권장

```
Content/
├── Characters/
│   ├── Hero/
│   │   ├── Mesh/             # SK_, SM_
│   │   ├── Animation/        # ABP_, AM_, AS_, BS_
│   │   ├── Materials/        # M_, MI_
│   │   └── BP_Hero
│   └── Enemies/{EnemyName}/
├── GAS/
│   ├── Abilities/            # GA_
│   ├── Effects/              # GE_
│   ├── Cues/                 # GC_
│   ├── AttributeSets/
│   └── Tags/                 # NativeGameplayTags ↔ DataAsset 동기화
├── Items/
│   ├── Weapons/{WeaponType}/
│   └── Consumables/
├── UI/
│   ├── HUD/                  # WBP_
│   ├── Menu/
│   └── Common/
├── AI/
│   ├── BehaviorTrees/        # BT_, BB_
│   ├── Tasks/                # BTT_, BTS_, BTD_
│   └── EQS/
├── Input/                    # IA_, IMC_
├── FX/
│   ├── Niagara/              # NS_, NE_
│   └── Materials/
├── Audio/                    # SCue_, SC_
├── Data/                     # DA_, PD_, DT_, CT_
├── Maps/
└── Cinema/                   # LS_
```

---

## 6. 안티 패턴 (Quick Reference)

| ❌ 금지 | ✅ 권장 | 이유 |
|--------|---------|------|
| `MyAbility.uasset` | `GA_Attack_Light.uasset` | 타입 식별 불가 |
| `Tag.Attack` (단일 레벨) | `Ability.Combat.Attack.Light` | 자동완성·필터링 |
| `IsAlive_Bool` | `bIsAlive` | UE 표준 위배 |
| `DoAttack()` (임의 함수명) | `TryActivateAbilityByTag(...)` | UE 함수 명명 규약 위배 |
| 헤더마다 GameplayTag 선언 | `{Project}GameplayTags.h` 중앙화 | 중복·충돌 위험 |
| 폴더 분류 없는 `Content/` 평면 구조 | `Content/GAS/Abilities/...` 계층 | 검색·유지보수 비용 |
| `MyClass.h` (프로젝트 prefix 없음) | `{Prefix}MyClass.h` | 모듈 식별 불가 |

---

## 7. 빠른 참조 카드

### 7.1 자주 쓰는 GAS 에셋 명명 템플릿
```
GA_{Domain}_{Action}_{Variant}        → GA_Combat_Attack_Light
GE_{Type}_{Source}_{Optional}         → GE_Damage_Slash_Critical
GC_{Category}_{Subject}               → GC_Hit_Sword
AM_{Action}_{Variant}                 → AM_Attack_Light_01
DA_{Category}_{ItemName}              → DA_Weapon_LongSword
DT_{Domain}_{Purpose}                 → DT_Ability_Cooldowns
```

### 7.2 신규 어빌리티 추가 체크리스트
- [ ] `GA_{Domain}_{Action}` 에셋 생성 (`Content/GAS/Abilities/`)
- [ ] `Ability.{Domain}.{Action}` GameplayTag를 `{Project}GameplayTags.h`에 추가
- [ ] BlockedTags / RequiredTags 정의
- [ ] `IA_{Action}` InputAction 매핑
- [ ] 필요 시 `AM_{Action}` AnimMontage 연결
- [ ] 데미지 발생 시 `GE_Damage_{Source}` 생성
- [ ] 시청각 피드백 시 `GC_{Category}_{Subject}` 연결

---

## 참고 자료

- Epic Games — [Unreal Engine Coding Standard](https://dev.epicgames.com/documentation/en-us/unreal-engine/epic-cplusplus-coding-standard-for-unreal-engine)
- Epic Games — [Asset Naming Convention](https://github.com/Allar/ue5-style-guide)
- Tranek — [GAS Documentation (Community)](https://github.com/tranek/GASDocumentation)
- 내부 분석: [[FORGE/Game-Analysis/Nakwon/02-에셋-통계.md]] (어비스 오브 더 와일드 패턴)

---

> **변경 이력**
> - 1.1 (2026-05-14) — 싱글플레이 기준 정리. 멀티플레이·리플리케이션·RPC 항목 제거, GAS 설계 규약 섹션 제거, 네이밍·폴더 구조 중심으로 슬림화.
> - 1.0 (2026-05-14) — 초안 작성. 어비스 분석 + UE5 표준 + GAS 베스트 프랙티스 통합.
