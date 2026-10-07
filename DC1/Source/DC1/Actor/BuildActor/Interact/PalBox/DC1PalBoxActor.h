// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "DC1PalBoxActor.generated.h"

/**
 * 
 */

UCLASS()
class DC1_API ADC1PalBoxActor : public ADC1InteractBuildActor
{
	GENERATED_BODY()

public:
	float							GetBaseRadius() const { return BaseRadius; }

	// 거점 범위 판단
	bool							IsInBase(const FVector& InLocation) const;

protected:
	virtual void					OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;

private:
	void							InteractF(ADC1Character* InInteractOwner);

	// 거점이 이미 있거나 다른 거점 범위와 겹치면 못 짓는다
	virtual bool					CanPlaceAt(const APlayerState* InBuilder) const override;

protected:
	// 거점 범위 반경
	UPROPERTY(EditDefaultsOnly, Category = "PalBox", meta = (ClampMin = "0.0"))
	float							BaseRadius = { 3000.f };
};
