// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/DC1AnimInstance.h"
#include "DC1AnimInstance_Pal.generated.h"

UCLASS()
class DC1_API UDC1AnimInstance_Pal : public UDC1AnimInstance
{
	GENERATED_BODY()

public:
	UDC1AnimInstance_Pal(const FObjectInitializer& ObjectInitializer);

public:
	virtual void		NativeInitializeAnimation() override;
	virtual void		NativeUpdateAnimation(float DeltaSeconds) override;

private:
	void				CachePalData();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Locomotion")
	float				NormalizedSpeed = {};

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float				BlendInterpSpeed = { 8.f };

private:
	float				MaxLocomotionSpeed = {};
};
