# [길동 보류] 처형 정렬/동기 — Contextual Animation Plugin 도입 인수인계

> 2026-06-15 승인·스니펫 전달 완료 → **승환 적용 직전에 버터 전환으로 중단** (2026-07-09 플랜 폴더에서 이관).
> 길동 컨텐츠 재개 시 여기서 그대로 이어하면 됨. 재개 시점의 엔진 버전에서 플러그인 API 재확인 필수 (Experimental).

## 왜 하려던 것

처형 발동 거리/각도마다 플레이어-적 위치가 어긋나고 액터가 겹침 = "두 캐릭터 상대 정렬" 문제. 수동 땜질(만남점 계산+워프+회전 스냅+캡슐 끄기+핸드셰이크) 대신 UE5.6 내장 **Contextual Animation Plugin**(Experimental)이 정렬·타이밍 동기·충돌 무시를 데이터(SceneAsset)로 제공. 카메라는 플러그인 영역 밖 → 기존 Level Sequence(GCN) 그대로 위에 얹힘.

## 선행 완료분 (처형 시네마틱 카메라 — 빌드 통과 ✅ 2026-06-13)

| 파일 | 내용 |
|---|---|
| `Library/KDCinematicLibrary.{h,cpp}` 신규 | `PlaySequenceAtActor(WorldContext, ULevelSequence*, AActor* Origin, bAutoPlay)` — 시퀀스를 지정 액터 기준(Transform Origin)으로 재생하는 **범용** 헬퍼 |
| `PlayerExecutionProfile.h` | `TObjectPtr<ULevelSequence> FinisherSequence` 추가 |
| `GA_PlayerExecution.cpp` | 큐 발사: `MakeEffectContext`→`AddSourceObject(Profile)`→`AddGameplayCue(Tag, Context)` |
| `GCN_ExecutionCamera.{h,cpp}` | OnActive=SourceObject→Profile→PlaySequenceAtActor / OnRemove=Stop+Destroy |
| `Project_KD.Build.cs` | `LevelSequence`, `MovieScene` 추가 |

신호 흐름: F → GA(지휘) → 큐(Profile 동봉) → GCN(중개) → 헬퍼(시퀀스 재생). 카메라 양보 통로 = `State.Camera.Cinematic` 태그 → `KDPlayerCameraManager::UpdateViewTarget` early-return.

## 미적용 스니펫 6건 (여기서 재개) ★

1. **`Project_KD.Build.cs`** — `PublicDependencyModuleNames`에 `"ContextualAnimation"` 추가
2. **`KDGameplayTags.{h,cpp}`** — `Event_Combat_ExecutionEnded`("Event.Combat.ExecutionEnded") 신규
3. **`ExecutionComponent.h`** — `class UContextualAnimSceneActorComponent;` 전방선언 + `UFUNCTION void OnSceneLeft(UContextualAnimSceneActorComponent*)`
4. **`ExecutionComponent.cpp`** — HandleExecution: 캡슐 3줄 제거 + 씬모드 분기:
   - CASC.SceneAsset 있으면 → `BP_CreateContextualAnimSceneBindingsForTwoActors`(적=Primary, 플레이어=Secondary) → `OnLeftSceneDelegate.AddDynamic` → `StartContextualAnimScene`
   - 폴백(SceneAsset 없음) = 기존 캡슐 끄기 + `OnExecutionBegin` 몽타주
   - 핸드셰이크 `Data.EventMagnitude = bSceneStarted ? 1 : 0`
   - 신규 `OnSceneLeft`: 언바인드 → `FinishExecution` → `ExecutionEnded` 발사
   - **적 회전 스냅(플레이어 향함)은 씬 시작 전 유지** (rigid pivot 확보)
5. **`GA_PlayerExecution.h`** — `UFUNCTION void OnExecutionEnded(FGameplayEventData)` + `TWeakObjectPtr<UAbilitySystemComponent> CameraCueASC`
6. **`GA_PlayerExecution.cpp`** — `bSceneMode = TriggerEventData->EventMagnitude > 0` 분기:
   - 씬모드 = 태그 부여 + 카메라 큐를 **적 ASC**에 발사(GCN이 적 트랜스폼 앵커) + `WaitGameplayEvent(ExecutionEnded)` → EndAbility
   - 몽타주 직접재생 X(씬이 재생) / 워프 X(씬이 정렬) / MeetingLoc 계산 불필요
   - 폴백 = 기존 워프/몽타주 경로. `OnCleanup` = `CameraCueASC`(없으면 플레이어 ASC)로 `RemoveGameplayCue`

**카메라 앵커 = 적 트랜스폼인 이유**: 플러그인이 GA 발동 *전에* 플레이어 워프를 시작 → 플레이어 기준은 회전 충돌. 적은 회전 스냅 후 정지 = 거리/각도 무관 동일 기준점. GCN/Library **무수정** (origin 없을 때 `Target->GetActorTransform()` 폴백 활용).

## 검증된 플러그인 API (2026-06-15 엔진 헤더 기준)

- `UContextualAnimSceneActorComponent` — 양쪽 액터 부착, `SceneAsset` UPROPERTY 직접 보유
- `UContextualAnimUtilities::BP_CreateContextualAnimSceneBindingsForTwoActors(SceneAsset, Primary, Secondary, OutBindings) → bool`
- `FContextualAnimSceneBindingContext(AActor*)` / `CASC->StartContextualAnimScene(Bindings) → bool`
- `CASC->OnLeftSceneDelegate` (OneParam, 몽타주 블렌드아웃 시 발화) / `CASC->EarlyOutContextualAnimScene(bool)`
- 충돌: 플러그인이 참여자끼리 자동 on/off → 기존 캡슐 코드 제거 대상
- ⚠️ **Experimental** (VersionName 0.1, 공식문서 없음) — 영향범위는 ExecutionComponent+GA 두 곳에 격리, SceneAsset 비우면 레거시 폴백

## BP/에셋 작업 (코드 적용 후)

1. 플러그인 켜기: `Motion Warping` + `Contextual Animation` → 에디터 재시작
2. 플레이어+적 BP 양쪽에 `ContextualAnimSceneActorComponent` 추가, 적 컴포넌트 `SceneAsset` 지정
3. Roles 에셋: `Attacker`/`Victim` 2개, Primary Role = `Victim`(적, 정지 앵커)
4. ContextualAnimSceneAsset: 역할별 AnimSet(피니셔/피격 몽타주), **둘 다 루트모션 ON**, 플레이어 몽타주에 Motion Warping 노티, disable_collision_between_actors 체크
5. 카메라: 기존 `LS_Execution_01` 그대로 (적 트랜스폼 앵커로 동작)

## 검증 (PIE)

1. 발동 거리/각도 다양(정면/측면/후방·가까이/멀리) → 전부 동일 구도 + 액터 안 겹침
2. 게임플레이↔시네 양방향 부드러운 블렌드
3. SceneAsset 비우면 레거시 폴백 / 시퀀스 비면 시네마틱 스킵
4. 데스블로(즉사) 카메라 복귀 타이밍 정합 + 생존 처형 정상
5. 벽 근처/좁은 공간 카메라 클리핑
6. 빌드 0 에러/0 경고 → dev-log 작성

## 나중 확장 (미룸): 처형 모션 랜덤 세트

- 세트 = {플레이어 몽타주 + 적 몽타주 + 시퀀스}. 모션 고르는 지점 = 플레이어(GA의 `Profile->FinisherMontage`) + 적(`GetExecutionMontage()`) 각 1곳 → 배열+랜덤 인덱스로 교체
- ⚠️ 듀엣이라 양쪽이 같은 인덱스를 골라야 함 → 핸드셰이크(`Event.Combat.ExecutionStarted`)에 세트 번호 실어 동기
- 미정: 세트 권한자 = (A)적 Profile / (B)플레이어 GA 목록 / (C)별도 페어링 DataAsset
