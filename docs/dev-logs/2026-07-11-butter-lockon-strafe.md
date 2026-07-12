# 버터 락온 스트레이프 시스템 (Phase 2 첫 기능)

> 2026-07-10 ~ 07-11. 락온 이동 = 길동 방식(BS 스왑) 확정, PSD_Combat(MM) 계획 폐기.

## 구현 내용

### 애니 소스: Sword_Animations 번들 (GS에서 변경)
- 사유: GS는 검 든 자세 기준 이동이라 맨손 버터에 부담 → Sword 번들 Run_Combat 10방향으로 교체
- 길동 락온도 형제 세트(Spear 번들) 리타게팅본 — 검증된 계보
- GS용 `RTG_Mannequin_to_UE5F`는 유지 (Phase 2 전투 몽타주용)

### 리타게팅 체인 (신규 에셋)
| 에셋 | 내용 |
|---|---|
| `Retarget/IK_Sword_Mannequin` | Cha_1_IKRig 복제 후 체인 20개를 타겟(IK_SKEL_UE5_F) 체계로 재구성 — 원본 체인명 비표준이라 FUZZY 매핑 시 몸 붕괴, EXACT 매핑용 |
| `Retarget/RTG_Sword_to_UE5F` | 소스 IK_Sword_Mannequin → 타겟 IK_SKEL_UE5_F, EXACT 20/28 (미매핑 8 = 메타카팔, 무해) |
| `Animation/LockOn/AS_Butter_LockOn_*` | 배치 리타게팅 11개 (8방향 Loop + F_0 등 + Idle_Combat) 전부 인플레이스 0.667s |

### BS_Butter_LockOn (2D BlendSpace)
- 축: VelocityX / VelocityY, **-550~550**, grid 4 (길동 BS_Player_Loco_LockOn 복제 구성)
- 샘플: **8방향 링만, 반지름 500** (= MaxWalkSpeed, 대각 ±354) — Idle은 BS에 넣지 않음 (아래 참조)
- rate 전부 1.0 (보정 불필요 판명)

### ABP_Butter_MM 변경
- **부모 교체**: AnimInstance → `UKDPlayerAnimInstance` (VelocityX/Y·bIsLockedOn 등 C++ 공급, 버터 폰 = KDPlayerCharacter 자손이라 캐스팅 성공)
- BP 로컬 `bIsInAir` 삭제 → 부모 상속본으로 교체 (Set 체인 삭제 + exec 재연결)
- AnimGraph 2단 스위치:
  ```
  Idle 시퀀스 ─→ [Blend#2 False]      MM ──────→ [메인Blend False]
  BS_LockOn ──→ [Blend#2 True]   Blend#2 ──→ [메인Blend True]
  bIsMoving ──→ [Blend#2 Active]  IsLockedOn → [메인Blend Active]
  → 메인Blend → Pose History → Slot 'DefaultSlot' → Output   (블렌드 전부 0.2s)
  ```
- Pose History가 블렌드 뒤라 락온 해제 시 MM이 스트레이프 포즈에서 이어받음 (복귀 팝 없음)

## 핵심 문제/해결

1. **BS "Idle 오염"** — 10초 Idle이 BS에 섞이면 재생 주기가 가중평균으로 늘어져 느릿느릿 (조깅 500 체감 0.44배속). rate 2.0으로도 해결 안 됨. 스프린트 700에선 축 클램프로 Idle 0% = 자연스러움이 결정적 증거. **처방 = Idle을 BS 밖으로 (bIsMoving 분기)** — 오염 원천 차단. ⚠️ 길동 BS도 동일 잠재 문제 (Idle 10s 포함) — 폴리싱 후보
2. **UE Python FixedArray 함정** — `params[i].set_editor_property()`는 복사본만 수정. `p = params[i]; p.set_...; params[i] = p` 후 전체 재대입 필수 (축 min/max 미적용 원인)
3. **에디터-Python 동시 수정 충돌** — 에셋 탭 열린 채 Python 저장 → 에디터 저장이 덮어씀. 규칙: 수정 채널 하나로 통일 (탭 닫고 Python)
4. **T-포즈 (비락온)** — bIsInAir Set 노드 삭제 시 exec 선 미재연결 → Trajectory/DB 갱신 중단 → MM 검색 실패. 락온(C++ 변수만 사용)은 정상이라 원인 특정 가능했음
5. **제작 기준 속도 측정법** — RM 쌍둥이 버전의 루트 이동거리 ÷ 길이 = 443cm/s (Sword Run). 애니가 어떤 속도 기준인지 데이터로 판정

## 검증 (PIE)
- ✅ 락온 8방향 스트레이프 (몸 타겟 고정, 조깅/스프린트)
- ✅ 정지 시 Idle_Combat / 이동 전환 0.2s 블렌드
- ✅ 락온 OFF → MM 복귀 팝 없음
- ✅ 느릿느릿 해소 (Idle 분리 후)

## 남은 것
- 락온 Start/Stop 20개 추가 리타게팅 (출발/정지 보강 — 필요 시)
- 길동 BS Idle 오염 동일 수술 (폴리싱)
- 로드맵 §2-1 stale (PSD_Combat 기술) — 본 문서가 최신
