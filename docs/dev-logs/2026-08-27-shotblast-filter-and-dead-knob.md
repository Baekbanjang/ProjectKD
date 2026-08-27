# 2026-08-27 — 총격 조준 필터 데이터화 + 죽은 노브 제거

08-27 오전의 `FKDTargetFilter` 데이터화가 **총격만 빠져 있던 것**을 마저 옮겼다.
곁들여 아무도 안 읽는 `UPROPERTY` 하나를 지웠다 — 그게 실제 값을 3일간 가리고 있었다.

## 1. 총격 조준 — 조립식에서 데이터로

`GA_ShotBlast` 는 발사할 때마다 필터를 **코드 안에서 조립**하고 있었다. 근접(`PlayerMelee`)은
이미 `AutoAimFilter` 멤버를 갖고 있었는데 총만 안 옮겨졌다.

```
종전   ShotFilter.Radius     = ShotRange           에디터 값
       ShotFilter.HalfAngle  = AimConeAngle * 0.5  에디터 값
       ShotFilter.Height     = 500.f               하드코딩
       ShotFilter.HeightOffset = -150.f            하드코딩
       ShotFilter.Basis      = Camera              하드코딩
       ShotFilter.SortType   = SmallestAngle       하드코딩

현행   UPROPERTY(EditDefaultsOnly) FKDTargetFilter AutoAimFilter;
       FKDTargetFilter ShotFilter = AutoAimFilter;      // 노티 오버라이드만 얹는다
```

높이·기준·정렬 4개가 코드에 박혀 있어 **"계단 위 적을 못 잡는다" 같은 걸 빌드 없이는 못 만졌다.**

### 기본값을 생성자에 둔 이유

`FKDTargetFilter` 구조체 기본값과 총격이 다른 건 **둘뿐**이다.

```cpp
// KDGameplayAbility_ShotBlast.cpp 생성자
AutoAimFilter.Basis    = EKDTargetBasisType::Camera;        // 구조체 기본 = CharacterForward
AutoAimFilter.SortType = EKDTargetSortType::SmallestAngle;  // 구조체 기본 = Nearest
```

나머지(`Arc` / `500` / `90` / `500` / `-150`)는 구조체 기본과 같아 안 쓴다.

★ **근접과 총격은 조준 철학이 반대다** — 근접은 `캐릭터 정면 · 최단거리`(코앞의 적),
총격은 `화면 기준 · 각도순`(조준선에 가까운 적). 이관하며 이 차이를 잃지 않는 게 핵심이었다.

## 2. `ShotRange` 는 지울 수 없었다 — 두 역할 겸업

이관 전 조사에서 걸린 것. 지웠으면 히트스캔이 통째로 죽었다.

```
:66   ShotFilter.Radius = ShotRange           조준 탐색 반경
:143  MakeSphere(ShotRange)                   실제 히트스캔 후보 수집 반경   <- 이것
:152  DrawDebugSphere(..., ShotRange, ...)
:157  DrawDebugCone(..., ShotRange, ...)
```

**칸 하나로 통일했다** — 세 곳 다 `AutoAimFilter.Radius` 로. 지금 두 값을 다르게 쓸 이유가 없고,
필터에 `Radius` 칸이 있는데 옆에 `ShotRange` 를 또 두면 어느 게 진짜인지 헷갈린다.

> 되돌릴 여지 = 나중에 "조준은 넓게 훑고 판정은 좁게" 가 필요해지면 그때 다시 가른다.
> 지금 가르면 **"조준선은 잡혔는데 안 맞는다"** 를 만들 수 있다.

`AutoAimConeAngle` 은 `:58` 한 곳뿐이라 필터의 `HalfAngle` 로 완전히 흡수됐다.
⚠️ **BP CDO 실값을 먼저 확인했다** — `ShotRange 500` · `AutoAimConeAngle 180` 둘 다 헤더 기본값
그대로라 지워도 잃을 값이 없었다. 값이 덮여 있었다면 구조체 필드로는 리다이렉트가 안 이어진다.

### 노티 오버라이드는 살렸다

```cpp
FKDTargetFilter ShotFilter = AutoAimFilter;
if (Notify && Notify->AutoAimConeAngleOverride > 0.f)
{
    ShotFilter.HalfAngle = FMath::Clamp(Notify->AutoAimConeAngleOverride * 0.5f, 5.f, 135.f);
}
```

`AM_SB_Combo_05_03` 5연발만 이 오버라이드를 쓴다(전방위 180). **이 변경의 유일한 위험지점**이라
PIE 확인 대상이었다.

## 3. 죽은 노브 — 값 하나가 3일간 가려져 있었다

`GA_Parry.h:50` 의 `PerfectParryWindowSec = 0.2f` 는 `EditAnywhere` 로 에디터에 떠 있는데
**소스 어디에서도 읽히지 않았다.** 선언 1회 + 주석 1회가 전부.

```
주석이 약속한 것   GA_Parry 가 Spec.Data->SetDuration(PerfectParryWindowSec) 로 길이 덮음
실제               SetDuration 호출처 = 그 주석 한 줄뿐. 코드가 없다
                   ApplySelfEffect 는 MakeOutgoingSpec -> ApplyToSelf 뿐
진짜 길이          GE_PerfectParry 에셋의 Duration
```

### 그래서 벌어진 일

```
08-24 기록   "촬영용 임시값 원복 - 퍼펙트 패링 창 0.5 -> 0.2"
실제          원복된 건 죽은 노브(GA_Parry.PerfectParryWindowSec)
              진짜 값 GE_PerfectParry.Duration 은 0.5 그대로
결과          3일간 퍼펙트 패링 창이 설계값(0.2)의 2.5배 · SB 실측(0.15)의 3.3배
```

**노브가 둘이면 하나는 반드시 거짓말을 한다.** 삭제로 진실을 하나로 만들었다.
난이도별 창 조절 같은 게 필요해지면 그때 배선한다(승환 판단).

🟠 **`GE_PerfectParry` 는 0.5 로 둔다** — 승환이 계속 테스트 중. **되돌릴 것.** 상세 = `CURRENT.md`

### 전수 조사 — 죽은 노브는 이거 하나뿐

`UPROPERTY(Edit*)` 스칼라 **178개**(float 119 · FName 27 · FGameplayTag 16 · bool 15 · int32 1)를
훑어 `PerfectParryWindowSec` 외에 같은 패턴은 없음을 확인했다.

⚠️ **하청 결과를 그대로 못 쓴 사례** — 서브에이전트는 "62개 전수 / int32 UPROPERTY 0건"이라
보고했으나 실제는 178개였고 `KDInputBufferComponent.h:30 MaxBufferSize` 가 있었다.
**개별 근거(`파일:줄` + 실사용 인용)는 정확했고 집계·커버리지 주장만 틀렸다.**
→ 앞으로 하청 프롬프트에 **"모집단 개수를 먼저 세어 보고하라"** 를 넣으면 구멍이 숫자로 드러난다.

## 4. 검증

```
빌드      통과 (승환) — .h 에서 프로퍼티를 지운 변경이라 풀빌드
정적 검사  옛 이름 잔존 0 — ShotRange · AutoAimConeAngle · PerfectParryWindowSec
          신규 배선 확인 — include · 멤버 · 생성자 2줄 · 오버라이드 · Radius 3곳
PIE       GA_ShotBlast 정상 (승환)
```

📌 `GA_ShotBlast` 의 `bDrawDebug = True` 는 **의도적으로 켜둔 상태**(승환, 테스트 중).

## 남은 것

```
조준 반경 분리   지금은 AutoAimFilter.Radius 하나. 필요해지면 판정용을 다시 가른다
GE_PerfectParry  0.5 -> 0.2(또는 0.15) 되돌리기. 테스트 끝나면
```
