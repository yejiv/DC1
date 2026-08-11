// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DC1AnimInstance.generated.h"

class ADC1Character;
class UCharacterMovementComponent;
/**
 * 
 */
UCLASS()
class DC1_API UDC1AnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UDC1AnimInstance(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ADC1Character>								Owner = { nullptr };
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCharacterMovementComponent>		MovementComponent = { nullptr };

	UPROPERTY(BlueprintReadOnly)
	bool				IsMove = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsFalling = { false };
	UPROPERTY(BlueprintReadOnly)
	float				GroundSpeed = {};
	UPROPERTY(BlueprintReadOnly)
	float				VelocityZ = {};
};
