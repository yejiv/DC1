// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/DC1AnimInstance.h"
#include "DC1AnimInstance_Player.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1AnimInstance_Player : public UDC1AnimInstance
{
	GENERATED_BODY()

public:
	UDC1AnimInstance_Player(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
