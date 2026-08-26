#include "AbilitySystem/Attributes/KDCharacterAttributeSet.h"

UKDCharacterAttributeSet::UKDCharacterAttributeSet()
{
	// 기능 : 공용 어트리뷰트 초기값 — Shield 0 = 적 기본 실드 X
	InitHealth(100.0f);
	InitMaxHealth(100.0f);
	InitPoise(100.0f);
	InitMaxPoise(100.0f);
	InitShield(0.0f);
	InitMaxShield(0.0f);
}

void UKDCharacterAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	// 기능 : 0 ~ Max 클램프
	Super::PreAttributeChange(Attribute, NewValue);

	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	if (Attribute == GetPoiseAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxPoise());
	}
	if (Attribute == GetShieldAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxShield());
	}
}
