// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// Lyra-style accessor macro — engine provides individual GAMEPLAYATTRIBUTE_*, this bundles them.
// Shared across all UAS_* headers so the #define lives in one place (avoids macro redefinition
// when a translation unit includes more than one AttributeSet header).
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
