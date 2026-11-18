// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DC1PalAttributeSet.generated.h"

UCLASS()
class DC1_API UDC1PalAttributeSet : public UDC1CharacterAttributeSet
{
	GENERATED_BODY()

public:
	UDC1PalAttributeSet();

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Sanity, Category = "Attributes|Pal")
	FGameplayAttributeData Sanity;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PalAttributeSet, Sanity)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxSanity, Category = "Attributes|Pal")
	FGameplayAttributeData MaxSanity;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PalAttributeSet, MaxSanity)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	// 아군이 가한 데미지는 적용 막기
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Sanity(const FGameplayAttributeData& OldSanity);
	UFUNCTION()
	void OnRep_MaxSanity(const FGameplayAttributeData& OldMaxSanity);
};
