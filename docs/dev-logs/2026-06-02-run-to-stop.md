# 달리다 정지 (Run-to-Stop) — 질주 정지 (2026-06-02)

**상태**: ✅ PIE 검증 통과. 질주(≥600) 정지 시 브레이크 모션 + 캡슐 감속 정합 완료.

## 작업 요약

달리다 멈출 때 Locomotion → Idle로 **톡 끊기는** 느낌 제거. 감속하며 멈추는 정지 애니 추가.
velocity-driven 구조라 **캡슐(물리)은 미끄러지며 멈추고, 메시(애니)는 정지 동작 재생** → 둘의 멈추는 시간을 맞춰 자연스럽게. Distance Matching 같은 복잡한 길 회피(방식 B = 감속값 튜닝 근사).

**스코프(PIE 후 축소)**: 일반 `Stop`(500급) 폐기 → **`Stop_Fast`(질주 정지)만 유지.** 보통 달리기(500) 정지는 톡 끊겨도 전투 반응성상 OK, 질주(700/800) 급정거만 어색해 브레이크 필요. 부작용(수용): 보통 W 달리기(≈500) → 키 떼면 정지 클립 없이 즉시 Idle.

## 4개 부품

| # | 부품 | 역할 | 작업 |
|---|---|---|---|
| ① | 정지 클립 | 메시의 멈추는 동작 (in-place) | 리타겟 |
| ② | 캡슐 감속 | 미끄러지는 거리 (물리) | `BrakingDecelerationWalking` 튜닝 |
| ③ | 트리거 신호 | "입력 뗌 + 속도 잔존" 감지 | C++ `bHasMovementInput` |
| ④ | ABP 상태 | 신호 받아 정지 클립 재생 | `Stop_Fast` 상태 |

## ① 리타겟 (in-place + Force Root Lock)
- 소스 `AS_Run_Fast_Stop_Seq`(`Content\Spear\...\04_Run\11_Run_Fast\`) → `AS_Run_Fast_Stop_Kildong`, 저장 `...\Player\Animation\Sequence\Move\Peaceful\`
- (`AS_Run_F_0_Stop_Seq` → `AS_Run_F_0_Stop_Kildong`도 리타겟됨 — 일반 Stop 폐기로 **현재 미사용**, 잔존)
- ⚠️ **Turn 클립과 정반대**: 정지는 **Enable Root Motion OFF + Force Root Lock ON**. 클립의 감속 전진(루트 baked)을 제거 → 캡슐이 감속 담당, 메시는 제자리 정지 동작만. (Turn은 루트모션 ON/Lock OFF로 회전을 캡슐에 전달)
- 핵심 한 줄: **"움직임을 클립이 만드냐(루트모션 ON) vs 물리가 만드냐(Force Root Lock ON)"** — 정지는 물리가 만듦

## ② 캡슐 감속 튜닝 — 방식 B 핵심 ★ **확정값**
- 위치: **BP_PlayerCharacter → CharacterMovement 컴포넌트 → Walking**
- **`Braking Deceleration Walking = 1900`** (엔진 기본 2048 → 1900). PIE에서 발 접지 보며 맞춘 값
- 원리: 멈추는 시간 = 시작속도 ÷ BrakingDecelerationWalking. 2048(기본)은 700÷2048≈0.34s로 급정거 → 살짝 낮춰 미끄러짐 부여
- (Braking Friction 분리는 결국 불필요 — 1900 단일 값으로 충분히 자연스러웠음)

## ③ C++ `bHasMovementInput` (승환 작성, 빌드 통과)
`KDPlayerAnimInstance.h` public:
```cpp
// 이동 입력(가속도) 유무. run-to-stop 트리거용.
UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
bool bHasMovementInput = false;
```
`.cpp` `NativeThreadSafeUpdateAnimation`(워커) 한 줄 — 기존 `AccelDir` 재사용:
```cpp
bHasMovementInput = !AccelDir.IsNearlyZero(); // 입력 있으면 true
```
> 왜 필요: `GroundSpeed`만으론 "천천히 가는 중"과 "감속 중" 구분 불가. `MovementInputAngle`은 정전진 시에도 0이라 입력 유무 구분 불가. → "입력 뗌(false) + 속도 잔존(GroundSpeed 높음)" = 정지 시작 신호.

## ④ ABP Locomotion SM — `Stop_Fast` 상태
- 상태 1개: `Stop_Fast`(`AS_Run_Fast_Stop_Kildong`), **Loop OFF**(자동 규칙 발동 위해 필수)
- **진입**: `NOT(bHasMovementInput) AND (GroundSpeed >= 600)`
- **탈출 1**: `bHasMovementInput == true`, **우선순위 0(최우선)**, 블렌드 0.1s — 멈추다 재가속 시 안 갇힘
- **탈출 2**: **자동 규칙**(클립 끝) — 정상 정지 완료

## 동작 흐름
```
Shift 질주(≈700) → 키 뗌
  bHasMovementInput=false + GroundSpeed≈700  → Stop_Fast 진입
  메시: 정지 클립 재생(Force Root Lock 제자리)
  캡슐: BrakingDeceleration 1900으로 미끄러지다 멈춤
  도중 재입력 → 탈출 1 → 즉시 Run / 끝까지 → 자동규칙 → Idle
```

## PIE 검증 결과
- [x] 질주(≥600) 중 키 뗌 → `Stop_Fast` 재생, 발 미끄러짐 없이 자연 정지 (1900)
- [x] 보통 달리기(≈500) → 정지 클립 없이 즉시 Idle (의도대로)
- [x] 정지 모션 중 재입력 → 즉시 Run 복귀 (안 갇힘)
- [x] 정지 클립 끝 → Idle 자연 복귀

## 산출물
- `Source/Project_KD/Player/KDPlayerAnimInstance.h/.cpp` (`bHasMovementInput` 추가)
- `Content/.../Animation/ABP_Player` (Locomotion SM `Stop_Fast` 상태)
- killdong 리타겟: `AS_Run_Fast_Stop_Kildong` (+ 미사용 `AS_Run_F_0_Stop_Kildong`)
- BP_PlayerCharacter CharacterMovement `Braking Deceleration Walking = 1900`
