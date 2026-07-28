# 버터 애니메이션 제작 파이프라인 + 락온 스트레이프 설계

> 출처: 구 플랜 `toasty-floating-scott.md` 발췌 (2026-07-09 이관).
> 참고: 이동 로코모션은 GASP 리타게팅(581개)으로 해결됨 — 이 문서는 **전투/시그니처 애니 확보** 방법.

## 도구 선택

| 도구 | 용도 | 비용 |
|---|---|---|
| Mixamo | 기초 애니 풀 (전투 대기/히트리액트/스트레이프) | 무료 |
| Cascadeur | 버터 시그니처 무브 커스텀 키프레임 | 무료(기본) |
| UE5 Control Rig | MM 보정용 절차적 애니 (발IK/조준) | 내장 |

## 제작 목표 애니 목록

```
[Mixamo 발굴]
- AS_Butter_Idle_Combat       (전투 대기 — 긴장된 맨손 자세)
- AS_Butter_Walk_Combat       (전투 중 걸음 — 무게감)
- AS_Butter_HitReact_F/B/L/R  (방향별 피격 반응)
- AS_Butter_Knockdown         (쓰러짐)
- AS_Butter_GetUp             (일어남)

[Cascadeur 커스텀]
- AM_Butter_Berserk_Activate  (버서커 각성 포즈 — 손발 벌림+포효)
- AM_Butter_Axe_Heavy_01~03   (도끼 강공격 3종)
- AM_Butter_Anchor_Break      (닻 게이지 방출 — 전방 돌진)
- AM_Butter_Death             (사망 — 무릎 꿇고 쓰러짐)
```

## Mixamo → SKEL_UE5_F 리타게팅 절차

1. mixamo.com → "Y Bot" 선택 → 애니 다운로드: **FBX Binary, 30fps, Skin 포함**
2. UE5 임포트: Import Mesh OFF, Import Animation ON
3. Mixamo는 전용 스켈레톤 → **Mixamo용 IKRig 한 번 세팅** → IKRetargeter(IK_Mixamo → 기존 IK_SKEL_UE5_F 재사용) → 배치 리타게팅

Cascadeur는 UE5 Manny 기준 제작 → SKEL_UE5_F 직행 (기존 RTG 재사용).

---

## 락온 스트레이프 설계 (Phase 2 PSD_Combat 재료)

> IsInCombat=true, 몸은 락온 타겟 방향 고정. GASP Box/Arc 스트레이프 리타게팅본이 1차 재료 — 부족분만 Mixamo 발굴.

| 포함 애니 | 각도/방향 | 비고 |
|---|---|---|
| Combat_Idle | - | 무기 들고 대기 (무릎 약간 굽힘) |
| Strafe_F | 0° / 250cm/s | 전진 스트레이프 |
| Strafe_B | 180° / 200cm/s | 후진 |
| Strafe_L / R | ∓90° / 220cm/s | 좌우 스트레이프 |
| Strafe_45FL/FR | ±45° | 대각 전진 |
| Strafe_45BL/BR | ±135° | 대각 후진 |
| Combat_Pivot_L/R | ±90° | 전투 중 방향 전환 |
| Combat_Step_B | 180° | 빠른 백스텝 (회피 예비) |

### 이동 속도 계층 (참고 기준)

```
Idle:      0 cm/s
Walk:      0 ~ 200 cm/s   (MaxWalkSpeed * 0.4)
Jog:       200 ~ 380 cm/s (transitional)
Run:       380 ~ 600 cm/s (MaxWalkSpeed = 500)
Sprint:    600+ cm/s      (스프린트 GA 활성 시)
```

> 동적값 원칙(메모리 feedback_adaptive_values): 고정 상수보다 CMC MaxWalkSpeed 기준 비율로.

### 각도 처리 원칙

```
일반 이동: OrientationWarping — 몸이 이동방향으로 회전, 45° 대각 애니 + Warp로 중간각 채움
전투 이동: 몸은 락온 타겟 고정 — 8방향 strafe 필수, OrientationWarping 사용 안 함
Turn-in-place: 속도<50cm/s + 회전량>45° → MM이 Turn 애니 자동 선택 (Trajectory Facing 기반)
```
