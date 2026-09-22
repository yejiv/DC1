// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DC1AppearancePreviewActor.generated.h"

UCLASS()
class DC1_API ADC1AppearancePreviewActor : public AActor
{
	GENERATED_BODY()

public:
	ADC1AppearancePreviewActor();

protected:
	virtual void BeginPlay() override;
};
