// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1AbilityData.generated.h"

class UDC1AbilitySystemComponent;
class UAttributeSet;
class UGameplayAbility;
class UGameplayEffect;
struct FDC1AbilitySetGrantedHandles;

USTRUCT(BlueprintType)
struct FDC1AbilityEntry
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Ability"))
	FGameplayTag		ActivationTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32					AbilityLevel = { 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility>	AbilityClass = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool					IsPassive = { false };
};

UCLASS()
class DC1_API UDC1AbilityData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	void GiveToAbilitySystem(UDC1AbilitySystemComponent* ASC,
									FDC1AbilitySetGrantedHandles& OutHandles,
									UObject* SourceObject = nullptr) const;

	TSubclassOf<UGameplayEffect> FindEffectByTag(const FGameplayTag& EffectTag) const;

public:
	// 공용 AbilityData 설정 (Ex. Monster 공용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Composition")
	TArray<TObjectPtr<UDC1AbilityData>> BaseAbilityData;

	// Abilities
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<FDC1AbilityEntry> Abilities;

	// Effects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effect")
	TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> Effects;

	// AttributeSets
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attribute")
	TArray<TSubclassOf<UAttributeSet>> AttributeSets;
};
