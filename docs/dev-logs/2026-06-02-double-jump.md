# 더블 점프 (Double Jump) (2026-06-02)

**상태**: ✅ PIE 검증 통과. 공중 재점프 시 DoubleJump 클립 재생 + 착지 분기 정상. 2차 점프 높이 튜닝은 후속 보류.

## 작업 요약

공중에서 한 번 더 점프하는 더블 점프 추가. 입력 무변경(`IA_Jump` 그대로 — 두 번째 탭 = 두 번째 `ACharacter::Jump()`).
엔진 더블점프는 `JumpMaxCount=2` 한 줄로 켜지고, 진짜 작업은 **2차 점프 때 더블점프 클립을 트는 애니 신호 하나뿐**.
`bIsInAir`만으론 1차/2차 점프 구분 불가 → **rising-edge 신호** 노출이 핵심.

**스코프(2회 축소)**: 방향별(F/B/L/R) 스킵 → 인플레이스 `_0` 하나만. 전투/일반 분기도 폐기 → **DoubleJump 상태 1개로 통일**(Peaceful·Combat 더블점프 애니가 거의 동일 → 구분 무의미). 리타겟된 `AS_Double_Jump_Combat_0_Kildong`은 잔존(미사용).

## 3개 부품

| # | 부품 | 역할 | 작업 |
|---|---|---|---|
| ① | 엔진 더블점프 | 공중 재점프 1회 허용 | C++ `JumpMaxCount=2` |
| ② | 트리거 신호 | "2차 점프 막 발동한 1틱"만 감지 | C++ `bDoubleJumpTriggered` (rising-edge) |
| ③ | ABP 상태 | 신호 받아 더블점프 클립 재생 | `DoubleJump` 상태 |

## ① 엔진 더블점프 켜기 (승환 작성, 빌드 통과)
`PlayerCharacter.cpp` 생성자(이동 설정 줄 옆):
```cpp
JumpMaxCount = 2; // ACharacter 멤버. 공중 재점프 1회 허용(단일→더블)
```
> `ACharacter::Jump()`이 `JumpCurrentCount < JumpMaxCount` 체크 → 두 번째 입력에서 공중 재점프. `JumpCurrentCount`는 착지 시 0 리셋. `JumpZVelocity`/`AirControl`은 기본값 유지(1·2차 공용).

## ② C++ `bDoubleJumpTriggered` — rising-edge ★ (승환 작성, 빌드 통과)
`KDPlayerAnimInstance.h` public 1개 + private 캐시 2개:
```cpp
// 더블점프 트리거: 2차 점프가 막 발동한 1틱만 true. ABP DoubleJump 진입용.
UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
bool bDoubleJumpTriggered = false;
// private:
int32 CachedJumpCount = 0;
int32 PrevJumpCount = 0;
```
`.cpp`:
- `NativeUpdateAnimation`(게임스레드, OwningPlayer 접근부):
  ```cpp
  CachedJumpCount = OwningPlayer ? OwningPlayer->JumpCurrentCount : 0;
  ```
- `NativeThreadSafeUpdateAnimation`(워커):
  ```cpp
  bDoubleJumpTriggered = (CachedJumpCount >= 2) && (CachedJumpCount > PrevJumpCount);
  PrevJumpCount = CachedJumpCount;
  ```
> **왜 rising-edge**: `JumpCurrentCount`는 착지 전까지 2로 유지됨 → `>=2`만 쓰면 ABP 전환이 매 틱 참 → 재진입 무한루프. `PrevJumpCount`(전 프레임 카운트 보관)와 비교해 **상승 에지(1→2 되는 1틱만 true)**로 잡아야 DoubleJump 상태가 1회만 재생. 1차 점프(0→1)는 `>=2` 불충족이라 비발동.

## ③ ABP Main States SM — `DoubleJump` 상태 1개
- 상태 1개: `DoubleJump`(`AS_Double_Jump_0_Kildong`), **Loop OFF**(자동 규칙 발동 위해 필수)
- **진입 2개** — 어느 공중 상태에서든 잡히게:
  - `Jump ──(bDoubleJumpTriggered == true)──▶ DoubleJump`
  - `Fall Loop ──(bDoubleJumpTriggered == true)──▶ DoubleJump`
  - (bool 직결 — 비교 노드 불필요. 블렌드 ~0.1s)
- **탈출 2개**:
  - `DoubleJump ──(자동 규칙: 클립 끝)──▶ Fall Loop` (계속 낙하)
  - `DoubleJump ──(bIsInAir == false, 우선순위 0)──▶ Fall Loop` (플립 중 착지, 블렌드 0.05~0.1s)
> ⚠️ 탈출은 컨듀잇 `To Land`로 직접 못 그어서 **`Fall Loop`로 보냄** — Fall Loop에 이미 `Fall Loop→To Land`(bIsInAir==false)가 있어 한 틱 뒤 착지 분기(Land/Land_Run/Land_RunFast)로 자동 연결. 컨듀잇 무수정, 결과 동일.

## 동작 흐름
```
점프(1차, JumpCurrentCount 0→1) → Jump/Fall Loop
  공중에서 재입력(2차, 1→2)
    bDoubleJumpTriggered = true (1틱)  → DoubleJump 진입
    클립 1회 재생 (rising-edge라 재진입 X)
  클립 끝 → Fall Loop / 도중 착지 → Fall Loop → (한 틱) To Land → Land 분기
착지 시 JumpCurrentCount 0 리셋 → 다음 공중에서 재무장
```

## 점프 높이 관찰 (후속 보류)
2차 점프 높이가 들쭉날쭉 느껴지는 건 **정상 물리**. `JumpZVelocity`는 그 순간 수직속도를 **덮어쓰는(리셋)** 값이라, 2차를 **언제 누르냐**에 따라 도달 높이가 달라짐(상승 중 누르면 낮고, 정점 근처면 높음). 애니 셋팅(루트모션 OFF + Force Root Lock ON) 문제 아님 — 높이는 100% 물리(CharacterMovement)가 결정, 클립은 시각 연출.
- 일정하게/다르게 하려면 → `BP_PlayerCharacter` CharacterMovement `JumpZVelocity` 조정(1·2차 공용), 또는 2차만 다르게 하려면 `JumpCurrentCount` 분기로 속도 교체(입력 도메인 코드). **YAGNI로 보류.**

## PIE 검증 결과
- [x] 점프 → 공중 재점프 → 2차 도약 + `DoubleJump` 클립 재생
- [x] 더블점프 클립 1회만 재생, 상태에 안 갇힘 (rising-edge 확인)
- [x] 더블점프 → 낙하/착지 → 기존 Land 분기(Land/Land_Run/Land_RunFast) 정상
- [x] 3단 점프 안 됨 (`JumpMaxCount=2` 상한)
- [x] 락온/전투 회귀 없음
- [ ] 2차 점프 높이 튜닝 — 후속

## 산출물
- `Source/Project_KD/Player/PlayerCharacter.cpp` (생성자 `JumpMaxCount=2`)
- `Source/Project_KD/Player/KDPlayerAnimInstance.h/.cpp` (`bDoubleJumpTriggered` rising-edge + `CachedJumpCount`/`PrevJumpCount`)
- `Content/.../Animation/ABP_Player` (Main States SM `DoubleJump` 상태 + 전환 4개)
- killdong 리타겟: `AS_Double_Jump_0_Kildong` (+ 미사용 `AS_Double_Jump_Combat_0_Kildong`)
