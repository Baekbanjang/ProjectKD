# 버터 전용 스킬 설계 (Phase 3)

> 출처: 구 플랜 `toasty-floating-scott.md` Phase 3 발췌 (2026-07-09 이관). 전부 미구현 — 착수 시 이 문서 기준으로 §0 파일 목록 승인부터.
> 목표: 길동과 차별화되는 버터만의 전투 감각.

## 1. 버서커 각성 (Yellow Card)

```
GA_Berserk
- 트리거: 체력 30% 이하 또는 특정 입력 조합
- 효과: 공격속도 1.4x, 데미지 1.5x, 방어력 -20%
- 시각: GC_Berserk → 붉은 오라 NS + PP 효과
- 지속: 스태미나 소진 또는 수동 해제
```

- 이동 애니 교체 병행: 앞으로 숙인 자세·거친 호흡 Idle + 무거운 스텝 (PSD_Berserk 또는 기존 DB 통합 — 착수 시 결정)
- 각성 포즈 애니는 Cascadeur 커스텀 제작 예정 → [butter-anim-pipeline](butter-anim-pipeline.md)

## 2. 도끼 전환 시스템

```
GA_WeaponSwitch (맨손 ↔ 도끼)
- WeaponComponent 확장 (X2 소켓 방식 유지 — 메모리 project_weapon_socket_x2)
- 전환 시 납검/발검 애니 트리거 (기존 발검/납검 자동화 흐름 재사용 — 메모리 reference_weapon_sheathe_flow)
```

## 3. 닻 게이지 (감정 시스템)

```
AS_Butter에 Anchor 어트리뷰트 추가 (0~100)
- 만충 시 GA_AnchorBreak 해금 (전방 돌진 + 광역)
- 소진 시 버서커 강제 전환 리스크
```

> AttributeSet 8개 초과 시 분리 룰(프로젝트 CLAUDE.md §1-1) 체크.

## 4. 버터 콤보 트리 (맨손 기준)

```
Light × 3 → 피니셔 A (업퍼컷)
Light × 2 → Heavy → 피니셔 B (회전 어퍼)
Heavy × 2 → Light → 피니셔 C (그라운드 슬램)
```

> 기존 ComboTreeDataAsset 구조 재사용. Prefix 겹침 함정 주의 (메모리 reference_combo_prefix_conflict).
