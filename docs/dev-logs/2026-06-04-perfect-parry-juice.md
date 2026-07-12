# 퍼펙트 패링 연출(juice) — 공부 노트 (2026-06-04)

> 승환 개인 학습용. P4 미공유. 쉬운 말로 정리.
> 이번에 만든 것: 퍼펙트 패링 성공 순간 **슬로우 + 사운드 + 무기 FX + 카메라셰이크 + 화면 청록 톤다운** 5겹 동시 발동. 전부 BP/에셋, C++ 0줄.

---

## 0. 전체 흐름 (한눈에)

```
AS_Combat (C++, 이미 완성)
  └─ 퍼펙트 성공 → Event.Combat.PerfectParryTriggered 발행 (플레이어 ASC에)
        │
        ▼
BP_GA_PerfectParryReaction (이벤트로 자동 발동되는 BP GA = "다리")
  · AbilityTriggers = {그 이벤트, Gameplay Event}
  · ExecuteGameplayCueOnOwner(GameplayCue.Combat.PerfectParry.SlowMo) → 즉시 EndAbility
        │
        ▼
GC_PerfectParry (GameplayCueNotify_Actor = 실제 연출 담당)
  OnExecute → RunSlowMo 커스텀이벤트 → 5겹 동시:
    (1) 슬로우(GlobalTimeDilation 0.3)  (2) 사운드  (3) 무기 Niagara FX
    (4) 카메라셰이크(CS_PerfectParry)   (5) 화면 포스트프로세스(청록 톤다운)
```

**왜 이렇게 단계가 많나?**
- 큐(GameplayCue)는 "이벤트로 자동 발동"이 안 됨 → 이벤트→큐를 잇는 **다리(BP GA)**가 필요.
- 로직(데미지0/카운터)은 C++, 연출(번쩍/소리)은 BP로 **분리**(디커플링). 연출을 통째로 떼도 게임플레이는 멀쩡.

---

## 1. 타임라인 (Timeline)

- **Event Graph 전용** — 함수 그래프 안엔 못 넣음. 그래서 **커스텀 이벤트(`RunSlowMo`)**를 만들어 우회했음.
- **트랙 + Length 있어야** 재생되고 **Finished**가 발동함. 트랙 없으면 끝나도 Finished 안 옴.
- **Float 트랙**은 출력 핀이 하나 생김 → 매 프레임 그 시점 값(0→1→0)을 뱉음 → 이걸로 MPC Intensity를 굴렸음.
- ⭐ **`Play` vs `Play from Start`** (이번 "첫 패링만 되는" 버그의 핵심):
  - `Play` = **멈춘 자리에서 이어 재생.** 한 번 끝까지 가면 바늘이 끝에 박혀 있음 → 다시 `Play` 눌러도 끝에서 안 움직임 → 두 번째 패링부터 화면효과 안 뜸.
  - `Play from Start` = **항상 0으로 되감고 재생.** 반복되는 효과는 무조건 이걸 써야 함. → 해결됨.

## 2. 시간 지연 (Time Dilation)

- `Set Global Time Dilation(0.3)` = 온 세상이 0.3배속 슬로우.
- **함정(닭-달걀)**: 슬로우 복귀를 일반 `Delay`나 타이머로 하면, 그 Delay 자체도 슬로우에 같이 느려져서 복귀가 한참 늦게 옴.
- **해결** = 타임라인을 **`Ignore Time Dilation` 켜기**(타임라인 노드 더블클릭 → 툴바 체크박스). 그러면 타임라인만 **실제 시간**으로 돌아 슬로우와 무관하게 정확히 복귀.
- 그래서 Finished에서 `Set Global Time Dilation(1.0)`으로 정상 복귀시킴.

## 3. 카메라 셰이크 — 베이스 클래스 3종

| 베이스 | 성격 |
|---|---|
| **LegacyCameraShake** (구식) | 진동 필드가 디테일에 **내장** → 원샷 흔들기에 제일 쉬움. FOV 진동도 포함. **이번에 이걸로 `CS_PerfectParry` 만듦** |
| CameraShakeBase (신식) | 비어 있음 → **Shake Pattern**을 추가해야 작동. 유연하지만 손 더 감 |
| DefaultCameraShakeBase | 기본 래퍼 |

- 호출: 큐에서 `Get Player Controller(0) → Client Start Camera Shake(Shake Class = CS_PerfectParry, Scale 1.0)`.

## 4. 진동 파라미터

- 흔들 수 있는 축: **회전(Pitch/Yaw/Roll) · 위치(X/Y/Z) · FOV**.
- 각 축마다 **진폭(Amplitude=크기)** + **주파수(Frequency=속도)**.
  - ⭐ **둘 다 0보다 커야** 그 축이 흔들림. 하나라도 0이면 안 흔들림.
- 느낌 조합:
  - **고주파 + 저진폭** = 날카로운 버즈(파르르) → **패링**에 어울림.
  - **저주파 + 고진폭** = 묵직하게 출렁 → 폭발/거대 타격.
  - **FOV 진동** = 화면이 확 줌됐다 풀리는 "FOV 펀치" 느낌.

## 5. 머티리얼 색 타입 — float3 vs float4 (이번 두 번의 에러 원인)

- **float3 = RGB**(색 3개 묶음), **float4 = RGBA**(색+투명도 4개 묶음).
- 연산(Multiply, Lerp)은 **묶음 크기가 같아야** 함. 다르면 컴파일 에러 `arithmetic between types ... are undefined`.
- ⭐ **함정**: `SceneTexture`의 Color 출력은 **float4**임. 그래서 float3짜리(Desaturation 결과 등)와 곱하면 에러.
- **해결** = **`Mask(RGB)`**(=ComponentMask, R·G·B만 체크, A 끄기) 노드를 끼워 float4 → float3로 맞춤.
  - 이번에 두 군데 끼웠음: ① Tint(float4) → Mask → Multiply / ② SceneTexture Color → Mask → Lerp.A.
- (추가 실수) Desaturation을 SceneTexture의 **Size 핀**(화면 해상도 숫자)에서 끌어옴 → **Color 핀**에서 끌어와야 맞음.

## 6. MPC + 포스트프로세스 머티리얼 로직 (핵심 = 재사용·디커플링)

- **MPC (Material Parameter Collection) = 공용 숫자 조종판/다이얼.**
  - `MPC_ScreenEffects`에 `Intensity`(강도) / `Desaturation`(채도 뺌) / `Tint`(덧입힐 색) 다이얼이 있음.
  - **밖(큐/BP)에서 실시간으로 값을 돌릴 수 있음** → 머티리얼은 그 값만 쳐다봄.
- **포스트프로세스 머티리얼(`M_PP_ScreenEffect`) = 그 다이얼 값 보고 화면을 칠하는 붓.**
  - Material Domain = **Post Process**. 현재 화면을 `SceneTexture: PostProcessInput0`로 읽음(관례. SceneColor도 되지만 톤매핑 전 단계라 약간 다름).
  - 로직: 원본 화면 ──┐
                    Lerp(A=원본, B=가공[채도↓+청록 곱], **Alpha = Intensity**) → 이미시브 컬러
    - **Intensity 0 = 원본 그대로(평소엔 안 보임)** / **1 = 효과 풀.**
  - 머티리얼은 **항상 카메라에 등록**돼 있지만 Intensity 0이라 평소엔 투명.
- **큐 = 그 다이얼을 돌리는 손.** 타임라인이 매 프레임 Intensity를 0→1→0으로 굴림 → 화면이 번쩍 청록 됐다 풀림.
- ⭐ **왜 이 구조?**: 다음에 퍼펙트 닷지/저체력/도술 화면효과 만들 때 **같은 MPC+머티리얼 재사용**, 큐만 새로 다이얼 돌리면 됨. C++ 컴포넌트 안 만들어도 됨(YAGNI).

## 7. "첫 번만 되는" 버그 = 풀링 + Play 끝멈춤

- **GCN_Actor는 재활용(풀링)됨** — 큐 액터를 매번 새로 만들지 않고 돌려씀.
- 거기에 타임라인 `Play`(끝에 바늘 박힘)가 겹쳐서 → 두 번째 패링에 화면효과 안 뜸.
- 해결 = **`Play from Start`**(되감기). (풀링도 `Destroy Actor(self)`가 풀을 깨뜨릴 수 있어 의심했지만, 1차 원인은 Play였음.)

## 8. (개념만, 나중 구현) 프리즈 프레임

- 아주 짧게 시간을 **0 근처로 완전히 멈췄다** 푸는 연출(타격 임팩트 강조). 스파 슈퍼무브 직전 같은 "딱!" 느낌.
- 슬로우(0.3)보다 더 강한 정지. 나중에 추가 예정 (승환 "좋네").

---

## 실제 만든 에셋/BP 정리

- `MPC_ScreenEffects` (Material Parameter Collection): Intensity 0 / Desaturation 0.7 / Tint 청록(R0 G1 B1)
- `M_PP_ScreenEffect` (Post Process 머티리얼): PostProcessInput0 → (Mask→Lerp.A) / (Desaturation→×Tint→Lerp.B), Alpha=Intensity → 이미시브
- `BP_PlayerCharacter` FollowCamera → Post Process Materials 배열에 `M_PP_ScreenEffect` Weight 1 등록
- `GC_PerfectParry` (GCN_Actor, 태그 `GameplayCue.Combat.PerfectParry.SlowMo`): RunSlowMo → 5겹 + 타임라인(Length 0.15, Ignore Time Dilation ON, Play from Start, 키 0.0=0 / 0.05=1 / 0.15=0)
- `CS_PerfectParry` (LegacyCameraShake 자식)
- `BP_GA_PerfectParryReaction` (이벤트 트리거 GA, StartupAbilities 등록)
