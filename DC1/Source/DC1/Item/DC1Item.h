// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DC1Item.generated.h"

class UStaticMeshComponent;
class USphereComponent;

UCLASS()
class DC1_API ADC1Item : public AActor
{
	GENERATED_BODY()

public:
	ADC1Item();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent>		InteractCollider = { nullptr };
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent>	ItemMesh = { nullptr };
};