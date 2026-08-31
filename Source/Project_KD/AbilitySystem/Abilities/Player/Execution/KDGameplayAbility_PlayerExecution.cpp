// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/Execution/KDGameplayAbility_PlayerExecution.h"

#include "AbilitySystemComponent.h"
#include "KDGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "KDPlayerExecutionProfile.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UKDGameplayAbility_PlayerExecution::UKDGameplayAbility_PlayerExecution()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 적이 쏜 Event.Combat.ExecutionStarted 도달 시 발동
	FAbilityTriggerData Trigger;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	Trigger.TriggerTag = GameplayTags::Event_Combat_ExecutionStarted;
	AbilityTriggers.Add(Trigger);
}

void UKDGameplayAbility_PlayerExecution::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo) || !Profile || !Profile->FinisherMontage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// InstancedPerActor 잔류 방지 — 매 활성화 시 명시 리셋.
	ActiveCameraCue = FGameplayTag();

	// 피니셔 몽타주의 Motion Warping 노티 동기점 이름
	static const FName ExecutionWarpName(TEXT("ExecutionTarget"));

	AActor* Player = GetAvatarActorFromActorInfo();
	const AActor* Enemy = TriggerEventData ? TriggerEventData->Instigator : nullptr;

	// 만남점(워프 도착지) = 적 정면 GrappleDistance 지점. 회전은 거기서 적을 본 방향(거리 무관 고정)
	FVector  MeetingLoc = FVector::ZeroVector;
	FRotator MeetingRot = FRotator::ZeroRotator;
	bool bHasMeeting = false;
	
	if (Player && Enemy)
	{
		const FVector Fwd = Enemy->GetActorForwardVector();
		MeetingLoc = Enemy->GetActorLocation() + Fwd * Profile->GrappleDistance;
		MeetingRot = (Enemy->GetActorLocation() - MeetingLoc).Rotation();
		MeetingRot.Pitch = 0.f; MeetingRot.Roll = 0.f; 
		bHasMeeting = true;
		Player->SetActorLocationAndRotation(
			MeetingLoc, MeetingRot, /*bSweep=*/false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->AddLooseGameplayTag(GameplayTags::State_Combat_Invulnerable); // 처형 중 무적
		ASC->AddLooseGameplayTag(GameplayTags::State_Camera_Cinematic);    // 카메라 매니저 손 뗌
	}

	// 적 정면으로 워프 후 피니셔 몽타주의 "ExecutionTarget" 워프 노티 사용
	if (UMotionWarpingComponent* MW = IsValid(Player) ? Player->FindComponentByClass<UMotionWarpingComponent>() : nullptr)
	{
		// 직전 발동 좌표 제거
		MW->RemoveWarpTarget(ExecutionWarpName);

		if (bHasMeeting)
		{
			MW->AddOrUpdateWarpTargetFromLocationAndRotation(ExecutionWarpName, MeetingLoc, MeetingRot);
		}
	}

	// 시네 카메라 켜기(Add/Remove 쌍 — OnCleanup에서 RemoveGameplayCue로 복구). 큐태그 비면 스킵
	if (Profile->CameraCueTag.IsValid())
	{
		ActiveCameraCue = Profile->CameraCueTag;
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
		{
			FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
			Context.AddSourceObject(Profile);
			// 카메라 기준점 = 워프 도착점(적 앞 고정 지점), 발동 거리 무관 동일 구도
			if (bHasMeeting)
			{
				Context.AddOrigin(MeetingLoc);
			}
			ASC->AddGameplayCue(ActiveCameraCue, Context);
		}
	}

	// 피니셔 재생 - 끝/중단/취소 어느 경로든 정리로 수렴.
	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Profile->FinisherMontage);
	Task->OnCompleted.AddDynamic(this, &UKDGameplayAbility_PlayerExecution::OnFinisherDone);
	Task->OnInterrupted.AddDynamic(this, &UKDGameplayAbility_PlayerExecution::OnFinisherDone);
	Task->OnCancelled.AddDynamic(this, &UKDGameplayAbility_PlayerExecution::OnFinisherDone);
	Task->ReadyForActivation();
}

void UKDGameplayAbility_PlayerExecution::OnCleanup(bool bWasCancelled)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
	{
		ASC->RemoveLooseGameplayTag(GameplayTags::State_Combat_Invulnerable);
		ASC->RemoveLooseGameplayTag(GameplayTags::State_Camera_Cinematic);
		if (ActiveCameraCue.IsValid())
		{
			ASC->RemoveGameplayCue(ActiveCameraCue); // 시네 카메라 끔 - 매니저 재장악
		}
	}
	
	Super::OnCleanup(bWasCancelled);
}

void UKDGameplayAbility_PlayerExecution::OnFinisherDone()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
