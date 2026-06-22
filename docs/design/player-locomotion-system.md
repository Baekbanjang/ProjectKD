# Player Locomotion System — Project_KD

> 플레이어 전용 기본 이동 애니메이션(로코모션). 2026-05-30 도입.
> Stellar Blade 실데이터 분석 + Deep Interview(3R) 결정 → C++ 데이터 공급 / 사람이 BlendSpace·ABP 그래프.
> 스코프 = Peaceful(탐험) + Battle(전투자세) + LockOn(strafe) + 공중. PIE 검증 통과.

## 핵심 원칙 — 디커플링/확장성/독립성

| 원칙 | 적용 |
|---|---|
| **디커플링** | ABP가 `GetVelocity`/태그 직접 쿼리 안 함 → `UKDPlayerAnimInstance`가 노출 변수만 공급. PlayerCharacter.h는 sprint getter 2개 외 무변경 |
| **확장성** | Battle 전환은 AnimInstance 자체 계산(`State.Combat.Attacking` 태그 감시) → 콤보/도술/피격이 태그만 켜면 자동 Battle 진입 |
| **독립성** | 로코모션 데이터 산출은 ASC 태그·MovementComponent 읽기만(단방향). 다른 시스템 무지 |

## 아키텍처

| 구성 요소 | 책임 | 위치 |
|---|---|---|
| `UKDPlayerAnimInstance` | ABP 데이터 공급. 게임스레드 스냅샷 → 워커스레드 float 산출 | `Source/Project_KD/Player/KDPlayerAnimInstance.h/.cpp` |
| `APlayerCharacter` sprint getter | `IsSprinting()` / `IsFullSprinting()` 노출 (인라인) | `Source/Project_KD/Player/PlayerCharacter.h:96-97` |
| `ABP_Player` | 스테이트 머신 + BlendSpace 그래프 (사람 작업) | `Content/Blueprints/Player/Animation/ABP_Player` |
| BlendSpace 3종 | Peaceful/Battle(1D Speed) + LockOn(2D VelocityX/Y) | `Content/.../Animation/Sequence/BlendSpace/` |
| `State.Character.LockOn` 태그 | 락온 상태 (LockOnComponent가 토글) | `KDGameplayTags.h` |
| `State.Combat.Attacking` 태그 | 공격 중 (Battle 타이머 갱신용) | `KDGameplayTags.h` |

## 노출 변수 (BlueprintReadOnly) — 9개

| 변수 | 산출 | 용도 |
|---|---|---|
| `GroundSpeed` (float) | `CachedVelocity.Size2D()` | Peaceful/Battle 1D BS Speed 축 |
| `VelocityX` (float) | `UnrotateVector(Velocity).Y` (로컬 좌우) | LockOn 2D BS 가로축 |
| `VelocityY` (float) | `UnrotateVector(Velocity).X` (로컬 전후) | LockOn 2D BS 세로축 |
| `bIsLockedOn` (bool) | ASC `HasMatchingGameplayTag(State.Character.LockOn)` | Free↔LockOn 전환 |
| `bIsInBattleStance` (bool) | `now - LastCombatTime < BattleStanceHoldSec` | Peaceful↔Battle 전환 |
| `bIsInAir` (bool) | `MovementComp->IsFalling()` | 지상↔공중 전환 (Main States SM) |
| `bIsSprinting` (bool) | `OwningPlayer->IsSprinting()` | (참고용) |
| `bIsFullSprinting` (bool) | `OwningPlayer->IsFullSprinting()` | (참고용) |
| `BattleStanceHoldSec` (float, EditAnywhere) | 기본 3.0 (Clamp 0.5~10) | Battle 유지시간 BP 튜닝 |

> **LeanAmount는 구현 안 됨** — 플랜 초안엔 Free 2D Lean 축이 있었으나 최종 구현은 1D Speed로 단순화. Peaceful/Battle BlendSpace는 1D.

## 스레드 분리 (결정 #7)

```
NativeInitializeAnimation   — OwningPlayer / MovementComp 캐싱
        ↓
NativeUpdateAnimation (게임스레드) — UObject 접근 전부 여기
    · CachedVelocity = GetVelocity()
    · CachedActorRotation = GetActorRotation()
    · bIsInAir = MovementComp->IsFalling()
    · bIsSprinting / bIsFullSprinting = getter
    · ASC 태그: bIsLockedOn, State.Combat.Attacking → LastCombatTime 갱신
    · bIsInBattleStance = (now - LastCombatTime) < BattleStanceHoldSec
        ↓
NativeThreadSafeUpdateAnimation (워커스레드) — 순수 float 연산만
    · GroundSpeed = CachedVelocity.Size2D()
    · LocalVelocity = CachedActorRotation.UnrotateVector(CachedVelocity)
    · VelocityY = LocalVelocity.X (전후) / VelocityX = LocalVelocity.Y (좌우)
```

## ABP 구조 (사람 작업, PIE 통과)

```
[Locomotion SM] ──→ [캐시 포즈 "Locomotion"]              ← 지상 이동
[Main States SM] ──→ [Slot DefaultSlot] ──→ [Control Rig IK] ──→ [Output Pose]
                                              ▲
                        (Is in Air → NOT)─────┘ Should Do IKTrace
```

### Locomotion SM (지상 3상태)

```
        Entry
          │
          ▼
     ┌──────────┐  ──(IsLockedOn, p1)──→  ┌─────────┐
     │ Peaceful │  ←─(!Locked & !Battle)─  │ LockOn  │
     └──────────┘                          └─────────┘
        │   ▲                               ▲   │
   (Battle)(!Battle)              (IsLockedOn)(!Locked & Battle)
        ▼   │                               │   ▼
     ┌──────────┐  ──(IsLockedOn, p1)────────────┘
     │  Battle  │
     └──────────┘
```

| 상태 | BlendSpace | 입력 핀 |
|---|---|---|
| Peaceful | `BS_Player_Loco_Peaceful` (1D) | Speed ← GroundSpeed |
| Battle | `BS_Player_Loco_Battle` (1D) | Speed ← GroundSpeed |
| LockOn | `BS_Player_Loco_LockOn` (2D) | X ← VelocityX, Y ← VelocityY |

전환 Duration 0.15~0.2s. 우선순위: LockOn > Battle.

### Main States SM (공중 처리 — 템플릿 재사용)

```
Entry → Locomotion(캐시 포즈) ←→ (To Land → Land)
            ↓ Is in Air==true
       (To Falling → Fall Loop → Jump)
```

| 상태 | 애님 |
|---|---|
| Jump | `AS_Jump_Start_0_Kildong` (루프 X) |
| Fall Loop | `AS_Jump_Loop_0_Kildong` (루프 O) |
| Land | `AS_Jump_End_0_Kildong` (루프 X) |

핵심: `Locomotion` 상태 1순위 탈출 전환 = `Is in Air == true`.

## SB 로코모션 아키텍처 (실데이터 = 설계 기준)

방식: **클래식 BlendSpace + 스테이트 머신** (모션 매칭 아님). SB는 GAS 미사용이나 로코모션은 순수 UE 패턴이라 차용. SB는 커스텀 `SBAnimation` 플러그인의 `AnimNode_SBOverriddenBlendSpacePlayer`로 몽타주가 BS를 덮어쓰지만, **우리는 stock UE Slot 노드 + (후속) Layered Blend Per Bone로 동일 효과 재현**.

| 상태 | SB BlendSpace | 축 | 우리 정합 |
|---|---|---|---|
| 탐험 | `IdleRun_BS_Peaceful2D` | Speed 0~800, Grid 30 | 1D Speed (Lean 생략) |
| 전투자세 | `IdleRun_BS_Battle` | Speed 0~800 | 1D Speed |
| 락온 | `LockOn_IdleRun_BS` | LeftRight −1~1 | 2D VelocityX/Y |
| 공중 | `Airborne_BS` | 없음 | 단일 시퀀스 (BS 불요) |

SB AnimBP 노출 변수 `SBVelocityX/SBVelocityY`(로컬 속도 2축) 확인 = 우리 strafe 축 설계 근거.

## 딥 인터뷰 결정사항 (확정)

1. 스코프 = Peaceful + Battle + LockOn + 공중
2. ~~Free BS 2D(Speed+Lean)~~ → **최종 1D Speed** (Lean 구현 보류)
3. LockOn strafe 축 = VelocityX/VelocityY 2축 (SB 정합)
4. 공중(bIsInAir) 포함
5. Battle 전환 = AnimInstance 자체 계산 (`State.Combat.Attacking` → LastCombatTime). PlayerCharacter.h 무변경 (디커플링)
6. Battle 유지시간 = 3.0초 (EditAnywhere)
7. ThreadSafe + 태그/getter는 게임스레드 캐싱
8. sprint = PlayerCharacter getter 2개만 .h 변경

## 속도 기준 (PlayerCharacter.h 실값)

| 항목 | 값 | 비고 |
|---|---|---|
| WalkSpeed | 550 | 기본 MaxWalkSpeed |
| SprintSpeed | 900 | Shift 홀드 |
| FullSprintSpeed | 1100 | 4초+ 홀드 → BS Speed 축 최대값 |

> 코드엔 walk/run 상태 구분 없음. 아날로그 입력 × MaxWalkSpeed 캡. BS 중간 샘플(Walk)은 디자인 선택.

## 검증 — PIE (통과)

- [x] 정지/이동/Sprint → GroundSpeed 반영, bIsSprinting/bIsFullSprinting 토글
- [x] 공격 → bIsInBattleStance true 3초 유지 → Peaceful 복귀
- [x] MMB 락온 → bIsLockedOn true → LockOn BS, VelocityX/Y 8방향 게걸음 (양호)
- [x] 락온 해제 → Battle/Peaceful 복귀
- [x] 점프 → bIsInAir true → Main States Jump/Fall/Land

## 트러블슈팅 메모

### 함정 1 — C++ 상속 변수가 ABP "변수" 패널에 안 보임

`UKDPlayerAnimInstance`의 `UPROPERTY` 변수는 부모 클래스 소속이라 기본 숨김. **"내 블루프린트" 패널 톱니(⚙) → "상속된 변수 표시" 체크** 필요. 또는 그래프 우클릭 → 변수명 검색 → Get.

### 함정 2 — 템플릿 BP 변수 vs C++ 변수 혼동

옛 ABP 템플릿이 자체 선언한 `GroundSpeed_0`(끝에 `_0`)·`ShouldMove`·`IsFalling`은 템플릿 EventGraph가 채우던 잔재. **반드시 C++ 변수**(`GroundSpeed`, `VelocityX/Y`, `Is in Air`)를 BS/전환에 써야 ThreadSafe 산출과 일치.

### 함정 3 — Main States(점프 SM) 삭제 금지

템플릿의 `Main States` 스테이트 머신이 Jump/Fall/Land = Airborne 처리기. 삭제하면 공중 포즈 깨짐. Locomotion SM는 지상 3상태만, 공중은 Main States 재사용.

## 미구현 / 후속

| 항목 | 비고 |
|---|---|
| 기본 이동(Peaceful) 자연스러움 폴리싱 | BS 샘플 보간/속도 구간 중간 샘플 추가 검토 |
| Slot 미리 분할 | 상하체 Layered Blend Per Bone / 공격·도술·피격 슬롯 분리 |
| LeanAmount (Free 2D Lean) | 컨트롤러 Yaw 델타 구동 — 필요성 확인 후 도입 |
| LockOn BS 2단(저속/고속) | LockOn/Walk 10개 killdong 리타겟 완료 → 확장 가능 |
| 피격 기반 Battle 갱신 | hit/stagger 태그 추가 감시 (현재 Attacking만) |
| 점프 입력 배선 | bIsInAir 노출까지만, 입력 자체는 별도 |
| 전이 MoveSpeedCurve | SB MoveSpeedCurve식 speed-warp — 필요성 PIE 확인 후 (YAGNI) |

## 관련 자료

- `memory/reference_sb_analysis_docs_2026-05-27.md` — SB 내부 아키텍처 분석
- `memory/reference_sb_feel_observations.md` — Movement/Attack/Camera 조작감 매핑
- `docs/design/lockon-system.md` — 락온 시스템 (State.Character.LockOn 출처)
- `.omc/dev-logs/2026-05-30-player-locomotion.md` — 구현 dev-log
