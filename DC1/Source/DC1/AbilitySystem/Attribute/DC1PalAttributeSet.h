// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DC1PalAttributeSet.generated.h"

/**
 * Health / MaxHealth / AttackPower 등은 UDC1AttributeSet에 있다. 여기엔 펠 고유 항목만 둔다.
 */
UCLASS()
class DC1_API UDC1PalAttributeSet : public UDC1AttributeSet
{
	GENERATED_BODY()

public:
	UDC1PalAttributeSet();

	// 메타 어트리뷰트. 복제하지 않고 PostGameplayEffectExecute에서 Health로 변환한다.
	UPROPERTY(BlueprintReadOnly, Category = "Pal|Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PalAttributeSet, Damage)

protected:
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
