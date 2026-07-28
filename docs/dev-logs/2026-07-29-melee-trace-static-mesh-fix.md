# 2026-07-29 근접 판정 부활 — 무기가 StaticMesh라 코드가 배제하고 있었다

지상 콤보의 근접 판정이 **아예 작동하지 않던 상태**를 고치고, 몽타주 20개에 노티를 일괄 배치했다.

전제 문서 = [2026-07-28 타격 시퀀스 실측](2026-07-28-gunsword-hit-sequence.md). 그 문서의 §0단계(선행 작업)와 §1단계(배치표)를 실행한 기록이다.

## 원인은 2겹이었다 — 데이터만 고쳤으면 못 고쳤다

전 세션 핸드오프는 원인을 **데이터**로 지목했다. 공격 GA 5개가 길동 시절 창 소켓(`Spear_Bottom`/`Spear_Tip`)을 물고 있고 `MeshSource`가 `OwnerBody`라는 것. 맞는 지적이지만 **절반이었다.**

```cpp
// GA_MeleeTraceBase.cpp (수정 전)
Avatar->GetComponents(USkeletalMeshComponent::StaticClass(), Components);
```

**검과 총은 StaticMesh다.**

```
DA_Weapon_Sword.WeaponStaticMesh = .../Meshes/Sword   (StaticMesh)
DA_Weapon_Gun.WeaponStaticMesh   = .../Meshes/Gun     (StaticMesh)
WeaponMesh(스켈레탈) = None
```

`WeaponComponent.cpp:134`가 `UStaticMeshComponent`로 만드는데 GA는 스켈레탈만 훑으니, **데이터를 아무리 고쳐도 무기를 못 찾는다.** `WeaponComponent` 자신은 2026-07-17 무기중립화 때 이미 `UMeshComponent`로 넓혀져 있었고, **판정 경로 2파일만 안 따라온** 상태였다.

### 고친 것

`USkeletalMeshComponent` → `UMeshComponent`(스태틱·스켈레탈 공통 부모)로 타입만 확장. 소켓 조회(`GetSocketLocation`/`DoesSocketExist`)는 `USceneComponent` 가상함수라 **로직은 한 줄도 안 바뀌었다.**

| 파일 | 변경 |
|---|---|
| `AT_MeleeTrace.h/.cpp` | 팩토리 파라미터 + 멤버 타입 |
| `GA_MeleeTraceBase.cpp` | `TraceMesh` 타입 / 컴포넌트 검색 / 캐스팅 |
| `GA_MeleeTraceBase.h` | 기본값 `Spear_*`/`Weapon` → `Sword_*`/`Sword` |

기본값을 고친 건 **새 GA를 만들 때 또 창 소켓이 딸려오는 걸 막기 위해서**다. 부수 효과로 BP 5개의 소켓·태그 3칸이 자동 정정됐다 — **BP 오버라이드가 아니라 C++ 기본값을 보고 있었기 때문.** 실제 BP 오버라이드는 `MeshSource` 하나뿐이었다.

## ★ 이 버그가 컴파일러를 통과한 이유

지시한 3줄 중 **캐스팅 한 줄이 누락됐는데 빌드가 통과했다.**

```cpp
UMeshComponent* TraceMesh = nullptr;               // 타입은 넓힘
TraceMesh = Cast<USkeletalMeshComponent>(Comp);    // Cast는 안 고침
```

`USkeletalMeshComponent*` → `UMeshComponent*`는 **업캐스트라 합법**이다. 검(StaticMesh)을 넣으면 런타임에 조용히 `nullptr`이 될 뿐. 에러도 경고도 없다.

증상이 구코드와 **100% 동일**해서 진단이 크게 돌아갔다 — 소켓·태그·BP 값·DLL 타임스탬프·에디터 프로세스 시작시각까지 전부 확인한 뒤에야 코드에서 찾았다.

### 결정적 단서는 런타임 로그 두 줄이었다

```
[KD-DBG]  WeaponMesh_Sword / hasTag=1     ← 찾긴 찾았다
[KD] Trace mesh not found                 ← 그런데 결과가 없다
```

**"조회 성공"과 "결과 없음"이 동시에 뜨면 그 사이의 변환을 의심한다.** 이 구간에 있는 건 캐스팅 한 줄뿐이었다.

### 재발 방지 2가지

1. **타입 확장 리팩토링은 옛 타입 이름을 전량 grep한다.** 컴파일러가 안 잡아주는 영역
2. **검증을 `git diff`로만 하지 않는다.** diff는 바뀐 줄만 보여준다 — 누락된 줄은 hunk 컨텍스트 밖이라 화면에 아예 안 뜬다. 실제로 이번에 diff 리뷰를 통과시켰다

`GA_MeleeTraceBase.cpp:142` 뒤에 **소켓 존재 가드**를 추가해, 소켓명이 어긋나면 조용히 죽는 대신 로그가 뜨게 했다.

## 🔴 같은 패턴이 한 군데 더 남아 있다

```
GCN_EnemyTelegraph.cpp:40  GetComponents(USkeletalMeshComponent::StaticClass(), ...)
GCN_EnemyTelegraph.cpp:45  Cast<USkeletalMeshComponent>(Comp)
```

적 무기 텔레그래프. 현재 적 무기가 스켈레탈이면 무해하지만, **적에게 스태틱 무기를 쥐여주는 순간 오늘과 똑같이 조용히 죽는다.** 승환 판단으로 보류.

## Sword 소켓

`GripPoint` 소켓이 없어 역보정이 안 걸리므로 **메시 원점이 곧 손 위치**다.

| 소켓 | 위치 | 근거 |
|---|---|---|
| `Sword_Bottom` | `(0, 0, 0)` | 원점 = 손 |
| `Sword_Tip` | `(0, -129, 0)` | 바운드 Y −129.1~+23.8, 긴 쪽이 날 |

X/Z를 0으로 둔 건 **Sweep 캡슐 반경이 3cm라 그 정도 오차를 덮기 때문**이다. 정밀도를 올려도 체감 차이가 없다.

> `AttachSocketName = weapon_r` 폴백은 죽은 값이다(SKM에 그 소켓 없음). `CurrentWeapon`이 지정돼 있어 DA의 `Sword_Weapon_RSocket`이 쓰이므로 현재는 무해.

## ANS_WeaponTrail — 무기 소켓 사용 가능해짐

`SpawnSystemAttached(NS, MeshComp, ...)`가 **몽타주 재생 중인 캐릭터 메시로 고정**돼 있어 무기에 트레일을 달 수 없었다. `WeaponMeshComponentTag`를 추가해 GA와 같은 방식으로 무기 메시를 찾아 붙인다. 태그를 비우면 기존대로 본체 메시(맨손/발차기용).

`SocketName` 기본값도 `weapon_trail`(프로젝트에 없는 죽은 값) → `Sword_Bottom`으로 교체. 코드가 NS에 `SwordLength`를 쏘는 구조라 손잡이에 붙이고 길이로 뻗는 방식이 맞다.

### ⚠️ AnimNotify는 액터별 인스턴싱이 없다

`SpawnedComponent`가 노티 **객체의 멤버**인데, 노티 객체는 몽타주 에셋 소속이고 **재생 액터마다 복사되지 않는다**(객체 경로 `AM_....AM_...:ANS_WeaponTrail_0`의 Outer가 몽타주다). 노티 함수가 `MeshComp`를 인자로 받는 것 자체가 공용이라는 신호다.

같은 몽타주를 동시에 재생하는 캐릭터가 2명 이상이면:
- 남의 트레일을 끄고 (`Deactivate`가 나중 포인터를 잡음)
- 자기 트레일은 아무도 안 꺼서 영구 잔류

**재생자가 1명이면 발생하지 않는다.** 적에게 트레일을 붙일 때 `TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<UNiagaraComponent>>`로 분리할 것. 지금 넣으면 안 쓰는 복잡도다.

## 노티 일괄 배치 — 지상 20개

전 문서의 배치표를 스크립트로 전량 반영하고 **되돌려 읽어 기계 대조(20/20 일치)** 했다.

트랙 6줄: `MeleeTrace` / `CancelWindow` / `MovementCancel` / `WeaponAttach` / `Sound_Swing` / `Sound_Hit`

| 트랙 | 개수 | 비고 |
|---|---|---|
| `MeleeTrace` | 25창 | 예외 4개 개별 확인 |
| `CancelWindow` | 15 | 4타 5개는 없음 |
| `MovementCancel` | 20 | 전 타 |
| `WeaponAttach` | 5 | 각 루트 1타 f0 |
| `Sound_Swing` / `Sound_Hit` | 22 / 25 | 간격 2f 이하 3개는 Hit만 |

**예외 4개** — `03_01`①=`foot_r`, `04_02`②·`04_03`②=`foot_l`(`MeshSourceOverride=OwnerBody`), `05_01`=`TraceModeOverride=TipLine`.

**사운드 트랙을 2줄로 쪼갠 이유**: 엔진 `Play Sound` 노티는 사운드가 비면 타임라인에 전부 `PlaySound`로 동일 표시되고, 지정하면 에셋명으로 자동 변경된다(`AnimNotify_PlaySound.cpp:75`). 비어 있는 동안 스윙/타격을 눈으로 가르는 유일한 방법이 트랙 분리다.

## 미결

### 4타 `MovementCancel` 값이 신뢰할 수 없다

측정 규칙(`notify_windows.py:81` RECOVER)은 **전신 속도가 그 클립 피크의 15% 아래로 떨어져 다시 안 올라오는 프레임**이다. 이게 뽑는 건 "몸이 멎는 시점"이지 "이동을 허용할 시점"이 아니고, 두 방향으로 깨진다.

- 느리게 끝나는 동작 → 임계를 일찍 밑돌아 **너무 이름**
- 끝에 잔동작(발 고쳐딛기 등)이 있으면 → 거기까지 밀려 **너무 늦음**
- 임계 분모가 클립마다 다른 피크값이라 **클립끼리 비교 불가**

| 4타 | 길이 | RECOVER | 클립 대비 |
|---|---|---|---|
| `01_04` | 120 | 27 | **23%** |
| `02_04` | 119 | 44 | 37% |
| `03_04` | 120 | 62 | 52% |
| `04_04` | 93 | 73 | **78%** |
| `05_04` | 145 | 84 | 58% |

23~78%. 비교로 **1~3타의 실측 캔슬은 35~57%에 몰려 있다**(중앙값 46%, 포즈 대조로 뽑은 진짜 값). 피니셔는 후딜이 더 길어야 리스크가 성립하므로 55~65%대가 출발점이지만, **결국 "막타 치고 얼마나 빨리 도망칠 수 있게 할 거냐"는 손맛 판단**이라 PIE에서 정한다. → **승환이 직접 조정 예정**

### 그 외

- **`AM_SB_Combo_01_01`만 PIE 검증됐다.** 나머지 19개 미검증
- **발차기 3개**는 `foot_r`/`foot_l`을 Start·End 둘 다 같은 본으로 넣어 캡슐이 반경 3cm 구(球)에 가깝다. 발등만 판정돼 작을 수 있음 — `ball_r`을 끝점으로 쓰면 발 전체가 되지만 배치표에 없던 값이라 넣지 않았다
- **`04_02`의 ①②창이 f19에서 맞닿는다** — 검`[11-19]` 끝 프레임에 발`[19-24]` 시작
- `ANS_WeaponTrail`은 **컴파일 미확인**(에디터 실행 중이라 빌드 못 돌림)
- `DA_ComboTree`는 **아직 한 번도 PIE로 안 돌려봤다.** 현재 `GA_LightCombo.DefaultAttackMontages=[AM_SB_Combo_01_01]` 1개짜리 임시 배선으로 동작 중
- 나머지 GA 4개 몽타주가 전부 stale — `HeavyCombo`(버터 발차기), `AirLightAttack`(창 공중), `SprintAttack`/`CounterThrust`(`AM_Run_Attack_*_Spear`)

## 커밋

| repo | 해시 | 내용 |
|---|---|---|
| 코드 | `1d25ccc` | 세션 시작 프로토콜 확장 |
| 코드 | `b3ad1bb` | 무기 판정 `UMeshComponent` 확장 |
| 코드 | `070d339` | `ANS_WeaponTrail` 무기 소켓 지원 |
| 코드 | `9515371` | Play Sound 배치표 |
| Content | `6611281` | `Sword` 소켓 2개 |
| Content | `85cadc7` | GA 5개 + `01_01` 노티 |
| Content | `d018f47` | 몽타주 20개 노티 일괄 |
