# 2026-07-09 — 모션매칭(Motion Matching) 자료조사

버터 로코모션 재구축(Phase 1) Step 3 착수 전 사전 조사. 개념·역사·트레이드오프(웹) + PoseSearch 에셋 내부 구조(UE5.6 엔진 헤더 직독).

## 핵심 요약

- **정의**: 상태머신 배선 대신 매 프레임 애니 DB에서 "지금 상황에 최저 비용인 프레임"을 검색해 트는 기술. 전환 로직 없음
- **비용 함수** = 포즈(뼈 위치) + 뼈 속도 + 궤적(미래)의 가중합. 궤적만→발 미끄러짐, 위치만→관성 무시로 떨림 → 속도 필수
- **근본 트레이드오프**: 품질(포즈 가중↑) vs 반응성(궤적 가중↑)은 경쟁 관계 → 발 슬라이딩/굼뜸 튜닝 = 스키마 채널 가중치
- **For Honor 실전 관행**: 뼈 몇 개만(발+골반) + 속도 포함, ~10항목 — GASP 스키마가 pelvis/foot_l/foot_r 3뼈인 근거
- **"애니 추가 = 품질 향상"** (배선 불필요). 검색 비용은 PCAKDTree(PCA 압축+KD트리)로 완화

## 검증된 사실 (엔진 헤더 직독, `D:\epicStore\UE_5.6\Engine\Plugins\Animation\PoseSearch\Source\Runtime\Public\PoseSearch\`)

| 에셋 | 역할 | 근거 |
|---|---|---|
| PSS (Schema) | 채점 기준표 — Channels(항목+Weight) + Skeleton + MirrorDataTable + SampleRate(30) | `PoseSearchSchema.h:44-89` |
| PSD (Database) | 애니 서랍 + 사전 계산 인덱스 — Schema 1개 + 엔트리별 bEnabled/MirrorOption/SamplingRange | `PoseSearchDatabase.h:419-459` |
| PSN (NormalizationSet) | DB 간 점수 환산기 (내용물 = DB 목록뿐) — 여러 DB 동시 검색의 전제 | `PoseSearchNormalizationSet.h:14-23` |
| CHT (Chooser) | 조건표 선택기 (ABP 변수 → 행별 DB 반환). 우리는 미사용, ABP 분기 3개로 대체 | GASP 디스크 조사 |
| MirrorDataTable | 좌우 뒤집기 규칙표. **스켈레톤 스왑 시 무효 → 복제 스키마에선 None** | `PoseSearchSchema.h:48` |

**PSD 튜닝 노브**: `ContinuingPoseCostBias` -0.01(현재 애니 유지 선호, 떨림 방지) / `BaseCostBias` 0(DB 전체 핸디캡) / `ExcludeFromDatabaseParameters` (0,-0.3)(끝 0.3초 선택 금지 — 블렌드 여유)

**공식 문서 추가 노브**: Search Throttle Time(검색 빈도) / Pose Jump Threshold(근처 프레임 재선택 금지) / **Anim Notify Filtering 기본 0.2s — 포즈 점프 시 노티 중복 방지, GA 몽타주 노티와 연관 주의** / 커버리지 부족 시 Animation Warping 권장

## 참고 URL

| 출처 | 내용 |
|---|---|
| [Motion Matching in Unreal Engine — Epic 공식](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-matching-in-unreal-engine) | 시스템 구성요소·튜닝 노브·베스트 프랙티스 (본문 확인) |
| [Motion Matching Debugging — Epic 공식](https://dev.epicgames.com/documentation/en-us/unreal-engine/motion-matching-debugging-in-unreal-engine) | Rewind Debugger 기반 Pose Search Debugger — 다이어트 도구 |
| [Motion Matching in O3DE — 기술 블로그](https://docs.o3de.org/blog/posts/blog-motionmatching/) | 비용 함수 원리 (포즈/속도/궤적이 각각 왜 필요한지 최고 설명) |
| [Motion-Matching in Ubisoft's For Honor — Game Anim](https://www.gameanim.com/2016/05/03/motion-matching-ubisofts-honor/) | 2016 GDC 원조 발표 정리, 실전 팁 |
| [Motion Matching: The Future of Game Animation — MoCap Online](https://mocaponline.com/blogs/mocap-news/motion-matching-games-guide) | 개념 개요 + 업계 사용 현황 |
| [unreal.PoseSearchSchema — Python API](https://dev.epicgames.com/documentation/en-us/unreal-engine/python-api/class/PoseSearchSchema?application_version=5.3) | 스키마 프로퍼티 레퍼런스 (MCP 스크립팅용) |

## 연계 문서

- 승인 플랜: `C:\Users\asdasd\.claude\plans\happy-gliding-sky.md` (Step 3 = 스키마 4 + PSN 1 + DB 6)
- 등록 답안지: `C:\Users\asdasd\.claude\tmp\gasp_db_contents.txt` (GASP Dense PSD별 실등록 목록)
- AI 메모리: `reference_posesearch_asset_roles.md` (재조사 방지용 영구 기록)
