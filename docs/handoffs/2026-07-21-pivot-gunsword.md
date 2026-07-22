# 프로젝트 피벗 — Gun & Sword 액션 (2026-07-21)

> 볼트 세션에서 결정 완료. 상세 기획 = 볼트(`C:\Users\asdasd\Desktop\Obsidian_organize\ProjectKD`)
> `notes/Project_New/신규기획_GunSword_액션_v0.md` · `GunSword_콤보구성_v0.md`

## 1. 버터(트릭컬 팬게임·맨손 복싱) 기획 폐기

- 사유: 복싱 애니 소스 부재 (보유 팩 전부 리얼 모캡 지향 — 스타일리시 임팩트 상한선 낮음)
- **"맨손 트레이스 분기" 선결과제 취소** — hand_l/r·foot 소켓, GA_WeaponTraceBase 분기 논의 폐기
  - 단, 이미 커밋된 MeleeTrace 리팩토링(`4ff5b9e`, WeaponTrace→MeleeTrace + ETraceMeshSource)은 무기중립 구조라 그대로 유효

## 2. 새 방향

- **9CG "Gun & Sword" 애니팩** (993클립, Fab, **아직 미구매**) 기반
- 지상 중심 스텔라 블레이드 스타일 액션. 검+총 → 기존 무기 트레이스 경로 그대로 적합

## 3. 확정 사항

- 톤: 지상 중심, **패링 긴장감 + 타격감 코어**. 공중전은 소비중(미니 그래프)
- 공격 RM(루트모션) + 이동 IP(인플레이스) — 클립 확인 후 최종 승인 예정
- 주인공 보류 — **마네킹(Manny) + 팩 애니로 전투 완성 먼저**, 스킨은 나중에
- 1차 스코프: **스테이지 1개(잡몹 5~10분) + 보스 1체**
- 밸런스 초깃값 = SB 덤프 실측치
  - 패링 창 0.1s / 라이트어택 계수 0.7·1.0·1.0·1.8 / 판정 0.1s / 경직 0.3s
  - 출처: 볼트 `notes/Reference/StellarBlade_이브_실측값_데이터시트.md`

## 4. 코드 작업 — 팩 구매 후 시작 (지금은 대기)

1. 임포트 → Manny 리타겟
2. 지상 콤보 1체인을 ComboTree에 연결
3. PIE 수직 슬라이스
- 무기폼 전환 시 ComboTree 스위칭 이슈는 여전히 유효 (M1 후반 과제 유지)

## 참고 — 폐기 직전 버터 맨손 작업 상태 (재활용 대비 기록)

- Fist05_1~4 포함 주먹/킥 몽타주 다수에 노티(MeleeTrace OwnerBody+소켓 / CancelWindow / MovementCancel / AttackSound / WeaponTrail) 배치 완료 상태로 잔존
- GA 5종(LightCombo/HeavyCombo/AirLightAttack/SprintAttack/CounterThrust) MeshSource=OwnerBody로 변경됨 → **무기 복귀 시 Weapon으로 되돌려야 함**
- BP_ButterCharacter WeaponComponent: CurrentWeapon/WeaponMeshAsset=null, bUseSheathing=false → **무기 복귀 시 복원 필요**
- `DefaultGame.ini`에 `+GameplayCueNotifyPaths=/Game/TrickalFanGame/GAS/GC` 추가됨 (유효, 유지)
