# 2026-08-26 — 개명 에디터 검증 + GCN 큐 맵 함정 + 전수 재저장

클래스 68개 개명(`2026-08-26-class-naming-convention.md`)의 에디터 쪽 마무리.
검증 → PIE → 큐 3개 복구 → 전수 재저장까지. **리다이렉트 의존이 끊겼다.**

## 1. 검증 결과 (읽기 전용 조회, sonnet 하청)

```
GA BP 15개        부모 전부 KDGameplayAbility_* 새 이름 해석. 구이름·None·REINST 0건
몽타주 55개        노티 증발 0건. per-window 오버라이드(calf_r/ball_r 등) 보존
적 DA 6종         MaxPoise 15/10/25(불멸 99999) · KnockbackDistance 200 · PoiseDamageByAttack 3키 일치
콤보 트리          22+4 = 26노드. PoiseMultiplier 칸 전부 0 (의도)
```

AbilityTags 빈 BP 4개는 전부 정상 — `GA_PlayerExecution` 은 태그 발동이 아니라
`Event.Combat.ExecutionStarted` 트리거 발동(`KDGameplayAbility_PlayerExecution.cpp:19`),
나머지 3개(HitReact/PerfectParryReaction/WeaponToggle)는 엔진 `GameplayAbility` 직결 BP.

## 2. PIE 회귀 — 처형 GC 만 죽어 있었다

승환 실측: 콤보·패링 3종·회피·총격·넉백·공중 콤보 전부 통과. 처형 로직도 통과.
**처형 카메라 연출만 안 나옴.**

### 원인 — 큐 매니저 스캔에 ClassRedirects 가 안 먹는다

`GameplayCue.PrintGameplayCueNotifyMap` 덤프로 확정. 맵에서 빠진 큐 = 정확히
**개명된 KD C++ GCN 클래스를 부모로 둔 BP 3개**. 엔진 클래스 직속 BP 8개는 전부 정상.

```
GCN_ExcutionCamera       GameplayCue.Camera.Execution      처형 카메라
BP_GCN_HitImpact_Light   GameplayCue.Combat.HitImpact.Light 경타 히트 임팩트
BP_GCN_EnemyTelegraph    GameplayCue.Enemy.Telegraph.*      적 전조 링
```

경로·태그·DA 값·C++ 상속 전부 정상 실측 — 남는 원인은 큐 매니저가 BP 후보를 고르는
클래스 상속 판정이 에셋에 구워진 옛 부모 문자열을 새 클래스의 자손으로 못 알아본 것.
**PIE 를 처형만 보고 넘겼으면 전조 링·히트 임팩트 증발을 놓칠 뻔했다** — 맵 덤프가 잡았다.

### 해결 = BP 3개 재저장 + 에디터 재시작

재저장으로 새 부모 이름을 굽고(옛 클래스 경로 grep 0건 확인), **재시작이 필수** —
큐 맵은 에디터 시작 시 스냅샷이라 세션 중 재저장으로는 안 갱신된다(재덤프로 실측).
재시작 후 세 큐 전부 맵 복귀 + 처형 카메라 PIE 정상.

## 3. 전수 재저장 (Content `372dcd6` · `f408086`)

명단 스캔(13GB grep) 대신 **참조 가능 클래스 필터**로 범위 확정 — 승환 결정.

```
대상    /Game/SB_Style_GameProject + /Game/Blueprints 에서
        AnimMontage·Blueprint 계열·KD* DataAsset·UserDefinedStruct/Enum = 312개
제외    AnimSequence 223 등 우리 클래스를 참조할 수 없는 타입 (재압축 낭비)
결과    312개 저장 실패 0 / 바이트 변화 246 파일 커밋
```

### 검증 3중

```
풀패스 옛 참조     Project_KD.(ANS_|AN_|AT_|GA_|GE_|AS_|GCN_)  ->  0건
새 임포트 실재     몽타주에 KDAnimNotifyState_* / BP 에 KDWeaponComponent 등 확인
잔존 옛 문자열     전부 무해 2종 — 노티 표시명 캐시(GetNotifyName 오버라이드가 덮음)
                  + 컴포넌트 인스턴스명(서브오브젝트 리네임 고아 함정으로 의도적 유지)
```

## 4. 곁가지 발견

```
Combat.Execution 큐   받는 GCN 이 프로젝트 전체 0개 (레지스트리 실측).
                     KDExecutionComponent.cpp:92 가 허공에 쏜다. 피니셔 연출은 몽타주 쪽.
                     처형 폴리싱 때 배선하거나 호출 삭제
CBP_SandboxCharacter  /Game/Blueprints/ 의 Epic 샘플 잔재. 삭제된 샘플 트래버설 함수
                     (GetLedgeTransforms) 참조로 원래부터 컴파일 깨짐. 정리 후보
MCP execute_python    대량 재저장 후 세션 중 사망(에디터는 생존, 다른 액션 정상).
                     디스크 grep 으로 검증을 대체했다
```

## 5. 남은 것

```
DefaultEngine.ini +ClassRedirects 80줄   당장 안 지운다. 며칠 문제없이 돌면 삭제
                                        (외부 두 폴더 밖 stale 에셋은 재저장 안 했으므로
                                         지우면 killdong 잔재 몽타주 노티가 죽는다 — 감수)
볼트 코드구조 10문서                      검증 통과 신호 발신 — 볼트가 개명 반영 시작 가능
```
