# 2026-08-27 — 적 상태 바를 락온 외에 피격에서도 표시

락온이 유일한 스위치였던 것을 **표시 이유 두 개의 OR** 로 바꿨다.

## 왜

승환 요청 = **"적이 공격받으면 상태 바를 띄우고, 몇 초 안 맞으면 사라지게."**
종전에는 `OnTargeted_Implementation` 이 `SetVisibility(bIsTargeted)` 를 직접 호출하는 게 전부였다.
BP 그래프 개입은 0 — 표시 경로가 C++ 한 줄뿐이었다.

## 설계 — 각자 SetVisibility 를 부르면 안 된다

이유가 둘이 되는 순간 서로 덮어쓴다. **락온을 푸는 순간 피격으로 켜둔 바까지 꺼진다.**
그래서 플래그 두 개를 두고 합쳐서 판단하는 함수 하나를 경유시켰다.

```
표시 = bStateBarLockedOn OR bStateBarRecentHit
```

피격은 순간이라 타이머로 상태화한다.

## 배선

```
KDEnemyBaseCharacter.h
    StateBarHitDuration = 4.f      EditDefaultsOnly — 적 BP 마다 조절
    RefreshStateBarVisibility()    두 플래그를 합쳐 반영
    ClearStateBarHitFlag()         타이머 콜백
    bStateBarLockedOn / bStateBarRecentHit / StateBarHitTimer

KDEnemyBaseCharacter.cpp
    OnTargeted_Implementation   플래그만 세우고 Refresh 호출
    OnHitReceived:452           bIsDead 가드 "다음" 에 플래그 + 타이머 재설정
    HandleDeath:247             bIsDead = true 직후 플래그 2개 해제 + 타이머 클리어
```

⚠️ **`HandleDeath` 쪽이 빠지면 죽은 적 위에 바가 떠 있는다.** 락온 중이던 적은 락온이 풀리며
꺼지지만, 피격으로 뜬 바는 타이머가 돌 때까지 남는다.
⚠️ `OnHitReceived` 삽입 위치도 가드 **다음** 이어야 한다. 앞에 넣으면 죽은 적이 맞아도 바가 뜬다.

## 동작

```
때리기만          뜨고 4초 뒤 사라짐
락온만            뜨고 풀 때까지 유지
때린 뒤 락온 풀기  4초는 남아 있다        <- OR 로 합친 효과
죽이면            즉시 사라짐
```

## 곁들여 — 클래스 책임 점검 (승환 지시)

"줄 수로 자르지 말고 기능이 과다한지로 보라" 는 방침에 따라 `AKDEnemyBaseCharacter` 를 훑었다.

```
과다한 것    AI 게터 11개 — GetSightRadius · GetAttackRange · GetStandoffRange 등
            전부 `EnemyDefinition ? X : 기본값` 한 줄. Pawn 책임이 아니라 DA 읽는 통로다
            이미 GetEnemyDefinition() 이 public 이라 호출자가 직접 읽어도 된다
적정한 것    사망·랙돌(Pawn 고유) / 컴포넌트 5개 델리게이트 조율(Pawn 역할)
            피격 분배(B2 에서 이미 4함수로 분해) / 상태 바(위젯 소유자가 곧 Pawn)
```
→ **상태 바 추가는 과다가 아니다.** 정리할 일이 생기면 그 게터 11개가 대상.

## 검증

```
빌드   통과 (승환)
PIE    정상 동작 확인 (승환)
```

## 남은 것

**적 상태 바 칸 폭** — 엘리트 `MaxPoise 25` 면 위젯이 폭을 `25 x 10 = 250px` 로 잡는데 바는 198px 다.
```
A안 (권장)  총 폭 198 고정 + 칸 폭 = 198 / MaxPoise   SB 방식. 값이 뭐가 되든 안 깨진다
B안         Min(MaxPoise x 10, 198)                  한 줄이나 20 이상이 전부 같아 보인다
C안         배율 10 -> 7                              Poise 30 짜리가 나오면 또 넘친다
```
⚠️ `DA_Dummy_Immortal` 은 `MaxPoise 99999` — 칸 개수 상한을 두지 않으면 폭발한다.
📌 위젯 실물은 **MCP 가 끊겨 못 봤다.** 위 구조는 `CURRENT.md` 기록 기준이라 열어서 대조할 것.
