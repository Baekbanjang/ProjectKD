#include "AbilitySystem/Attributes/AS_CharacterBase.h"

UAS_CharacterBase::UAS_CharacterBase()
{
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitPoise(100.0f);
	InitMaxPoise(100.0f);
}

void UAS_CharacterBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoise());
	}
}
