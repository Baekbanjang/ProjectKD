// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Player/GA_PlayerMeleeAttackBase.h"

#include "GameFramework/Character.h"

void UGA_PlayerMeleeAttackBase::ApplyHitStop(AActor* Target, float Duration) const
{
	ACharacter* Char = Cast<ACharacter>(Target);
	if (!Char || Duration <= 0.f) return;
	USkeletalMeshComponent* Mesh = Char->GetMesh();
	UAnimInstance* AnimInst = Mesh ? Mesh->GetAnimInstance() : nullptr;
	UAnimMontage* CurMontage = AnimInst ? AnimInst->GetCurrentActiveMontage() : nullptr;
	if (!AnimInst || !CurMontage) return;

	// SetPlayRate(0) 대신 Pause — 겹친 두 번째 타격이 rate=0을 물어 영구 정지하는 것 방지
	AnimInst->Montage_Pause(CurMontage);

	TWeakObjectPtr<UAnimInstance> WeakAnim(AnimInst);
	TWeakObjectPtr<UAnimMontage> WeakMontage(CurMontage);
	FTimerHandle TH;
	Char->GetWorldTimerManager().SetTimer(TH,
		FTimerDelegate::CreateLambda([WeakAnim, WeakMontage]()
		{
			if (WeakAnim.IsValid() && WeakMontage.IsValid())
			{
				WeakAnim->Montage_Resume(WeakMontage.Get());
			}
		}), Duration, false);
}
