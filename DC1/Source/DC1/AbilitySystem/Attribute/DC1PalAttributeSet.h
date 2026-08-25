// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DC1PalAttributeSet.generated.h"

UCLASS()
class DC1_API UDC1PalAttributeSet : public UDC1AttributeSet
{
	GENERATED_BODY()

public:
	UDC1PalAttributeSet();

protected:
	// 아군이 가한 데미지는 적용 막기
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
};
