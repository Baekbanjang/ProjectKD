// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "KDPlayerState.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class UAS_Combat;
class UAS_Player;
class UGameplayAbility;

UCLASS()
class PROJECT_KD_API AKDPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AKDPlayerState();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// Called by APlayerCharacter::PossessedBy after ASC InitAbilityActorInfo.
	void GrantStartupAbilities();

protected:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent; 

	// 플레이어 전용 어트리뷰트 (Stamina, Ammo)
	UPROPERTY()
	TObjectPtr<UAS_Player> PlayerAttributes;

	UPROPERTY()
	TObjectPtr<UAS_Combat> CombatAttributes;

	// 시작 시 적용할 GA
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;

	// 시작 시 1회 적용할 GE
	UPROPERTY(EditDefaultsOnly, Category = "GAS")
	TArray<TSubclassOf<UGameplayEffect>> StartupEffects;
	
	bool bAbilitiesGranted = false;
};
