# 2026-07-17 트레이스 무기중립화 리팩토링 (WeaponTrace → MeleeTrace)

버터 맨손 콤보(옵시디언 `Project_New/모션기획` 12모션) 착수 선결과제. 무기 전제 트레이스를 맨손/킥까지 담는 무기중립 구조로 전환.

## 왜

기존 트레이스는 "무기 메쉬"를 전제했다. `OnTraceBeginEvent`가 `Weapon` 태그 스켈레탈 컴포넌트를 찾고, 못 찾으면 판정 자체를 스킵. 소켓 기본값도 `Spear_Bottom/Tip`(킬동 창 잔재). 무기 컴포넌트 없는 캐릭터(맨손 버터, 맨손 적, 킥 공격)는 공격 판정이 아예 불가능했다.

## 무엇을 바꿨나

### 1. 리네임 (무기중립 이름)

| 기존 | 변경 |
|---|---|
| `GA_WeaponTraceBase` | `GA_MeleeTraceBase` |
| `ANS_WeaponTrace` | `ANS_MeleeTrace` |
| `AT_WeaponTrace` | `AT_MeleeTrace` |
| `GA_PlayerWeaponAttackBase` | `GA_PlayerMeleeAttackBase` |
| 팩토리 `WeaponTrace()` | `MeleeTrace()` |

- 라이더(Unreal 플러그인)가 리네임 시 `DefaultEngine.ini [CoreRedirects]`에 리다이렉트 자동 생성(Class 4종 + Function 1종). 기존 몽타주 에셋의 노티 참조 보존 확인.
- 적 GA 계열(`GA_EnemyWeaponTraceBase`)은 유지 — 적은 무기 사용, 스코프 밖.

### 2. 판정 출처를 데이터로 승격

`ETraceMeshSource { Weapon, OwnerBody }` 신설 (`AT_MeleeTrace.h`). "어디로 긋는가"가 코드 가정이 아니라 데이터가 됨.

- **GA 층** (`GA_MeleeTraceBase.h`): `MeshSource` 기본값 `Weapon` — 기존 무기 GA는 동작 무변경
- **노티 층** (`ANS_MeleeTrace.h`): `bOverrideMeshSource` + `MeshSourceOverride` — 창마다 출처 전환(무기 콤보 중 킥 한 방만 OwnerBody 등). 기존 `bOverrideTraceMode` 패턴과 대칭

### 3. `OnTraceBeginEvent` 재구성 (`GA_MeleeTraceBase.cpp`)

- 오버라이드 계산을 메쉬 검색보다 **앞으로** 이동 + `EffSource` 추가 (메쉬를 고르려면 소스를 먼저 알아야 함)
- `OwnerBody`면 `GetCurrentActorInfo()->SkeletalMeshComponent`(본체 메쉬), 아니면 기존 Weapon 태그 검색
- **소켓 유효성 가드**: OwnerBody인데 소켓이 본체에 없으면(`DoesSocketExist` false — 소켓+본 둘 다 검사) 경고 후 return. 노티 소켓 오버라이드 누락 시 몸통 원점 헛트레이스를 표면화 (`ensureMsgf(AssetTags)` 패턴과 일관)
- **판정창 단위 히트 리셋**: `AlreadyHitActors.Reset()`를 활성화 1회가 아니라 판정창 시작마다 호출. 한 몽타주 2연타(속4 바디→헤드)가 같은 적에게 각각 적중. 창 1개짜리 기존 몽타주는 무변경

## 검증

- UBT 빌드 통과 (UHT 리플렉션 20개 생성, 컴파일 무에러)
- 기존 무기 공격 회귀 검증: 몽타주 노티 생존(CoreRedirect 작동), 판정/데미지/나이아가라 정상
- code-reviewer(§5-2 게이트): CRITICAL/HIGH 0, 룰 위반 0. MEDIUM 1건(소켓 미오버라이드 헛트레이스) → 가드로 반영 완료

## 안 한 것 (연기)

- **데미지 계수**: branch 아니라 `ANS_MeleeTrace`에 둘 것(멀티히트 3모션 = 판정창별 계수 다름). 밸런싱 착수 시
- **폼 태그 / 트리 스위칭**: 도끼 폼(M1 후반) 시점. 태그명 = Axe(Spear 아님)
- **주석 잔재**: 구이름 `GA_WeaponTraceBase` 참조 주석 몇 개(선택), `FWeaponTraceHitDelegate`/`SCENE_QUERY_STAT` 라벨(코스메틱)

## 커밋

- `4ff5b9e [refactor] 트레이스 무기중립화 — WeaponTrace→MeleeTrace + MeshSource 분기`
- `31247d3 [chore] 에디터 시작맵 LV0_Test로 변경`

## 다음 (Phase 2 — 맨손 콤보 실배선)

계층 확인 결과 맨손용 새 코드 거의 불필요(`GA_PlayerMeleeAttackBase`가 히트스탑만, 무기 전용 로직 없음). 기존 GA에 데이터만 교체:

1. 맨손 애니 선별 (마네킹 Manny + Lucy/MartialArts/Motifect/RamsterZ 팩 -> 12모션 매핑)
2. 몽타주화 + 노티 배치 (`ANS_MeleeTrace` MeshSource=OwnerBody + hand_l/foot_r 소켓, CancelWindow, 속4·피니A는 판정 노티 2개+)
3. 맨손 ComboTree 에셋 신설 (피니셔 5 branch + 중간타 7 = GA DefaultAttackMontages)
4. 에디터 세팅 (맨손 GA에 MeshSource=OwnerBody + 맨손 몽타주/트리)
5. PIE 검증 (hand_l 트레이스 / 콤보 연결 / 속4 다단히트)
