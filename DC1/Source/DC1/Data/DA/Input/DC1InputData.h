// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1InputData.generated.h"

class UInputAction;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct FDC1InputMappingContext
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag IMCTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 Priority = 0;
};

USTRUCT(BlueprintType)
struct FDC1InputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag IATag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UInputAction> InputAction = nullptr;
};

UCLASS()
class DC1_API UDC1InputData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	const UInputMappingContext*		FindInputMappingContextbyTag(const FGameplayTag& Tag) const;
	const FDC1InputMappingContext*  FindIMCEntrybyTag(const FGameplayTag& Tag) const;
	const UInputAction*				FindInputActionbyTag(const FGameplayTag& Tag) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IMC")
	TArray<FDC1InputMappingContext>		InputMappingContexts;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IA")
	TArray<FDC1InputAction>				InputActions;
};
