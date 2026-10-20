// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "DC1CharacterAttributeSet.generated.h"

/**
 * 캐릭터( 플레이어 · 팰 ) 공통 스탯
 */
UCLASS()
class DC1_API UDC1CharacterAttributeSet : public UDC1AttributeSet
{
	GENERATED_BODY()

public:
	UDC1CharacterAttributeSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Stamina, Category = "Attributes")
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, Stamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxStamina, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, MaxStamina)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_AttackPower, Category = "Attributes")
	FGameplayAttributeData AttackPower;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, AttackPower)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Level, Category = "Attributes")
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, Level)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_WorkSpeed, Category = "Attributes")
	FGameplayAttributeData WorkSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, WorkSpeed)

	// 이동 속도 배율 (100 = 100%)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, MoveSpeed)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Experience, Category = "Attributes")
	FGameplayAttributeData Experience;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1CharacterAttributeSet, Experience)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;

protected:
	UFUNCTION()
	void OnRep_Stamina(const FGameplayAttributeData& OldStamina);
	UFUNCTION()
	void OnRep_MaxStamina(const FGameplayAttributeData& OldMaxStamina);
	UFUNCTION()
	void OnRep_AttackPower(const FGameplayAttributeData& OldAttackPower);
	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldLevel);
	UFUNCTION()
	void OnRep_Experience(const FGameplayAttributeData& OldExperience);

	UFUNCTION()
	void OnRep_WorkSpeed(const FGameplayAttributeData& OldWorkSpeed);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldMoveSpeed);
};
