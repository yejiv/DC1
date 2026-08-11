// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "DC1PalAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class DC1_API UDC1PalAttributeSet : public UDC1AttributeSet
{
	GENERATED_BODY()

public:
	UDC1PalAttributeSet();

protected:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
