// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DC1PalAttributeSet.generated.h"

/**
 *
 */
UCLASS()
class DC1_API UDC1PalAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UDC1PalAttributeSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Pal|Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PalAttributeSet, Health)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Pal|Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PalAttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, Category = "Pal|Attributes|Meta") //PostGameplayEffectExecute에서 Health로 변환
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PalAttributeSet, Damage)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
};
