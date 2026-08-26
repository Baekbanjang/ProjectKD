#include "Character/KDBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"

AKDBaseCharacter::AKDBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// 캐릭터끼리 서로의 캡슐 위로 올라타는 step-up 금지 — 근접전에서 캡슐이 겹칠 때
	// 상대 캡슐 반구를 바닥으로 밟고 공중에 뜨거나 밀려나는 현상의 원인.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->CanCharacterStepUpOn = ECB_No;
	}
}

UAbilitySystemComponent* AKDBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
