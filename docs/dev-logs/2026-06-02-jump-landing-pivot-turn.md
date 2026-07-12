# 전진 점프 속도별 착지 + 180° 이동 피벗 턴 (2026-06-02)

**상태**: ✅ 착지 분기 PIE 검증 통과 / 🔶 피벗 턴 ABP 배선 완료, PIE 검증 대기 / ⏸ 착지 발 위상(Sync Marker) 폴리싱 보류

## 작업 요약

두 동작 추가: (A) 전진하며 점프 → **착지 순간 속도로 착지 모션 분기**(정지/달리기/질주), (B) **이동 입력 방향 기준 180° 피벗 턴**(전진 중 S, A→D 등). 둘 다 Spear 소스 실재 확인 후 killdong 리타겟 → ABP 배선. C++은 피벗 턴 신호 1개만 추가(나머지 0줄).

---

## A. 전진 점프 + 속도별 착지 (사람: 리타겟 + ABP / C++ 0줄)

### A-1. 리타겟 (IK Retargeter, killdong)
| 소스(_Seq) | → killdong | 용도 |
|---|---|---|
| `AS_Jump_Start_F_0` | `_Kildong` | 전진 점프 도약 |
| `AS_Jump_Loop_0` | `_Kildong` | 공중 낙하 루프(F 전용 없음 → 0 재사용) |
| `AS_Jump_End_F_0` | `_Kildong` | 전진 착지(정지) |
| `AS_Jump_to_Run_F_0` | `_Kildong` | 착지→달리기 (중속) |
| `AS_Jump_to_Run_Fast_F_0` | `_Kildong` | 착지→질주 (고속) |
| `AS_Jump_Start_0` | `_Kildong` | 제자리 점프 도약(저속/정지) |

### A-2. ABP Main States SM — 착지 분기 ("한 입구 → 조건별 3출구")
착지 속도 판정 = **착지 순간 GroundSpeed**. UE는 공중에서 수평속도 유지 → 점프 시점 속도와 사실상 동일 → **C++ 추가 0줄**(GroundSpeed/bIsInAir 이미 노출).

- `Fall Loop → To Land`(입구 하나): `bIsInAir == false`
- `To Land →` 3갈래 (**⚠️ 우선순위 숫자 작을수록 먼저 평가** — 600을 먼저 안 잡으면 150에 묻힘):

| 우선순위 | 조건 | 상태(클립) | 의미 |
|---|---|---|---|
| 0 | `GroundSpeed >= 600` | `Land_RunFast` (`AS_Jump_to_Run_Fast_F_0_Kildong`) | 질주 착지 |
| 1 | `GroundSpeed >= 150` | `Land_Run` (`AS_Jump_to_Run_F_0_Seq_killdong`) | 달리기 착지 |
| 2 | (조건 없음) | `Land` (`AS_Jump_End`) | 제자리 착지 회복 |

- 임계값 150/600 = 속도 사다리(150/500/700/800) 정합. 전환 블렌드 경과시간 0.2 + Hermite-Cubic InOut.

### A-3. 착지 상태 → Locomotion 탈출 (각 상태 전환 2개 = OR)
1. **AutomaticRule**(클립 끝나면) — 계속 달리는 정상 케이스. ⚠️ **클립 Loop OFF 필수**(루프면 안 끝나 미발동)
2. **`GroundSpeed < 100`**(블렌드 0.15~0.2s) — 착지 후 키 떼서 정지하면 클립 안 기다리고 즉시 탈출

> 둘 다 있어야 함. AutomaticRule만 → 멈췄을 때 클립 끝까지 "제자리 달리기". `GroundSpeed<100`만 → 달리는 중 영원히 못 빠져나옴.

### A-4. 재점프 공중 탈출 (PIE 버그 → 별칭으로 해결)
연속 점프 시 착지 상태에 갇혀 to_Run 클립을 **공중에서 재생**하는 버그. 착지 3상태(Land/Land_Run/Land_RunFast)를 **"To Falling" 별칭**에 포함 → `Is In Air == true AND VelocityZ > 100` 규칙으로 Jump 복귀. 원칙: "bIsInAir==true면 어느 상태든 무조건 공중 분기로"(착지 상태는 지상 전용이라 공중 탈출구 없으면 갇힘).

### A-5. 컴파일러 경고 처리
`Transition X is using an automatic transition rule but the source ... is set to looping` = 단발 클립(Jump_Start/Land/to_Run 계열)이 **Loop=켜짐**인데 자동전환을 건 것. 해당 시퀀스 플레이어 노드 Details → **Loop Animation 체크 해제**(Loop 유지할 건 Fall Loop/Locomotion뿐).

---

## B. 180° 이동 피벗 턴 (C++ 신호 1개 = 승환 / ABP·회전 = 사람)

### ⚠️ 설계 정정 (PIE 중 발견)
초안은 **카메라(컨트롤 회전) 기반 제자리 턴**(`TurnYawOffset`)이었으나 의도와 불일치. S 누르면 카메라 정면으로 도로 돌려버리는 버그 발생. **승환 의도 = 이동 입력 방향 전환 피벗**: 전진 중 S(반대 입력), A→D 등 이동 입력이 메시 facing과 ~180° 반대일 때 턴 애니 발동 후 그 방향으로 이동. → **신호를 카메라 → 이동입력(가속도)로 교체.** `TurnYawOffset`은 미사용(잔존 OK).

### B-1. KDPlayerAnimInstance C++ — MovementInputAngle 노출 (승환 작성·빌드 완료)
`.h` (public 1 + private 캐시 1):
```cpp
// 피벗 턴용: 메시 facing -> 이동 입력(가속도) 방향의 부호있는 Yaw 차(deg). +=우, ±180=반대.
UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
float MovementInputAngle = 0.f;
// private:
FVector CachedAcceleration = FVector::ZeroVector;
```
`.cpp`:
- `NativeUpdateAnimation`(게임스레드, MovementComp 줄 옆): `CachedAcceleration = MovementComp ? MovementComp->GetCurrentAcceleration() : FVector::ZeroVector;`
- `NativeThreadSafeUpdateAnimation`(워커):
  ```cpp
  const FVector AccelDir = CachedAcceleration.GetSafeNormal2D();
  MovementInputAngle = AccelDir.IsNearlyZero()
      ? 0.f
      : FMath::FindDeltaAngleDegrees(CachedActorRotation.Yaw, AccelDir.Rotation().Yaw);
  ```
> 부호 규약: `FindDeltaAngleDegrees(A,B)` = A→B 부호있는 차 [-180,180]. >0=우(R), <0=좌(L). 가만히 서면 가속도 0 → 각도 0 → 자동 비발동(이동 중에만).

### B-2. 리타겟 — ⚠️ 루트 회전 보존
`AS_Turn_180_L/R` → `_Kildong`. **루트모션 살림(루트 yaw 회전 보존), 강제잠금 OFF**. 스코프: Turn 180만(Turn_90 스킵 → 좌/우 부호로만 분기). "Show Root Motion"으로 root yaw ~180° 확인.

### B-3. ABP 배선 — 루트모션 턴
- ABP **클래스 디폴트** → **`Root Motion Mode = Root Motion from Everything`**(로코모션 in-place 클립은 루트모션 0이라 무영향)
- Locomotion SM에 `Turn_180_R`/`Turn_180_L` 상태 2개(각 `AS_Turn_180_R/L_Kildong`, **Loop OFF**)
- 진입(Peaceful → Turn): `MovementInputAngle >= 135` → `Turn_180_R` / `<= -135` → `Turn_180_L` (카메라 기반 `TurnYawOffset` 전환은 삭제)
- 복귀(Turn → Peaceful) 2개: **자동 규칙**(클립 끝) + `GroundSpeed > 10`(턴 중 이동 시 즉시 탈출)

### B-4. ⚠️ 회전 권한 충돌 (PIE 관문, 미검증)
free 모드 orient-to-movement(540°/s) + 턴 루트모션이 같은 방향 → 과회전/싸움 가능. 1차는 신호만 고쳐 PIE 확인. 과회전 시 → 턴 클립에 **AnimNotifyState로 재생 중 `bOrientRotationToMovement=false` 토글**(begin OFF / end ON)이 정공법.

---

## KDPlayerAnimInstance 노출 변수 최종 (스레드 분리)
| 변수 | 산출 | 스레드 |
|---|---|---|
| GroundSpeed | `CachedVelocity.Size2D()` | 워커 |
| VelocityX / VelocityY | `UnrotateVector` 로컬 분해 (Y=좌우, X=전후) | 워커 |
| TurnYawOffset | facing↔컨트롤 Yaw 차 (**현재 미사용**) | 워커 |
| MovementInputAngle | facing↔가속도 Yaw 차 (피벗 턴) | 워커 |
| bIsLockedOn / bIsInBattleStance / bIsInAir / bIsSprinting / bIsFullSprinting | 태그·getter·IsFalling | 게임 |
| BattleStanceHoldSec (EditAnywhere, 3.0) | 튜닝값 | — |

- **게임스레드**(`NativeUpdateAnimation`): UObject 접근 전부 — Velocity/ActorRotation/Acceleration 스냅샷, IsFalling, sprint getter, ASC 태그, LastCombatTime 갱신
- **워커스레드**(`NativeThreadSafeUpdateAnimation`): 캐시된 멤버로 순수 float 연산만

---

## 핵심 함정 (이번 세션 발견)

### 함정 1 — 우선순위 역순 평가
착지 3출구에서 우선순위 숫자가 **작을수록 먼저** 평가. 600(질주)을 먼저 안 잡으면 150(달리기)에 다 묻힘.

### 함정 2 — AutomaticRule엔 Loop OFF 필수
자동전환(클립 끝나면)은 루프 클립에서 영영 미발동. 단발 클립은 시퀀스 플레이어 노드에서 Loop 해제.

### 함정 3 — AND로 묶지 말 것
`GroundSpeed<100 AND (Get Relevant Anim Time Remaining Fraction < 0.2)`로 짜면 둘 다 참 요구 → 달리는 중 갇힘. 자동규칙 + GroundSpeed<100은 **OR(전환 2개 분리)**여야 함.

### 함정 4 — Get Relevant Anim Time Remaining Fraction은 노드
Details 옵션이 아니라 규칙 그래프 **노드**. 입력 배선 불필요(Update Context 비움). AutomaticRule 체크와 충돌 → 둘 중 하나만.

### 함정 5 — 피벗 신호는 카메라가 아니라 이동입력
orient-to-movement 게임에서 카메라(컨트롤 회전) 기반 턴은 입력과 싸움. 가속도(이동 입력) 방향 기준이 정답.

---

## PIE 검증 결과
- [x] 전진 점프 착지 → 멈춤 없이 to_Run 전진 지속 ("잘 된다잉")
- [x] 착지 후 정지 → GroundSpeed<100으로 즉시 Locomotion 복귀
- [x] 연속 점프 → To Falling 별칭으로 공중 복귀(착지 상태 안 갇힘)
- [ ] 180 피벗 턴 — ABP 배선 완료, PIE 검증 대기
- [ ] 회전 권한 충돌(과회전) 여부 — 미검증

## 미해결 / 후속
- **착지→로코모션 발 위상(발 스냅)** — `Land_Run` → Run 루프 전환 순간 다리 덜 올라간 채 블렌드 시작 → 발 내려찍힘. 블렌드 문제 아닌 **위상 불일치**. 해결 = `Land_Run`/`Land_RunFast`/Run 루프에 발접지 **Sync Marker** + 동일 **Sync Group**. (180 턴 먼저 하느라 보류)
- **피벗 과회전** — PIE 확인 후 필요시 AnimNotifyState로 orient 토글
- 락온 중 피벗 비발동 확인(스트레이프 유지)
- 도약 분기(`Locomotion→Jump` GroundSpeed>150 → Jump_Start_F_0 / else Jump_Start_0) — 착지 검증 후 부착

## 산출물
- `Source/Project_KD/Player/KDPlayerAnimInstance.h/.cpp` (MovementInputAngle + CachedAcceleration 추가)
- `Content/.../Animation/ABP_Player` (Main States 착지 분기 + Locomotion Turn_180_R/L)
- killdong 리타겟: 전진 점프 6종 + Turn_180_L/R
