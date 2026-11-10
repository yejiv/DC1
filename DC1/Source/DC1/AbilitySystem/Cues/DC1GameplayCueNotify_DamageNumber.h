// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "DC1GameplayCueNotify_DamageNumber.generated.h"

/**
 *
 */
UCLASS()
class DC1_API UDC1GameplayCueNotify_DamageNumber : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;
};
