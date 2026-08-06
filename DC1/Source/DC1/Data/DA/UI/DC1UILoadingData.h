// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "DC1UILoadingData.generated.h"


UCLASS()
class DC1_API UDC1UILoadingData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UMaterialInterface* GetRandomSpinnerMaterial() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading")
	TArray<TObjectPtr<UMaterialInterface>>	SpinnerMaterials;
};
