// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

// 어트리뷰트 접근자 묶음 매크로 — 엔진은 GAMEPLAYATTRIBUTE_ 개별 매크로만 제공
// 모든 UAS_ 헤더가 공유 — 헤더 2개를 같이 include 할 때의 매크로 재정의 X
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)
