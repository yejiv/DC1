// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "DC1PlayerSet.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1PlayerSet : public UDC1CharacterAttributeSet
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

	// 방패 장비가 주는 보호막 (데미지 흡수는 미구현)
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Shield, Category = "Attributes")
	FGameplayAttributeData Shield;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, Shield)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxShield, Category = "Attributes")
	FGameplayAttributeData MaxShield;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, MaxShield)

	// 기절 게이지. 기절 중 시간 / 피격으로 감소, 0이면 사망 ( 모두가 감시하므로 COND_None 복제 )
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DownedGauge, Category = "Attributes")
	FGameplayAttributeData DownedGauge;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, DownedGauge)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxDownedGauge, Category = "Attributes")
	FGameplayAttributeData MaxDownedGauge;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1PlayerSet, MaxDownedGauge)


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

	UFUNCTION()
	void OnRep_Shield(const FGameplayAttributeData& OldShield);

	UFUNCTION()
	void OnRep_MaxShield(const FGameplayAttributeData& OldMaxShield);

	UFUNCTION()
	void OnRep_DownedGauge(const FGameplayAttributeData& OldDownedGauge);

	UFUNCTION()
	void OnRep_MaxDownedGauge(const FGameplayAttributeData& OldMaxDownedGauge);
};
