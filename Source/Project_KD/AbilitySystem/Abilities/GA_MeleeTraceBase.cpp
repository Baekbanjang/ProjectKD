#include "AbilitySystem/Abilities/GA_MeleeTraceBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/AS_Combat.h"
#include "KDGameplayTags.h"
#include "AbilitySystem/AnimNotifies/ANS_MeleeTrace.h"
#include "AbilitySystem/Tasks/AT_MeleeTrace.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/MeshComponent.h"
#include "TimerManager.h"

UGA_MeleeTraceBase::UGA_MeleeTraceBase()
{
	// Default Sweep covers the whole blade so point-blank hits land (TipLine whiffs inside the tip arc).
	TraceMode = ETraceMode::Sweep;
}

void UGA_MeleeTraceBase::ActivateAbility(
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

	// Asset tags carry the attack identity (Ability.Player.*) that enemy poise/execution gate on.
	// Empty = silent no-op downstream, so surface the missing setup once here.
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
	MontageTask->OnCompleted.AddDynamic(this, &UGA_MeleeTraceBase::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MeleeTraceBase::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MeleeTraceBase::OnMontageInterrupted);
	MontageTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TraceBeginTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceBegin, nullptr, false, true);
	TraceBeginTask->EventReceived.AddDynamic(this, &UGA_MeleeTraceBase::OnTraceBeginEvent);
	TraceBeginTask->ReadyForActivation();

	UAbilityTask_WaitGameplayEvent* TraceEndTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(
		this, GameplayTags::Event_Montage_TraceEnd, nullptr, false, true);
	TraceEndTask->EventReceived.AddDynamic(this, &UGA_MeleeTraceBase::OnTraceEndEvent);
	TraceEndTask->ReadyForActivation();

	StartSafetyTimer(AttackMontage->GetPlayLength(), EffRate);

	OnActivated();

	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UGA_MeleeTraceBase::OnTraceBeginEvent(FGameplayEventData Payload)
{
	// Race guard: cancel chain may EndAbility before this notify fires
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
	if (const UANS_MeleeTrace* Window = Cast<UANS_MeleeTrace>(Payload.OptionalObject))
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
	
	// Defensive: end any prior trace from a previous notify pair within the same activation.
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
	
	ActiveTraceTask = UAT_MeleeTrace::MeleeTrace(
		this, TraceMesh, EffStartSocket, EffEndSocket, EffMode, EffRadius, bDrawDebug);
	if (!IsValid(ActiveTraceTask))
	{
		return;
	}
	
	ActiveTraceTask->OnHit.AddDynamic(this, &UGA_MeleeTraceBase::OnWeaponHit);
	ActiveTraceTask->ReadyForActivation();
}

void UGA_MeleeTraceBase::OnTraceEndEvent(FGameplayEventData Payload)
{
	ActiveWindow = nullptr;
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
}

void UGA_MeleeTraceBase::OnWeaponHit(const FHitResult& Hit)
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

void UGA_MeleeTraceBase::OnMontageCompleted()
{
	//UE_LOG(LogTemp, Log, TEXT("[KD-Montage] %s: Completed"), *GetName());
	EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), false, false);
}

void UGA_MeleeTraceBase::OnMontageInterrupted()
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

void UGA_MeleeTraceBase::OnActivated()
{
	// No-op base. Overridden by subclasses (e.g. player auto lock-on in GA_PlayerAttackBase).
}

void UGA_MeleeTraceBase::OnCleanup(bool bWasCancelled)
{
	if (ActiveTraceTask)
	{
		ActiveTraceTask->EndTask();
		ActiveTraceTask = nullptr;
	}
	AlreadyHitActors.Reset();
	ActiveWindow = nullptr;
}
