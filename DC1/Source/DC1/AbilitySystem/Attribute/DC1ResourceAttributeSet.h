// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "DC1ResourceAttributeSet.generated.h"

/* 
* Resourc Actor 전용
*/
UCLASS()
class DC1_API UDC1ResourceAttributeSet : public UDC1AttributeSet
{
	GENERATED_BODY()

protected:
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
};
