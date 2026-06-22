#include "AbilitySystem/Abilities/Enemy/GA_EnemyHitReact.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Enemy/EnemyDefinitionDataAsset.h"
#include "Enemy/KDEnemyBaseCharacter.h"
#include "KDGameplayTags.h"

UGA_EnemyHitReact::UGA_EnemyHitReact()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	// 연타 피격 시 움찔 갱신(재발동) — 진행 중 움찔을 끊고 새 방향으로 다시 재생.
	bRetriggerInstancedAbility = true;

	FAbilityTriggerData Trigger;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	Trigger.TriggerTag = GameplayTags::Event_Combat_HitReact;
	AbilityTriggers.Add(Trigger);

	ActivationBlockedTags.AddTag(GameplayTags::State_Combat_SuperArmor);
	ActivationBlockedTags.AddTag(GameplayTags::State_Combat_Staggered);
	ActivationBlockedTags.AddTag(GameplayTags::State_Dead);
}

void UGA_EnemyHitReact::ActivateAbility(
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

	const AActor* Avatar = GetAvatarActorFromActorInfo();
	const AKDEnemyBaseCharacter* Enemy = Cast<AKDEnemyBaseCharacter>(Avatar);
	UAnimMontage* Montage = (Enemy && Enemy->GetEnemyDefinition())
		? Enemy->GetEnemyDefinition()->HitReactMontage : nullptr;
	if (!Montage)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 피격 방향 4분기 — AS_Combat의 HitAngle 식과 동일(공격자 위치 기준 부호있는 좌우 각도).
	// |각도| ≤ 45 = 정면, ≥ 135 = 후면, 그 사이 = 부호로 좌/우.
	FName Section(TEXT("HitF"));
	const AActor* Attacker = TriggerEventData ? TriggerEventData->Instigator.Get() : nullptr;
	if (Avatar && Attacker)
	{
		const FVector ToAttacker = (Attacker->GetActorLocation() - Avatar->GetActorLocation()).GetSafeNormal2D();
		const float Angle = FMath::FindDeltaAngleDegrees(
			Avatar->GetActorForwardVector().GetSafeNormal2D().Rotation().Yaw, ToAttacker.Rotation().Yaw);
		const float AbsAngle = FMath::Abs(Angle);
		if (AbsAngle >= 135.f)    { Section = TEXT("HitB"); }
		else if (AbsAngle > 45.f) { Section = (Angle > 0.f) ? TEXT("HitR") : TEXT("HitL"); }
	}
	// 섹션 없는 몽타주(단일 움찔)면 처음부터 재생.
	if (!Montage->IsValidSectionName(Section)) { Section = NAME_None; }

	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this, NAME_None, Montage, 1.f, Section);
	if (!Task)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	Task->OnCompleted.AddDynamic(this, &UGA_EnemyHitReact::OnMontageFinished);
	Task->OnInterrupted.AddDynamic(this, &UGA_EnemyHitReact::OnMontageFinished);
	Task->OnCancelled.AddDynamic(this, &UGA_EnemyHitReact::OnMontageFinished);
	Task->ReadyForActivation();

	// 방향 섹션만 단발 재생 — 에디터에서 섹션들이 순차 체인(HitF→HitB→…)돼 있어도 점프 후 끝까지 줄줄이
	// 재생된다. ReadyForActivation이 동기로 몽타주를 시작하므로 그 직후 점프한 섹션의 다음을 None으로 끊어
	// 한 섹션만 재생하고 멈추게 한다(몽타주별 Next Section 수동 정리 불필요).
	if (Section != NAME_None)
	{
		if (UAnimInstance* AnimInst = ActorInfo->GetAnimInstance())
		{
			AnimInst->Montage_SetNextSection(Section, NAME_None, Montage);
		}
	}

	// 베이스 안전망 — 몽타주 콜백 유실 시에도 EndAbility 보장.
	StartSafetyTimer(Montage->GetPlayLength(), 1.f);
}

void UGA_EnemyHitReact::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}
