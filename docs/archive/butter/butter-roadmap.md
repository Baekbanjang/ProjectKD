# 버터 개발 로드맵 — Phase 2 이후

> 출처: 구 플랜 `toasty-floating-scott.md`(2026-06-16~07-08)에서 미래 설계만 발췌 (2026-07-09 이관).
> Phase 0(임시 캐릭터 셋업)·Phase 1(MM 로코모션)은 완료/진행 — 상세는 활성 플랜 + dev-log 참조.
> 관련 문서: [butter-skills](butter-skills.md) · [butter-anim-pipeline](butter-anim-pipeline.md)

## 순서 원칙

**로코모션 정상화(Phase 1) → 전투 애니 교체+락온 MM(Phase 2) → 버터 스킬(Phase 3) → 보스 코미 → 아트/맵**

---

## Phase 2: 전투 애니메이션 교체 + 락온 MM

**목표**: 버터 전용 전투 모션 세팅 + ABP 전투 레이어 추가

### 2-1. 락온 스트레이프 — ✅ 완료 (2026-07-11)
- ~~PSD_Combat(MM) 구성~~ **폐기** → 길동 방식(BS 스왑, Blend Poses by Bool) 채택
- 애니 소스: Sword_Animations 번들 Run_Combat 10방향 리타게팅 (GS는 검든 자세라 배제)
- `BS_Butter_LockOn` (2D BlendSpace, VelocityX/Y 축) + ABP 2단 Blend Poses by Bool 분기
- 각도 처리: 몸은 락온 타겟 방향 고정(bUseControllerDesiredRotation), 8방향 strafe, OrientationWarping 미사용
- 상세 = [dev-log 2026-07-11](../dev-logs/2026-07-11-butter-lockon-strafe.md)

### 2-2. 전투 ABP 레이어
```
Base: MM 결과 (하체)
Layer: Slot 'DefaultSlot' → Layered Blend Per Bone (spine_01 이상)
Post: AimOffset (락온 시 상체 타겟 추적)
```

### 2-3. GA 몽타주 정리 — ⬜ 방향 전환 (2026-07-11)
- ~~GA_LightCombo / GA_HeavyCombo GhostSamurai 버전으로 확정~~ **보류** — GS는 버터 전투 톤에 무리 판단, 신규 애니(구매/직접 제작)로 교체 예정. 소스 미정 → 착수 시 결정
- DA_ComboTree 재구성 (맨손 LLL / LLH / HHL 분기) — 유효, 소스 애니 확정 후 진행
- Dodge / Parry / CounterThrust / SprintAttack 버터 몽타주 최종 연결 — 유효

### 2-4. 검증
- 이동 중 공격 시 하체 이동 유지되는지 (상하체 분리)
- 락온 전환 시 PSD 스위치 동작하는지
- 전투 중 strafe 8방향 부드럽게 블렌딩되는지

---

## Phase 3: 버터 전용 스킬

버서커 각성 / 도끼 전환 / 닻 게이지 / 콤보트리 → 상세 = [butter-skills](butter-skills.md)

버서커 진입 시 이동 애니도 교체 필요(PSD_Berserk 또는 기존 DB에 통합) — 착수 시 결정.

---

## Phase 4 (구 Phase 3): 보스 AI 프로토타입 — 코미

**목표**: 코미(수인 보스) 기본 패턴 구현

```
BP_BossKomi (BP_KDEnemy 기반)
- GA_EnemyLightAttack (GA_EnemyWeaponTraceBase 확장)
- GA_EnemyHeavyAttack
- GA_EnemyParry (카운터 조건)
- GE_KomiPhase2Trigger (HP 50% 이하 → 패턴 강화)
```

- Phase 1: 기본 근접 2타 + 회피 유도 패턴
- Phase 2 (HP 50%↓): 타락 어사이드 발동 → 속도 증가 + 광역 패턴 추가

---

## Phase 5: 아트 방향 결정 (전투 구축 후)

**전제**: 전투 완성 후 실제 플레이해보고 결정 (현재 하이브리드 보류 방침)

| 옵션 | 장점 | 단점 |
|---|---|---|
| 사실체 | 임팩트 강함, Rodin 에셋 즉시 활용 | 셰이딩 작업량 |
| 카툰(NPR) | 트릭컬 원작 감성 일치 | UE5 NPR 셰이더 구축 필요 |

결정 후 버터 3D 제작: Meshy/Rodin 생성 → Blender 리토폴로지 → AccuRIG → UE5 임포트 (목표 30~40k 쿼드, 2K PBR)

---

## Phase 6: 맵/환경 — 황혼 풍차마을 (병행 가능)

### 완료
- ✅ 4레이어 랜드스케이프 머티리얼 (Grass/Soil/Dirt/Moss)
- ✅ DirectionalLight 기본값 리셋

### 남은 것
- ⬜ 지형 Sculpting (풍차 언덕)
- ⬜ FBX 임포트: 풍차/가옥1~2/헛간/고목A~B/마른나무/잔해/울타리 (Scale=0.01)
- ⬜ 하늘/조명 최종 세팅 (맵 구성 완료 후)
  - 석양 오렌지 (#FF8C42), Intensity 3~5 lux, Pitch -15~-25
  - ExponentialHeightFog 밀도 0.02~0.05

> 에셋 위치/파이프라인 상세 = 메모리 `reference_butter_map_pipeline`
