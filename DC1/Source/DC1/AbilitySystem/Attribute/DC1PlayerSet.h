// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "DC1PlayerSet.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1PlayerSet : public UDC1AttributeSet
{
	GENERATED_BODY()

public:
	UDC1PlayerSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxWeight, Category = "Attributes")
	FGameplayAttributeData MaxWeight;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, MaxWeight)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Satiety, Category = "Attributes")
	FGameplayAttributeData Satiety;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, Satiety)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSatiety, Category = "Attributes")
	FGameplayAttributeData MaxSatiety;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, MaxSatiety)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

	// 무적 상태(회피 중)면 데미지 적용 막기
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

	UFUNCTION()
	void OnRep_MaxWeight(const FGameplayAttributeData& OldMaxWeight);

	UFUNCTION()
	void OnRep_Satiety(const FGameplayAttributeData& OldSatiety);

	UFUNCTION()
	void OnRep_MaxSatiety(const FGameplayAttributeData& OldMaxSatiety);
};
