// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DC1AttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1AttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UDC1AttributeSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Health)

		UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DefensePower, Category = "Attributes")
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, DefensePower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Attributes")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Level)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WorkSpeed, Category = "Attributes")
	FGameplayAttributeData WorkSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, WorkSpeed)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Experience, Category = "Attributes")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Experience)

	// 메타 어트리뷰트
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Damage)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	UFUNCTION()
	void OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower);
	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldLevel);
	UFUNCTION()
	void OnRep_Experience(const FGameplayAttributeData& OldExperience);

	UFUNCTION()
	void OnRep_WorkSpeed(const FGameplayAttributeData& OldWorkSpeed);
};
