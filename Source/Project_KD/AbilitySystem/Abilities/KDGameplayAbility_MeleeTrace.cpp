#include "AbilitySystem/Abilities/KDGameplayAbility_MeleeTrace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/KDCombatAttributeSet.h"
#include "AbilitySystem/Library/KDAbilityStatics.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/AnimNotifies/KDAnimNotifyState_MeleeTrace.h"
#include "AbilitySystem/Tasks/KDAbilityTask_MeleeTrace.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/MeshComponent.h"
#include "TimerManager.h"

UKDGameplayAbility_MeleeTrace::UKDGameplayAbility_MeleeTrace()
{
	// 기능 : 판정 기본값
	// 기본 판정 = Sweep. 날 전체 / TipLine 은 팁 선분만
	TraceMode = ETraceMode::Sweep;
}

void UKDGameplayAbility_MeleeTrace::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	// 기능 : 몽타주 재생 + 판정 노티 대기 등록
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!IsValid(AttackMontage))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	AlreadyHitActors.Reset();

	const float EffRate = GetEffectiveMontagePlayRate();

	// AssetTags = 공격 신원(Ability.Player.*). 적 포이즈·처형이 이걸로 분기
	ensureMsgf(!GetAssetTags().IsEmpty(),
		TEXT("[KD] %s has empty AssetTags enemy poise/execution won't trigger. Set Ability.Player.* on the GA."),
		*GetName());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage, EffRate, NAME_None, true, 1.0f);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	MontageTask->OnCompleted.AddDynamic(this, &UKDGameplayAbility_MeleeTrace::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UKDGameplayAbility_MeleeTrace::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UKDGameplayAbility_MeleeTrace::OnMontageInterrupted);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TraceBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceBegin, nullptr, false, true);
	TraceBeginTask->EventReceived.AddDynamic(this, &UKDGameplayAbility_MeleeTrace::OnTraceBeginEvent);
	TraceBeginTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TraceEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceEnd, nullptr, false, true);
	TraceEndTask->EventReceived.AddDynamic(this, &UKDGameplayAbility_MeleeTrace::OnTraceEndEvent);
	TraceEndTask->ReadyForActivation();

	StartSafetyTimer(AttackMontage->GetPlayLength(), EffRate);

	OnActivated();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UKDGameplayAbility_MeleeTrace::OnTraceBeginEvent(FGameplayEventData Payload)
{
	// 기능 : 판정 시작 — 트레이스 메쉬·소켓 확정 후 AT_MeleeTrace 생성

	// 캔슬 체인이 먼저 EndAbility 한 경우 제외
	if (!IsActive()) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	
	if (!IsValid(Avatar)) return;
	
	// 판정 구간마다 "액터당 1히트" 초기화 — 한 몽타주의 2연타(속4 바디->헤드)가 같은 적에게 각각 들어가게
	AlreadyHitActors.Reset();
	
	// 이번 창의 유효 설정 — 노티 오버라이드가 GA 기본값을 이김
	FName EffStartSocket = StartSocket;
	FName EffEndSocket = EndSocket;
	ETraceMode EffMode = TraceMode;
	float EffRadius = CapsuleRadius;
	ETraceMeshSource EffSource = MeshSource;
	if (const UKDAnimNotifyState_MeleeTrace* Window = Cast<UKDAnimNotifyState_MeleeTrace>(Payload.OptionalObject))
	{
		ActiveWindow = Window;
		if (Window->StartSocketOverride != NAME_None) EffStartSocket = Window->StartSocketOverride;
		if (Window->EndSocketOverride != NAME_None) EffEndSocket = Window->EndSocketOverride;
		if (Window->CapsuleRadiusOverride > 0.f) EffRadius = Window->CapsuleRadiusOverride;
		if (Window->bOverrideTraceMode) EffMode = Window->TraceModeOverride;
		if (Window->bOverrideMeshSource) EffSource = Window->MeshSourceOverride;
	}
	
	// 출처별 트레이스 메쉬 해석
	UMeshComponent* TraceMesh = nullptr;
	if (EffSource == ETraceMeshSource::OwnerBody)
	{
		// 맨손/킥 — 아바타 본체 메쉬의 본(hand_l, foot_r 등)으로 트레이스
		TraceMesh = GetCurrentActorInfo()->SkeletalMeshComponent.Get();

		// 본체엔 무기 소켓(Spear_*) 없음 — 노티 hand_l/foot_r 오버라이드 빠지면 몸통 원점서 잘못된 트레이스
		if (IsValid(TraceMesh) && !TraceMesh->DoesSocketExist(EffStartSocket))
		{
			/*UE_LOG(LogTemp, Warning,
				TEXT("[KD] OwnerBody 트레이스인데 '%s' 소켓/본 없음 — 노티 소켓 오버라이드 누락"),
				*EffStartSocket.ToString());*/
			return;
		}
	}
	else
	{
		// 무기 메쉬 — 태그가 일치하는 메쉬 컴포넌트 검색(스태틱/스켈레탈 공용)
		TArray<UActorComponent*> Components;
		Avatar->GetComponents(UMeshComponent::StaticClass(), Components);
		for (UActorComponent* Comp : Components)
		{
			if (Comp->ComponentHasTag(WeaponMeshComponentTag))
			{
				TraceMesh = Cast<UMeshComponent>(Comp);
				break;
			}
		}
	}
	
	if (!IsValid(TraceMesh))
	{
		/*UE_LOG(LogTemp, Warning, TEXT("[KD] Trace mesh not found (source=%d) on %s"),
			(int32)EffSource, *Avatar->GetName());*/
		return;
	}

	// 무기 소켓 존재 체크
	if (!TraceMesh->DoesSocketExist(EffStartSocket) || !TraceMesh->DoesSocketExist(EffEndSocket))
	{
		/*UE_LOG(LogTemp, Warning, TEXT("[KD] 트레이스 소켓 없음 — '%s'/'%s' (mesh=%s)"),
			*EffStartSocket.ToString(), *EffEndSocket.ToString(), *TraceMesh->GetName());*/
		return;
	}
	
	// 같은 활성화 안 이전 판정 잔여 정리
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
	
	ActiveTraceTask = UKDAbilityTask_MeleeTrace::MeleeTrace(
		this, TraceMesh, EffStartSocket, EffEndSocket, EffMode, EffRadius, bDrawDebug,
		ArcBulge, TraceSegments, MaxSubSteps);
	if (!IsValid(ActiveTraceTask))
	{
		return;
	}
	
	ActiveTraceTask->OnHit.AddDynamic(this, &UKDGameplayAbility_MeleeTrace::OnWeaponHit);
	ActiveTraceTask->ReadyForActivation();
}

void UKDGameplayAbility_MeleeTrace::OnTraceEndEvent(FGameplayEventData Payload)
{
	// 기능 : 판정 종료 — 트레이스 태스크 정리
	ActiveWindow = nullptr;
	if (ActiveTraceTask)
	{
		// 마지막 구간 판정
		ActiveTraceTask->TraceOnce();
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
}

void UKDGameplayAbility_MeleeTrace::OnWeaponHit(const FHitResult& Hit)
{
	// 기능 : 명중 처리 — 데미지 GE 적용 + 히트 이벤트 발신
	AActor* HitActor = Hit.GetActor();
	if (!IsValid(HitActor)) return;

	if (bOncePerActor)
	{
		if (AlreadyHitActors.Contains(HitActor)) return;
		AlreadyHitActors.Add(HitActor);
	}

	UAbilitySystemComponent* AttackerASC = GetAbilitySystemComponentFromActorInfo();
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
	if (!AttackerASC || !TargetASC) return;

	// 팀 게이트 — 적 무기가 적을 때리는 경우 제외. 플레이어 ASC 는 Team.Enemy 없음
	if (UKDAbilityStatics::IsFriendlyFire(AttackerASC, TargetASC))
	{
		return;
	}

	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[KD] DamageEffectClass not set on %s"), *GetName());
		return;
	}

	const float AttackPower = AttackerASC->GetNumericAttribute(UKDCombatAttributeSet::GetAttackPowerAttribute()) * DamageMultiplier;
	const FGameplayEffectContextHandle Context = UKDAbilityStatics::ApplyDamageEffect(
		AttackerASC, TargetASC, DamageEffectClass, AttackPower, PoiseMultiplier, Hit,
		GetAvatarActorFromActorInfo(), this);

	// 히트 알림 — 반응은 맞은 쪽이 선택
	UKDAbilityStatics::SendHitEvent(HitActor, GetAvatarActorFromActorInfo(), GetAssetTags(), Context, KnockbackMultiplier);

	// 데미지 후처리 훅 — 플레이어만 HitConfirm 큐
	OnTargetHit(HitActor, TargetASC, Hit);
}

void UKDGameplayAbility_MeleeTrace::OnMontageCompleted()
{
	// 기능 : 몽타주 정상 종료
	//UE_LOG(LogTemp, Log, TEXT("[KD-Montage] %s: Completed"), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void UKDGameplayAbility_MeleeTrace::OnMontageInterrupted()
{
	// 기능 : 몽타주 중단 — 현재 재생 몽타주 로그 후 종료
	FString NowPlaying = TEXT("<none>");
	if (const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo())
	{
		if (UAnimInstance* Anim = Info->GetAnimInstance())
		{
			if (UAnimMontage* Cur = Anim->GetCurrentActiveMontage())
			{
				NowPlaying = Cur->GetName();
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[KD-Montage] %s: INTERRUPTED nowPlaying=%s (attackMontage=%s)"),
		*GetName(), *NowPlaying, AttackMontage ? *AttackMontage->GetName() : TEXT("null"));
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, true);
}

void UKDGameplayAbility_MeleeTrace::OnActivated()
{
	// 기능 : 자식용 훅. 베이스는 비어 있음
}

void UKDGameplayAbility_MeleeTrace::OnCleanup(bool bWasCancelled)
{
	// 기능 : 종료 정리 — 트레이스 태스크 · 히트 목록 해제
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
	AlreadyHitActors.Reset();
	ActiveWindow = nullptr;
}
