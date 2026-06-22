#include "AbilitySystem/Abilities/GA_WeaponTraceBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AS_Combat.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/AnimNotifies/ANS_WeaponTrace.h"
#include "AbilitySystem/Tasks/AT_WeaponTrace.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

UGA_WeaponTraceBase::UGA_WeaponTraceBase()
{
	// Default Sweep covers the whole blade so point-blank hits land (TipLine whiffs inside the tip arc).
	TraceMode = ETraceMode::Sweep;
}

void UGA_WeaponTraceBase::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
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

	// Asset tags carry the attack identity (Ability.Mugong.*) that enemy poise/execution gate on.
	// Empty = silent no-op downstream, so surface the missing setup once here.
	ensureMsgf(!GetAssetTags().IsEmpty(),
		TEXT("[KD] %s has empty AssetTags enemy poise/execution won't trigger. Set Ability.Mugong.* on the GA."),
		*GetName());

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, AttackMontage, EffRate, NAME_None, true, 1.0f);
	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	MontageTask->OnCompleted.AddDynamic(this, &UGA_WeaponTraceBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_WeaponTraceBase::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_WeaponTraceBase::OnMontageInterrupted);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TraceBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceBegin, nullptr, false, true);
	TraceBeginTask->EventReceived.AddDynamic(this, &UGA_WeaponTraceBase::OnTraceBeginEvent);
	TraceBeginTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TraceEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceEnd, nullptr, false, true);
	TraceEndTask->EventReceived.AddDynamic(this, &UGA_WeaponTraceBase::OnTraceEndEvent);
	TraceEndTask->ReadyForActivation();

	StartSafetyTimer(AttackMontage->GetPlayLength(), EffRate);

	OnActivated();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_WeaponTraceBase::OnTraceBeginEvent(FGameplayEventData Payload)
{
	// Race guard: cancel chain may EndAbility before this notify fires — skip to avoid a stray 1-frame trace.
	if (!IsActive()) return;

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsValid(Avatar)) return;

	USkeletalMeshComponent* WeaponMesh = nullptr;
	TArray<UActorComponent*> Components;
	Avatar->GetComponents(USkeletalMeshComponent::StaticClass(), Components);
	for (UActorComponent* Comp : Components)
	{
		if (Comp->ComponentHasTag(WeaponMeshComponentTag))
		{
			WeaponMesh = Cast<USkeletalMeshComponent>(Comp);
			break;
		}
	}

	if (!IsValid(WeaponMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("[KD] Weapon mesh with tag '%s' not found on %s"),
			*WeaponMeshComponentTag.ToString(), *Avatar->GetName());
		return;
	}

	// Defensive: end any prior trace from a previous notify pair within the same activation.
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}

	// Per-window notify overrides win over GA defaults (empty/zero = inherit) → one montage traces
	// different shapes per swing.
	FName EffStartSocket = StartSocket;
	FName EffEndSocket = EndSocket;
	ETraceMode EffMode = TraceMode;
	float EffRadius = CapsuleRadius;
	if (const UANS_WeaponTrace* Window = Cast<UANS_WeaponTrace>(Payload.OptionalObject))
	{
		if (Window->StartSocketOverride != NAME_None) EffStartSocket = Window->StartSocketOverride;
		if (Window->EndSocketOverride != NAME_None) EffEndSocket = Window->EndSocketOverride;
		if (Window->CapsuleRadiusOverride > 0.f) EffRadius = Window->CapsuleRadiusOverride;
		if (Window->bOverrideTraceMode) EffMode = Window->TraceModeOverride;
	}

	ActiveTraceTask = UAT_WeaponTrace::WeaponTrace(
		this, WeaponMesh, EffStartSocket, EffEndSocket, EffMode, EffRadius, bDrawDebug);
	if (!IsValid(ActiveTraceTask))
	{
		return;
	}
	ActiveTraceTask->OnHit.AddDynamic(this, &UGA_WeaponTraceBase::OnWeaponHit);
	ActiveTraceTask->ReadyForActivation();
}

void UGA_WeaponTraceBase::OnTraceEndEvent(FGameplayEventData Payload)
{
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
}

void UGA_WeaponTraceBase::OnWeaponHit(const FHitResult& Hit)
{
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

	// Faction gate: an enemy weapon never hits another enemy (player ASC carries no Team.Enemy, so it passes).
	if (AttackerASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy)
		&& TargetASC->HasMatchingGameplayTag(GameplayTags::Team_Enemy))
	{
		return;
	}

	if (!DamageEffectClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[KD] DamageEffectClass not set on %s"), *GetName());
		return;
	}

	const float AttackPower = AttackerASC->GetNumericAttribute(UAS_Combat::GetAttackPowerAttribute());
	FGameplayEffectContextHandle Context = AttackerASC->MakeEffectContext();
	Context.AddSourceObject(GetAvatarActorFromActorInfo());
	Context.AddHitResult(Hit);

	FGameplayEffectSpecHandle SpecHandle = AttackerASC->MakeOutgoingSpec(DamageEffectClass, 1.f, Context);
	if (SpecHandle.IsValid())
	{
		// Positive = damage into the IncomingDamage gateway (AS_Combat consumes it).
		UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
			SpecHandle, GameplayTags::SetByCaller_AttackPower, AttackPower);
		AttackerASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, TargetASC);
	}

	// Generic hit event carrying this ability's identity tags — decoupled, the victim decides the reaction.
	// ContextHandle carries the impact for directional knockback / hit-react.
	FGameplayEventData HitEvent;
	HitEvent.Instigator = GetAvatarActorFromActorInfo();
	HitEvent.Target = HitActor;
	HitEvent.InstigatorTags = GetAssetTags();
	HitEvent.ContextHandle = Context;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor, GameplayTags::Event_Combat_Hit, HitEvent);

	// Post-damage hook (player places HitConfirm cue, enemy ignores).
	OnTargetHit(HitActor, TargetASC, Hit);
}

void UGA_WeaponTraceBase::OnMontageCompleted()
{
	UE_LOG(LogTemp, Log, TEXT("[KD-Montage] %s: Completed"), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void UGA_WeaponTraceBase::OnMontageInterrupted()
{
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

void UGA_WeaponTraceBase::OnActivated()
{
	// No-op base. Overridden by subclasses (e.g. player auto lock-on in GA_PlayerAttackBase).
}

void UGA_WeaponTraceBase::OnCleanup(bool bWasCancelled)
{
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
	AlreadyHitActors.Reset();
}
