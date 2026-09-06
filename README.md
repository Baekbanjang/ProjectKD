# Project KD

**Unreal Engine 5.6 + GAS 기반 3인칭 액션 전투 시스템** — 1인 개발

스텔라 블레이드를 레퍼런스로, 검과 총을 함께 쓰는 근·원거리 전투를 구현했습니다.
콤보 분기부터 패링·처형·스킬까지 하나의 흐름으로 연결하는 것이 목표였습니다.

▶ **[시연 영상 (7분 52초)](https://www.youtube.com/watch?v=RzbQjqXZrcQ)** — 챕터로 구간이 나뉘어 있습니다

---

## 기술 스택

```
Unreal Engine 5.6  ·  C++  ·  Gameplay Ability System
Motion Warping  ·  Enhanced Input
```

C++ 소스 **204개 파일** — GameplayAbility 클래스 26종, AttributeSet 2종

---

## 주요 구현

### 콤보 시스템 — 노드 그래프

콤보를 **입력 시퀀스 배열이 아니라 노드 그래프**로 구성했습니다.

```cpp
FComboNode  { NodeId · Montage · TArray<FComboLink> NextLinks · 데미지/넉백/Poise 배수 }
FComboLink  { InputTag → NextNodeId }
FComboEntry { RequiredStateTag + InputTag → StartNodeId }
```

시퀀스를 기억하지 않고 **현재 노드에서 나가는 길만** 봅니다. 그래서 세 가지가 자연히 풀립니다.

- 콤보 도중 다른 계열로 갈아탄 뒤에도 파생이 이어짐
- 여러 진입 경로가 같은 공격 노드로 합류 (약공 시작 · 강공 시작 · 회피 후)
- 회피가 콤보를 끊지 않고 그래프 상의 위치 이동이 됨

`NextNodeId`가 배열 인덱스가 아닌 `FName`인 이유는 데이터 에셋을 에디터에서 직접 편집하기 때문입니다. 노드를 중간에 삽입해도 링크가 깨지지 않습니다.

📁 `AbilitySystem/Combo/`

### 타격 판정 — 프레임 보간

무기 소켓을 매 프레임 읽어 **이전 위치와 현재 위치 사이를 훑습니다.** 이동 거리에 비례해 그 사이를 나눠 보간하므로, 빠른 스윙이나 낮은 프레임에서 판정이 프레임 사이로 빠져나가지 않습니다.

```
판정 구간의 첫 프레임과 마지막 프레임을 살리고 그 사이를 채운다
→ 30FPS 에서도 60FPS 와 같은 밀도로 검사
```

판정 형태는 공격 특성에 따라 4가지 중에서 선택합니다.

| | |
|---|---|
| `Sweep` | 무기 축 캡슐을 이전→현재 위치로 스윕 |
| `TipLine` | 검 끝만 선으로 검사 |
| `ArcSweep` | 2차 베지어로 호를 그려 캡슐 스윕 |
| `ArcTri` | 호를 선 격자로 채워 두께 없이 판정 |

판정 구간은 코드가 아니라 **AnimNotifyState가 정의**합니다. 타이밍 조정에 재컴파일이 필요 없습니다.

📁 `AbilitySystem/Tasks/` · `AbilitySystem/AnimNotifies/`

### 접근 · 자동 조준

대상 탐색 조건을 구조체로 분리해 어빌리티마다 다르게 설정합니다 (부채꼴/원기둥 · 반경 · 반각 · 정렬 기준).

선택한 대상 쪽으로 캐릭터를 회전시킨 뒤, **모션 워핑으로 공격 애니메이션의 이동량을 실제 거리에 맞춰 늘리거나 줄입니다.** 사거리 밖이면 제자리에서, 이미 붙어 있으면 원래 이동량으로 재생합니다.

📁 `Combat/Data/KDTargetFilter.h` · `AbilitySystem/Abilities/Player/`

### 카메라 — 스플라인 돌리

시점을 각도 기반 계산이 아니라 **스플라인 위의 위치**로 다룹니다. 기본 시점과 사격 시점에 각각 스플라인을 두고 상태에 따라 전환합니다.

```cpp
CameraDollySpline    기본 시점
AimDollySpline       사격 시점
```

📁 `Player/KDSpringArmComponent` · `Player/KDPlayerCameraManager`

### 사격 — 조준점 보정

3인칭에서는 카메라가 어깨 뒤에 있고 총구는 손에 있어, 화면 가운데를 조준해도 총구가 향하는 방향이 다릅니다.

```
1  카메라 시점에서 화면 중앙으로 트레이스 → 조준점
2  총구에서 그 조준점을 향하도록 발사각 재계산
```

지형과 캐릭터를 나눠 두 번 조회합니다. 캐릭터 콜리전 프로파일이 `Visibility` 채널을 무시하기 때문에, 오브젝트 타입으로 직접 찾고 **두 번째 트레이스의 끝점을 첫 결과로 제한**해 벽 뒤의 적을 걸러냅니다.

📁 `AbilitySystem/Abilities/Player/KDGameplayAbility_Shoot.cpp`

### 데미지 파이프라인

모든 데미지가 **메타 어트리뷰트 한 곳으로 들어와** 분기합니다. 패링 가로채기 → 비율 경감 → 실드 흡수 → 체력 순으로 처리하고, 그 결과에 따라 히트리액션·가드 붕괴·처형 신호를 갈라 보냅니다.

📁 `AbilitySystem/Attributes/`

---

## 디렉터리

```
Source/Project_KD/
├─ AbilitySystem/      117    GAS 전반
│   ├─ Abilities/       54    어빌리티 (플레이어 · 적 · 공용 베이스)
│   ├─ AnimNotifies/    22    판정 구간 · 워프 · 이펙트 노티파이
│   ├─ Effects/         16    GameplayEffect
│   ├─ Attributes/       7    AttributeSet · 데미지 파이프라인
│   ├─ GameplayCues/     6    타격 연출 큐
│   └─ Combo/            4    콤보 트리 데이터 에셋
├─ Combat/             28    락온 · 넉백 · 경직 · 처형 · 타격 피드백
├─ Enemy/              27    적 캐릭터 · AI 컨트롤러 · 인카운터
├─ Player/             14    플레이어 캐릭터 · 카메라
├─ Character/ Movement/ Input/ UI/ Game/ Interface/ Library/
```

---

## ⚠️ 이 저장소에 대하여

**코드 열람용입니다.** 클론해도 그대로는 빌드되지 않습니다.

```
미포함    Content/ (에셋 · 블루프린트 · 애니메이션)
          Plugins/ · Config/ · .uproject
```

에셋은 용량 문제로 별도 관리하며, 플러그인은 외부 배포판이라 제외했습니다.
**구현 결과는 위 시연 영상에서 확인하실 수 있습니다.**
