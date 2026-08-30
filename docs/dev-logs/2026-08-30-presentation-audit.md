# 2026-08-30 — 3단계 연출 이식 전 구조 감사

## 진행

**코드 구현 없이 조사만.** 3단계 연출 이식(시간 제어 · 카메라 쉐이크 · 이펙트) 착수 전에 우리 현황을 파악하고 중복을 피하기 위한 사전 감사.

계획 전문 = `docs/handoffs/CURRENT.md` §2026-08-30 연출

## 1. 왜 조사했나

3단계 연출을 얹으려면 이미 있는 것이 뭔지 알아야 한다. 그냥 만들면 이식이 아니라 **중복 추가**가 된다.

### ⚠️ 내가 틀렸던 것 2건

```
① CustomTimeDilation 1.5 = 플레이어만 빨라지는 구간? → 틀렸다
   SB의 Art/Show 파티클 키 프로퍼티다. 월드 슬로모 상쇄를 위한 연출 보정일 뿐
   게임플레이 시간이 아님 → 3단계 목록에서 빠진다
   
② 일반 타격 카메라 쉐이크 3중? → 실제는 1중
   BP_GCN_HitImpact_Light 에 카메라 쉐이크 노드 없음 (승환 에디터 확인)
   원인 = 바이너리 스캔은 "있다" 판정 근거로 쓰면 안 된다
         에셋 지워도 문자열 테이블에 이름이 남음
   현재 실제 = GCN_PlayerHitConfirm → LCS_Hit 하나뿐
```

## 2. 우리 연출 지도 — MCP 실측

### 퍼펙트 패링

```
발신 = GA_PerfectParryReaction

GameplayCue  GameplayCue.Combat.PerfectParry.SlowMo
GCN 클래스   GCN_PerfectParry (Actor)
  Global TD  0.3 → Timeline 0.15초 → Set 1.0
  MPC         MPC_ScreenEffects  Tint / Desaturation 0.7 / Intensity (곡선)
  Sound       Metal_Hit_Flesh_7_Cue (bIsUISound=true)
  이펙트      NS_Slash_PowerUp_Burst_01 무기 소켓 Spear_Tip (bAutoDestroy=false)
  카메라      LCS_PerfectParry  scale 1.0 CameraLocal
```

### 퍼펙트 회피

```
발신 = KDGameplayAbility_Dodge.cpp:71

GameplayCue  GameplayCue.Combat.PerfectDodge
GCN 클래스   GCN_PerfectDodge (Actor)
  Global TD  0.7 → Timeline 1.5초 → Set 1.0
  MPC         MPC_ScreenEffects  (퍼펙트 패링과 동일)
  Sound       음성 파일명
  이펙트      NS_PerfectDodge (RootComponent)
  카메라      없음
```

### 반격 발동

```
발신 = KDGameplayAbility_CounterSlash::OnActivated

GameplayCue  GameplayCue.Combat.CounterThrust
GCN 클래스   GCN_CounterThrust
  기능        KDPlayerCameraManager::TriggerFovPunch(1.0) 뿐
```

### 반격/일반 명중

```
발신 = KDGameplayAbility_PlayerMelee::OnTargetHit

GameplayCue  GameplayCue.Combat.PlayerHitConfirm
GCN 클래스   GCN_PlayerHitConfirm (Static)
  읽음 출처   DA_HitLightAttack 에서 프로필 가져옴
  카메라      LCS_Hit
  이펙트      NS_Hit_Basic_Once
  사운드      SC_Sword_Hit
  히트스톱    0.08초
  
★ 반격과 일반 타격이 완전히 같은 프로필을 쓴다
```

### 적 방어형 패링

```
발신 = KDGameplayAbility_EnemyParry.cpp:78

GameplayCue  GameplayCue.Combat.ParryClash
GCN 클래스   BP_GCN_ParryClash (Burst)
  이펙트      NS_Hit_Distortion_Once
  사운드      SC_Hit_Cue
  카메라      BurstCameraShake = None (빈 칸)
```

### 가드 붕괴

```
발신 = (C++ 신호)

GC 태그    (없음 — GA_GuardBreak 몽타주만)
효과       AM_SB_Block_Hit_Break 1.0초 재생 (02-30 패치 적용됨)
```

## 3. 카메라 쉐이크 3종 비교

| 이름 | 용도 | 진동 축 | FOV | 비고 |
|------|------|--------|-----|------|
| **LCS_Hit** | 일반 타격 | Loc Y amp 2.0 freq 40 | — | 가장 담백 |
| **LCS_PerfectParry** | 퍼펙트 패링 | Rot 2.0 / Loc 1.0 | ✓ FOV 3.0 | 유일하게 FOV 진동 |
| **LCS_PlayerHitConfirm** | (고아) | Rot 3축 + Loc 2축 freq 40 | — | 미참조 (DA_HitLightAttack이 LCS_Hit 써서) |

⚠️ `LCS_PlayerHitConfirm` 은 아무 데서도 호출되지 않는 고아 에셋. 지우지 말고 **반격 전용으로 재활용** 권장.

## 4. ★ 발견 — 카메라 랙 동적 제어가 이미 구현돼 있다

```
GCN_CounterTrail    태그 GameplayCue.Camera.DashTrail
  동작              KDSpringArmComponent 에 bEnableCameraLag 토글
    true → CameraLagSpeed 3.0 → Delay 0.5s
    → CameraLagSpeed 20.0 → Delay 0.25s → false

★ 트레일 VFX는 없다. 이름과 달리 카메라 랙만 한다

발신 미발견      C++ 0건 / 몽타주 119개(스캔) 0건
                (추론: 서드파티 몽타주 미스캔 또는 발동 안 됨)
```

### 코드 검증 — 랙이 실제로 작동하는 구조

```cpp
// KDSpringArmComponent.cpp:25-26
ApplyRailPosition();      // SocketOffset · TargetArmLength만 정한다
Super::TickComponent();   // 엔진이 그 값으로 랙·충돌·소켓 갱신
```

✓ 위치를 직접 덮어쓰지 않음 → 엔진 랙이 정상 작동  
✓ 부모 `USpringArmComponent` 가 `CameraLagSpeed` 를 BlueprintReadWrite 로 노출  
✓ 우리 코드에 그 이름이 없는 건 손댄 적이 없어서 (못 만져서 아님)

## 5. 🔴 실제 버그 — Global Time Dilation 이중 소유

```
GCN_PerfectParry   Set Global TD 0.3 → 0.15초 → Set 1.0
GCN_PerfectDodge   Set Global TD 0.7 → 1.5초  → Set 1.0

둘 다 저장 없이 1.0 하드 복구
GameplayCueNotify_Actor 라 큐마다 액터가 따로 스폰돼 서로를 모른다
```

### 재현

```
t=0.00  퍼펙트 닷지 → TD 0.7 / 1.5초
t=0.20  그 중에 퍼펙트 패링 → TD 0.3 / 0.15초
t=0.35  패링 끝 → Set 1.0
        ⚠️ 닷지가 1.15초 남았는데 시간이 정상으로 돌아온다
```

### Content 전수 스캔

`Set Global Time Dilation` 사용처 = 우리 에셋 2개뿐  
(`OrionGlobalGameplayCollection` 은 Paragon 샘플 MPC 무관)

### C++ 쪽 — 안전함

```cpp
// KDHitStopComponent.cpp:34-47
float SavedTimeDilation = 1.f;  // 저장
Owner->CustomTimeDilation = 0.f;  // 정지

// 라인 36-42: 이미 돌고 있으면 남은 시간이 더 길면 무시 / 짧으면 연장만
// = "더 긴 쪽이 이긴다"
```

✓ `CustomTimeDilation` (액터 단위) 사용  
✓ `SavedTimeDilation` 에 저장했다가 복원  
✓ 중첩 정책 이미 있음

## 6. ★★ SB는 어떻게 하나 — 덤프 전수 조사

덤프 루트 = `D:/stellarblade_Fakuri/Exports/SB/Content/`

### 시간 제어 필드 (Content 152,371 JSON 전수)

```
SBShowTimeScaleKey = 시간 관련 유일한 Show 키 타입

필드 (572 키)
  Duration 572 / TimeScale 564 / StartTime 542
  BlendInTime 503 / BlendOutTime 484 / Priority 24
  bEnable 43 (False만) / CheckNoneShowKeyTag 11 / CheckShowKeyTag 7

없는 것들
  Layer · Channel · Slot · Group · bOverride · bExclusive
  EndTime · LifeTime · TargetScale · GlobalScale · bPlayerCharacterOnly

★ "복귀 목표값" 필드가 없다. BlendOutTime으로 빠져나올 뿐
```

### Priority 분포 (시간 배율 우선순위)

```
Priority 1000   사망 연출 17건      (Art/Show/Dead/* 등)
Priority 10     짧고 강한 히트스톱  7건
                P_Eve_Tachy_Skill_ParrySlash1(2)
                Fusion_ParrySlash1(2)
                FlashBehindAttack_S · ChainStabCombo1_Ex(2)
Priority 0      나머지 전부 (미기입)
```

### ★★ Priority 값이 "긴 쪽 승"도 "느린 쪽 승"도 부정한다

```
Priority 10 인 7개 = Duration 0.04~0.3초 짧음
  → "긴 쪽 승" 이면 항상 져야 하는데 일부러 올려놨다

Priority 1000 인 사망 연출 중 TimeScale 0.5도 있음
  그게 0.045 인 ActionAssist를 눌러야 한다
  → "느린 쪽 승" 이면 못 누른다

=> 중요도는 숫자로 유추 불가. 별도 필드 필수
   SB가 그래서 Priority 를 뒀다
```

### ★★★ SB 도 전역 슬롯이 하나다 (실측)

```
Art/Character/PC/CH_P_EVE_01/Blueprints/
  CH_P_EVE_01_AnimBP_{New,Snapshot,Studio}.json
    
    Function isSlomode
      CallFunc_GetGlobalTimeDilation_ReturnValue  ← 전역 배율을 읽는다
      CallFunc_Less_FloatFloat_ReturnValue         ← 임계값 비교
      → ReturnValue(bool) + Time(float)를 매 프레임 소비
```

📌 애님BP가 슬로모 여부를 **스택 조회가 아니라 전역 스칼라 하나**로 판단  
📌 칸이 하나뿐이니 겹치는 키들을 먼저 정산해서 써넣어야 한다  
📌 이게 Priority 필드가 존재하는 **구조적 이유**

### 게임플레이 레이어 — 별도 슬롯

```
EffectTable (7건)  EffectAction_TimeScale  전역
                   {TotalTime, FadeInTime, FadeOutTime, TimeScale}
                   
(6건)               EffectAction_AreaTimeScale  범위
                   {TargetFilterAlias, TimeScaleValue(-80=20% 속도)}

중첩 정책
  EffectOverlap_Overlap  누적 (2952행)
  EffectOverlap_Change   교체 (780)
  EffectOverlap_Unique   단독 (495)
```

### 겹칠 수 있는 조합 실측 (일부)

```
저스트 패링      TimeScale 0.25 / Duration 0.15 / Priority 0
저스트 패링 여운  0.3         / 0.5           / 0
퍼펙트 회피      0.2         / 0.2           / 0
블링크 시작      0.1         / 0.07          / ★10
패리슬래시       0.1→0.25    / 0.04→0.3      / ★10
히트스톱         0.15        / 0.025         / 0
액션어시스트     0.045       / 0.1           / 0, bApplyImmediately=true
사망 연출        0.2~0.5     / 0.17~0.5      / ★1000
```

📌 **SB 히트스톱은 시간 배율만이 아니다**  
— 같은 ubergraph 에 `IsEnableHitStopIK` / `GetHitStopIKData` 호출이 따로 있다  
— 시간 배율 + IK 보정을 병행. 우리 범위 밖이지만 타격감 폴리싱 때 보면 도움

## 7. 결정 — 우리 설계 방향

### 신설 Subsystem

```cpp
UKDSlowMotionSubsystem : UWorldSubsystem
  RequestSlowMo(Scale, Duration, Priority = 0)
    스택에 엔트리 등록 → 최고 Priority 의 Scale 적용
    만료 시 그 항목만 제거 후 재계산  ← 1.0 하드코딩 없음

우선순위 (SB 실측 그대로)
  1000  처형 · 사망         Priority 최고
  10    퍼펙트 패링         중간
  0     퍼펙트 닷지 (기본)   낮음
  
동률이면 나중 것이 이긴다  ← SB 덤프에도 없다. 우리가 정한다
```

### 패턴

```
GCN_PerfectParry / GCN_PerfectDodge 는
  Set Global Time Dilation 대신 이 Subsystem만 호출

Subsystem이 Singleton 아님 이유
  Global TD 는 월드에 하나뿐인 값
  GCN 은 큐마다 액터가 스폰돼 서로를 모른다
  CLAUDE.md 싱글톤 금지에 "UE5 Subsystem 제외" 명시
```

## 8. 이번에 처리한 것

```
✓ GA_PerfectParryReaction의 Print String 제거
✓ GCN_HitReact 쉐이크 제거
✓ GCN_PerfectParry의 bAutoDestroy false → true (02-30 진행 중)
✓ Spear_Tip 소켓 유효성 확인 (아래 §9 참조)
```

## 9. ★ Spear_Tip 소켓 — 결함 아니었다

```
GCN_PerfectParry가 GetComponentsByTag(SkeletalMeshComponent,"Weapon")[0]
의 소켓 Spear_Tip에 이펙트를 붙인다

DA_Weapon_Bong 확인
  WeaponStaticMesh = None
  WeaponMesh = SKM_Bong
  
SKM_Bong_Skeleton의 소켓 = GripPoint · Spear_Tip  둘 다 존재

KDWeaponComponent.cpp:176  SkeletalMeshComponent 경로
KDWeaponComponent.cpp:184  WeaponMesh→ComponentTags.Add("Weapon")
```

✓ 이름은 폐기된 길동 창이지만 현역 무기다

### ⚠️ 새로 걸린 것 — 태그 중복

```
WeaponComponent 와 GunWeaponComponent가 둘 다 태그 "Weapon" 부여
GetComponentsByTag[0] 이 순서 의존이 된다

현재 안전  — 총 데이터(CurrentWeapon·WeaponMeshAsset) 비어 있음
위험 신호  — 총 무기를 채우는 순간 패링 이펙트가 총구에서 날 수 있다

선례      — KDAnimNotifyState_WeaponTrail 은 이미 Sword/Gun으로 구분
```

## 이후 구현 — 조사한 날 바로 고쳤다

### ① `UKDSlowMotionSubsystem` 신설

```
Source/Project_KD/Combat/KDSlowMotionSubsystem.h / .cpp   신규

UKDSlowMotionSubsystem : public UWorldSubsystem
  ├ FKDSlowMoRequest (구조체)
  │   ├ Scale : float = 1.f          (시간 배율)
  │   ├ Priority : int32 = 0         (우선순위)
  │   └ ExpireTime : float = 0.f     (만료 시각, 실시간 초)
  │
  ├ RequestSlowMo(Scale, Duration, Priority = 0)   BlueprintCallable
  │   → 요청을 목록에 등록, 우선순위 갱신, 타이머 설정
  │
  ├ Recalculate()   private
  │   → 만료 항목 제거 → Priority 최고인 Scale 적용 → 다음 만료에 타이머
  │
  ├ Deinitialize()   override
  │   → 레벨 전환 시 TimerManager 정리 + GlobalTimeDilation 1.0 복구
  │
  └ Requests : TArray<FKDSlowMoRequest>
```

★ **"복귀" 개념을 없앤 것이 핵심**

```
종전   Set 0.3 → 0.15초 → Set 1.0 (하드코딩)
      결과: 아래 있던 0.7 배율을 밀어버린다

현행   요청만 등록 후 목록에서 빼기
      결과: 아래 것이 자동으로 드러난다
```

### ② 설계 판단 3개

**①-1 만료 = `GetRealTimeSeconds()` 기준**

```
게임 시간이면 슬로모 중에 연출 타이밍이 늘어난다
0.15초는 항상 0.15초여야 한다

⚠️ FTimerManager 는 게임 시간 기준이라 알람이 늦게 울린다
   하지만 Recalculate 에서 Now 와 다시 비교하므로 판정은 정확하다
   정리(RemoveAll)만 한 프레임 늦다 → PIE에서 눈에 띄면 그때 손본다
```

**①-2 `CancelSlowMo()` 함수를 넣지 않음**

```
쓸 상황을 찾지 못했다
"처형 중 다른 슬로모 취소" = Priority 1000 이 우선순위로 대신한다
필요하면 코드 5줄 추가면 된다
```

**①-3 동률이면 나중 등록된 것이 승**

```
for 루프의 비교가 R.Priority >= Winner->Priority 라서
배열 뒤쪽(나중 등록)이 이긴다

SB 덤프에도 동률 규칙이 없다
우리가 정한 설계 (우선순위 같으면 최신이 우선)
```

### ③ BP 배선

| GCN | 변경 | 상세 |
|-----|------|------|
| **GCN_PerfectParry** | Set Global TD 2개 제거 | `Get World Subsystem → Request Slow Mo(0.3 / 0.15 / P10)` 추가 |
| **GCN_PerfectDodge** | Set Global TD 2개 제거 | `Get World Subsystem → Request Slow Mo(0.7 / 1.5 / P0)` 추가 |

⚠️ **Timeline 은 유지** — MPC Intensity 곡선을 위해 (슬로모와 별개 축)

⚠️ **닷지 쪽 주의** — 종전 Timeline 복귀(1.0)가 Subsystem 만료(실시간 1.5초)보다 늦게 울려 다른 슬로모를 밀어버릴 수 있었다. 지금은 둘 다 없어 정상.

### ④ 곁가지로 고친 것

```
GA_Parry.ActivationBlockedTags 에 State.Combat.Dodging 추가
  회피 중 패링이 그냥 발동했다
  GA_Dodge 쪽도 BlockAbilitiesWithTag 비어 있었다
  → 차단 완성: Dead · InAir · Aiming · GuardBroken · Dodging

GA_PerfectParryReaction   Print String 디버그 제거 (BP 콘솔용)
GCN_CounterThrust        Print String 제거

GCN_HitReact             중복 카메라 쉐이크 1개 제거
GCN_PerfectParry         나이아가라 bAutoDestroy false → true (누수 방지)
```

### ⑤ UFUNCTION DisplayName 관련

현재 코드:
```cpp
UFUNCTION(BlueprintCallable, Category = "SlowMotion", 
  meta =(ToolTip = "0 = 기본 | 10 = 짧고 강한 히트스톱 | 1000 = 처형·사망"))
void RequestSlowMo(float Scale, float Duration, int32 Priority = 0);
```

📌 **DisplayName 은 포함하지 않음** (한글은 노드 검색·자동완성에서 걸리고 나중에 찾기 어렵다)  
📌 **ToolTip 은 한글 유지** (설명은 읽기용)  
📌 ⚠️ 이미 컴파일된 BP는 옛 캐시를 유지한다. 재컴파일 필요.

### ⑥ 검증

```
빌드        통과 (C++ 컴파일 성공)
PIE 로컬    ✓ 퍼펙트 패링 단독 → 0.3 배율 0.15초 정상
            ✓ 퍼펙트 닷지 단독 → 0.7 배율 1.5초 정상
            ✓ 닷지 슬로모 중 패링 → 패링 0.3 덮음 → 끝나면 닷지 0.7 살아남 ★
            ✓ 회피 중 패링 차단 작동
```

### ⑦ 남은 것 갱신

**✅ 해결** (2026-08-30)
```
Global TD 이중 소유           Subsystem 신설로 완전 제거
회피 중 패링                 ActivationBlockedTags 추가
Print String 디버그          2곳 제거
중복 카메라 쉐이크           GCN_HitReact 1개 제거
나이아가라 누수              bAutoDestroy false → true
회피 Timeline 복귀 깨짐      Timeline 통째로 없음 (Subsystem 대체)
```

**남음**
```
GCN_CounterTrail 배선        태그 GameplayCue.Camera.DashTrail 발신자 만들기
                            (발동 미확인 — 몽타주 스캔 0)

GA_CounterSlash             HitConfirmProfile = None 채우기
  + 반격 전용 DA_HitProfile  (일반과 분리, LCS_PlayerHitConfirm 재활용 권장)

BP_GCN_ParryClash           BurstCameraShake 빈 칸 채우기
가드 붕괴 전용 GC           (현재 GA_GuardBreak 몽타주만 있음)
일반 막기 임팩트 GC         (없음 — Block_End 재생 안 되는 결함과 관련)

MPC 원복 노드               Desaturation · Tint 커브 끝값 미확인
이펙트 소켓 중복 위험        WeaponComponent · GunWeaponComponent 둘 다 "Weapon" 태그
                          총 무기 완성 시 구분 필수
```

---

**커밋:** `[Combat] Subsystem 기반 Global Time Dilation 스택 — 우선순위 체계로 전환`

## 남은 것

(위 ⑦에 통합됨)

---

**커밋:** 코드 변경 (Subsystem 신설 + BP 배선 + 곁가지 수정)
