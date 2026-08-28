// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/DC1AnimInstance.h"
#include "DC1AnimInstance_Player.generated.h"

class ADC1Player;

UCLASS()
class DC1_API UDC1AnimInstance_Player : public UDC1AnimInstance
{
	GENERATED_BODY()

public:
	UDC1AnimInstance_Player(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly)
	bool				IsCrouch = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsAiming = { false };
	UPROPERTY(BlueprintReadOnly)
	FVector2D		MoveXY = {};

private:
	void			Compute_Direction();

private:
	UPROPERTY()
	TObjectPtr<ADC1Player>		Player = { nullptr };
};
