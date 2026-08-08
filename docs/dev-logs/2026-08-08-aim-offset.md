# 2026-08-08 — 조준 AimOffset (상체 오프셋)

총 조준 1단계의 마지막 조각. 조준 중 상체가 겨누는 쪽으로 따라 틀어진다.

**커밋** — 소스 `8a658dd` `[Combat] 조준 스탠스 폴리싱 + AimOffset 축 배선` / Content `bae681d`
**설계 문서** — 볼트 `notes/코드구조/09_조준_AimOffset.md`

---

## 구현

```
마우스 상하 → ControlRotation.Pitch → RailAlpha(0~1) → AimPitch(-1~+1) ┐
마우스 좌우 → TurnYawOffset(도)                      → AimYaw(-1~+1)  ┘
                                                                       │
                                        ABP_SB  AimOffset Player 'AO_SB_Aim'
                                                X = AimYaw   Y = AimPitch
```

| 손댄 것 | 내용 |
|---|---|
`KDSpringArmComponent.h/.cpp` | `RailAlpha` 멤버 + `GetRailAlpha()` — 이미 계산하던 지역 변수를 밖으로 |
`KDPlayerCharacter.h/.cpp` | `AimPitchScale`(EditAnywhere) + 게터 2개 |
`KDPlayerAnimInstance.h/.cpp` | `AimPitch` `AimYaw` `AimYawRange` + 캐시 2개 |
`AO_SB_Aim` (신규 에셋) | Aim Offset 2D, 축 -1~+1, 9칸 |
`AS_Idle_Aim_*` 9개 | Mesh Space Additive 변환 |
`ABP_SB` | `LayeredBoneBlend` 뒤에 `Aim Offset` 노드 1개 |

## 결정 3개

**① 축은 각도가 아니라 -1 ~ +1**
SB `Aim_BS.json` 실측이 `Min -1 / Max 1`(레일건·낚시 BS도 동일). 우리 쪽 이유가 하나 더 — 조준 레일 좌표가 영상 프레임 추정치라 앞으로 반드시 바뀐다. 각도를 축에 박으면 레일을 손댈 때마다 BlendSpace를 같이 고쳐야 한다.

**② 입력값은 레일 진행도 `RailAlpha`**
카메라 위치를 정하는 값과 상체 각도를 정하는 값이 **같은 소스**라 어긋나지 않는다. 상수 0개.
⚠️ `ControlRotation.Pitch`를 각도로 쓰면 안 된다 — 카메라 개편(08-03) 이후 그건 레일 눈금이다. 락온이 이 함정에 빠져 한 번 죽었다(08-05).

**③ Additive는 Mesh Space (SB는 Local)**
`UAimOffsetBlendSpace::IsValidAdditiveType`이 `AAT_RotationOffsetMeshSpace`만 받는다(`AimOffsetBlendSpace.cpp:18`). SB가 Local인 건 일반 BlendSpace를 골라서 딸려온 결과일 수 있다 — "Local이 낫다"는 근거가 아니다.
우리는 조준 로코모션이 모션매칭 33개 클립을 매 프레임 갈아끼워 베이스가 계속 바뀐다. 부모를 무시하는 Mesh가 안전하다.

## `AimPitchScale` — 왜 필요했나

```
마우스 풀 스윙   -89 ~ +45  = 134도   →  상체는 포즈를 끝까지 다 씀
같은 동안 카메라 -17.3 ~ +25.0 = 42도

134 ÷ 42 = 3.2배
```
승환 체감("1 움직였는데 3 움직인 느낌")과 맞았다.

**정확한 배율은 계산으로 못 낸다** — 팩 포즈가 몇 도인지 모른다(뼈 각도 측정 실패). 노브를 빼서 화면 보며 맞춘다. `0.33`은 체감을 뒤집은 출발점.
액터(`AKDPlayerCharacter`)에 `EditAnywhere`로 둔 이유 = **PIE 중 아웃라이너에서 실시간 조절**. `EditDefaultsOnly`로 AnimInstance에 두면 못 바꾼다.

## 검증 (PIE 5항목 전수 통과)

마우스 상하 추종 / 해제 시 중앙 복귀 / 걸으며 조준 시 총구 흔들림 없음 / 검 콤보·회피 회귀 없음 / 어깨·목 정상

## 함정 4개

1. **Preview Base Pose에 Additive를 넣으면 화면이 빈다** — 전부 0인 포즈를 베이스로 삼아 뼈가 원점으로 무너진다. `AAT_None`인 애니를 써야 한다
2. **9포즈 전부 같은 Base Pose Animation을 봐야 한다** — 각자 자기 자신이면 9개가 전부 0이 되어 아무 일도 안 일어난다
3. **`Weight Speed`는 축 설정이 아니라 `Sample Smoothing` 섹션**에 있다. 놓치기 쉽다
4. **1프레임 클립을 PoseSearch DB에 넣지 마라** — `PSD_SB_Idles_Aim`이 `AS_Idle_Aim_0_Seq`(0.033초)를 물고 있어 매 틱 검색 실패. `AS_Aim_the_Target_Loop_Seq`(10초)로 교체

## 곁가지 — 진입 몽타주가 안 나오던 것도 같이 잡았다

`ABP_SB`의 `Slot 'UpperBody'` 노드도 배선도 정상이었다. **에셋 값이 문제였다.**
`AS_Aim_the_Target_Start_Seq_Montage`가 길이 0.25초인데 Blend In/Out이 각각 0.25초 → `BlendOutTriggerTime -1`이라 아웃이 **t=0에서 시작** → 가중치가 뜨기 전에 내려갔다.
`AM_SB_Aim_the_Target_Start`(In 0.05 / Out 0.10)로 교체해 해결.
→ **몽타주가 안 보이면 슬롯을 의심하기 전에 길이와 블렌드 시간을 재라.**

## 남은 것

- `AimPitchScale` 확정 → `BP_SBPlayer` 기본값에 박기 (PIE 값은 끄면 날아감)
- **좌우 6포즈는 사실상 안 쓰인다** — 캐릭터가 카메라 쪽으로 돌아 `TurnYawOffset`이 0으로 수렴. SB도 `RotateAnglePerSecond 720`으로 즉시 정렬한다(전 스탠스 동일). 발사가 붙은 뒤 재검토
- 조준 중 상하 범위가 42도뿐 — 좁으면 조준 레일 양 끝 Z를 벌린다
- 공격 중 조준 진입 차단 — `GA_AimMode`의 `Activation Blocked Tags`에 `State.Combat.Attacking`
