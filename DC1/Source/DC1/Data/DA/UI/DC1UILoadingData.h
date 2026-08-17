// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "DC1UILoadingData.generated.h"

USTRUCT(BlueprintType)
struct FUILoadingEntry
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly)
	TArray<TObjectPtr<UTexture2D>> Frames;

	UPROPERTY(EditDefaultsOnly)
	float FPS = 10.f;
};

UCLASS()
class DC1_API UDC1UILoadingData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	const FUILoadingEntry* GetRandomSprites() const;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loading")
	TArray<FUILoadingEntry>	LoadingEntries;
};
