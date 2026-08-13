# 2026-08-13 — 일반 공격 자동 조준 + 총격 사운드

락온을 걸어야만 되던 자동 조준을 **평상시에도** 되게 했다. 총격 판정 원점을 총구에서 캡슐로 옮기고, 총격 몽타주 21발에 샷건 사운드를 붙였다.

커밋 = 소스 `dc0f7c7` `1f62da2`(push 완료) / Content `595c78b` `d8976c9`(로컬)

---

## 1. 자동 조준 — 락온 게이트 제거

`GA_PlayerMeleeAttackBase::OnActivated`이 이 한 줄로 막혀 있었다.

```cpp
if (!ASC || !ASC->HasMatchingGameplayTag(GameplayTags::State_Character_LockOn)) return;
```

락온 태그가 없으면 그냥 나간다 = 락온을 안 걸면 자동 조준이 아예 안 돈다. 적 옆에서 휘둘렀는데 허공을 베는 원인.

재료는 이미 다 있었다. `ULockOnComponent::FindBestTarget()`이 public이고 Sphere Overlap → `IKDTargetable` 자격 → 시야 콘 → LoS를 전부 통과시킨다. **새 클래스도 새 검색 코드도 만들지 않았다.**

### `UGA_ActionBase::FindAutoAimTarget(Range, ConeAngle)` 신설

```cpp
ULockOnComponent* LockOn = GetLockOnComponentFromActorInfo();
if (!LockOn) return nullptr;
return LockOn->IsLockedOn() ? LockOn->GetLockedTarget() : LockOn->FindBestTarget(Range, ConeAngle);
```

근접 GA와 `GA_ShotBlast` 두 곳이 같은 판단을 쓴다. `GA_ShotBlast`는 `UGA_ActionBase` 직속이라 근접 쪽 코드를 못 물려받는다 → 부모로 올렸다(§3 "2회 이상이면 함수 분리"). 접근자를 `UGA_ActionBase` 한 곳에 모으는 §1-3 규칙과도 맞는다.

`GA_PlayerAirAttackBase.cpp:55`가 `Super::OnActivated()`를 부르므로 **공중 콤보에도 자동으로 붙었다.**

### `FindBestTarget` — 인자 2개 + 정렬 교체

```cpp
AActor* FindBestTarget(float OverrideRadius = -1.f, float OverrideConeAngle = -1.f) const;
```

음수면 Config 값(1000 / 90)을 쓴다. 락온은 인자 없이 부르므로 종전 그대로.

고르는 규칙을 **최단거리 → 콘 중심선 각도 최소**로 바꿨다.

```cpp
// 최단거리 갱신을 내적 최대로
float BestDot = -1.f;   // 각도 최소 = 내적 최대 (같은 방향이 +1)
if (Dot > BestDot) { BestDot = Dot; BestTarget = Candidate; }
```

⚠️ **락온 토글도 같은 함수를 쓴다**(`ToggleLockOn` / 타겟 자동 전환). 락온 키가 "가장 가까운 적"에서 **"화면 한가운데 적"**으로 바뀌었다. 의도한 변경 — SB 락온 필터도 `SortType = SmallAngle`이다.

### 값 근거 = SB 덤프 실측

| 항목 | 채택값 | 근거 |
|---|---|---|
| 고르는 규칙 | 각도 최소 | Eve 지상 검 콤보 17개(Light 8 + Strong 9)가 전부 `SortType = SmallAngle` |
| 부채꼴 | ±90° (`AutoAimConeAngle 180`) | `P_Eve_Sword_0500_180_500_Target` |
| 사거리 | 500cm | 같은 필터 `FarDistance` |
| 방향 기준 | 카메라 forward (현행 유지) | **SB는 기준 벡터가 표에 없다 = C++.** 추측을 넣지 않기로 |

우리 콘이 SB의 절반(±45°)이었던 게 "왜 안 돌지"의 실제 원인이었다.

## 2. 총격 판정 원점을 캡슐 중심으로

`GA_ShotBlast::GatherTargets`가 후보 수집·각도·시야 검사를 **전부 총구 기준**으로 했다. 총구는 캐릭터보다 앞으로 나가 있어서, **몸에 붙은 적이 총구에서 보면 뒤쪽 = 각도 90° 초과로 탈락**했다.

```cpp
const FVector ConeOrigin = Avatar->GetActorLocation();   // 부채꼴 꼭짓점 = 캡슐 중심
GatherTargets(ConeOrigin, ShotDir, HalfAngle, Hits);
```

방향(`ShotDir`)은 안 건드렸다 — 총구 X축 또는 자동 조준 대상 그대로. **바뀐 건 부채꼴이 시작하는 자리 하나뿐.** 인자 이름도 `MuzzleLoc` → `Origin`으로 바꿨다.

곁가지 이득 — 시야 확인 라인트레이스 시작점도 같이 몸으로 왔다. 총구가 벽을 뚫고 나가 있으면 벽 너머 적이 맞던 경로가 막힌다.

## 3. 디버그 — 90° 이상은 구로

`DrawDebugCone`은 각도 180°에서 꼭짓점 16개가 뒤쪽 한 점으로 뭉쳐 **바늘로 보인다**(`LineBatchComponent.cpp:515~546`). `AM_SB_Combo_05_03`이 정확히 그 경우라 판정 범위를 눈으로 볼 수 없었다.

```cpp
if (HalfAngle >= 90.f)  DrawDebugSphere(World, Origin, ShotRange, 24, ...);
else                    DrawDebugCone(World, Origin, ShotDir, ShotRange, ConeRad, ConeRad, 16, ...);
```

판정과 무관한 그리기 문제였다. 이제 180°는 노란 구로 나온다.

## 4. 총격 사운드 21발

`Sound_Shot` 트랙 신설(주황). `AN_ShotBlast`와 **정확히 같은 시각**에 `AnimNotify_PlaySound` 배치 → 판정과 소리가 같은 프레임.

```
ShotGun_Shot_Sound (SoundWave 직결) / Volume 0.5 / Pitch 1.0
13개 몽타주 21발 = 01_01~04 · 02_02~04 · 03_02 · 03_03 · 04_03 · 04_04 · 05_03(5) · 05_04
```

⚠️ **땜빵이다.** SoundWave 직결이라 매번 같은 소리가 난다 — 05_03의 5연사에서 티가 난다. 감쇠도 없다(2D 재생). 나중에 `SC_Shotgun_Shot` 큐로 감싸 Modulator로 피치를 흩뜨리고 볼륨·감쇠를 한 곳에서 잡을 것. **노티의 `Sound` 칸만 바꾸면 되므로 재배치는 불필요.**

---

## 검증 (PIE 9항목 전수 통과)

```
1  락온 X, 적 옆에서 공격        몸이 적 쪽으로 돈다
2  적 둘(정면 / 측면)            정면 쪽을 친다              ← 각도순 확인
3  5m 밖 적                      안 돈다                     ← AutoAimRange 500
4  등 뒤 적                      안 돈다                     ← 135도 예외
5  락온 X 로 콤보 총격           콘이 적 쪽으로 돈다         ← 신규
6  몸에 붙은 적                  맞는다                      ← 캡슐 원점 효과
7  05_03 윈드밀                  5발이 사방으로 돈다         ← bUseMuzzleDirection 회귀
8  락온 걸고 공격                종전과 같다
9  락온 토글                     화면 중앙 적을 잡는다(변경됨, 정상)
```

총격 사운드 재생 확인. 볼륨 1.0이 커서 0.5로 조정.

## 실측으로 뒤집힌 것 3건

문서에 적혀 있던 것이 셋 다 틀렸다. 전부 실측으로 확인해 핸드오프를 고쳤다.

1. **"발사체 리팩토링 3건 착수 가능"** → **2건은 08-10에 이미 닫혀 있었다.** 델리게이트 바인딩은 `KDProjectile.cpp:53~54`에서 `BeginPlay`로 옮겨졌고, `GA_Dodge.cpp:198`에 `&& Proj->GetInstigator() != Avatar`가 있다. 남은 ③(faction 게이트 비대칭)은 소환수·동료가 생겨야 터진다
2. **"05_03 첫 Shot 노티만 각도 10, 나머지 179"** → **5개 전부 `muzzleDir=True` / `halfAngle=180` / `ignoreHitStop=True`로 통일돼 있었다.** 총격 노티 전수(13몽타주 21발) 조회 결과 **05_03만 특수하고 나머지 12개는 전부 기본값**
3. **"발소리가 전부 없다"** → **`/Game/MotionMatchingAnimation/Audio`에 286개가 이미 있다.** 걷기30·달리기29·착지20·점프17 등 + `MSS_FoleySound_*` 동작별 래퍼 13개. **없는 건 재료가 아니라 배선**

## SB 스킬 테이블 조사

자동 조준 값을 정하려다 테이블 구조를 파게 됐다. 전문 = 볼트 `notes/Reference/StellarBlade_스킬테이블_3층구조.md`(신규).

- **3층 구조** — `SkillCommandTable`(연결) / `SkillTable`(정의, 108칸) / `SkillActiveStepTable`(실행, 6958행). **골격은 우리 `FComboLink` / `FComboNode` / 몽타주+노티와 같다**
- **`_Target`(누구를 향하나)과 `_HitArea`(누가 맞나)가 별도 필터.** 타격 판정은 `ActiveCollision` 1552 / `TargetFilter` 437 — **SB도 대부분 무기 콜리전으로 때린다**(우리 `ANS_MeleeTrace`와 같은 구조)
- **`RotateInputDirection`** — 1414개 중 true 177, 그 중 **175가 Eve**. "입력 방향 회전"의 유일한 근거이나 **동작은 C++이라 미확인**
- **노드마다 사거리 칸을 파는 건 근거 없다** — SB조차 17개가 같은 필터를 공유한다. 반면 `AttackDamageRate`는 전부 다르다 → **`DamageMultiplier`가 다음에 팔 칸**

## 사고 기록 — 파이썬으로 만든 SoundCue가 에디터를 죽였다

검 스윙 큐를 MCP 파이썬으로 짜다 `Assertion failed: Array index out of bounds: 0 into an array of size 0`로 에디터가 죽었다.

`USoundNodeMixer`는 자식마다 볼륨을 **병렬 배열** `InputVolume`에 들고 있는데(`SoundNodeMixer.cpp:24`), 그 배열을 채우는 함수 셋(`CreateStartingConnectors` / `InsertChildNode` / `#if WITH_EDITOR SetChildNodes`)이 **전부 에디터 전용**이다. `new_object` + `set_editor_property("child_nodes", ...)`는 셋 다 안 탄다 → 자식 2개 / `InputVolume` 0개.

**저장도 로드도 성공하고 재생 순간에만 죽는다.** 큐는 에디터에서 짜는 게 맞다. 상세 = 볼트 트러블슈팅 `MCP-파이썬으로만든_SoundCue_Mixer가_재생시크래시`.

곁가지 — `sound_cue_graph`가 파이썬에 미노출이라 **에디터 그래프 창에는 옛 배선이 그대로 그려진다.** "노드가 똑같은데?"로 보이는 이유.

## MCP 실측으로 밝힌 것

- **몽타주 노티는 파이썬으로 읽힌다.** `notifies`가 protected인 건 맞지만 `unreal.AnimationLibrary.get_animation_notify_events(montage)`가 우회한다. 볼트의 "우회도 실패" 기록은 폐기
- **bool UPROPERTY는 파이썬에서 접두 `b`가 떨어진다** — `bUseMuzzleDirection` → `use_muzzle_direction`. `b_`를 붙이면 "property not found"
- **프로젝트 사운드 전수** — SoundWave 593 / SoundCue 268. 우리가 만든 건 `Audio/Combat` 큐 3개가 전부였다. "빈 철봉" 소리의 정체 = `Metal_Light_Whoosh`가 **공기 가르는 소리만 든 팩**

## 남은 것

1. **총성 조달** — Sonniss GDC 2026 번들(347개 전수)에 **총기 라이브러리가 0개**. 답은 **The Free Firearm Sound Library**(CC0, 194MB, OpenGameArt). 샷건 수록이라 `GA_ShotBlast`와 맞는다
2. **`SC_Shotgun_Shot` 큐** — 지금 SoundWave 직결이라 5연사가 같은 소리. Modulator로 피치 흩뜨리기 + 감쇠
3. **검 스윙 소재 교체** — `_SoundPicks/01_Sword/METLFric_SWING SCRAPE ... Long Blade 14`가 `Metal_Light_Whoosh` 대체 1순위. 되면 지금 Mixer 구조가 통째로 불필요
4. **발소리 배선** — 재료 286개는 이미 있다
5. **`FComboNode.DamageMultiplier` + `InputWindow` 26칸** — 폴리싱으로 미룸(승환 결정). 설계 4단계는 핸드오프에
6. `AutoAimRange 500` / `ShotRange 500` 튜닝 — 원점이 몸으로 와서 앞쪽 도달이 줄었다. 체감으로 조정
