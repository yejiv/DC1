// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1UIData.generated.h"

class UDC1UserWidget;

UCLASS()
class DC1_API UDC1UIData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	TSubclassOf<UDC1UserWidget> FindUserWidgetClassByTag(const FGameplayTag& UITag) const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
	TMap<FGameplayTag, TSubclassOf<UDC1UserWidget>>  UserWidgetEntries;
};
