// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Material/DC1MaterialItem.h"
#include "DC1Projectile.generated.h"

class UProjectileMovementComponent;

UCLASS()
class DC1_API ADC1Projectile : public ADC1MaterialItem
{
	GENERATED_BODY()

public:
	ADC1Projectile();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent>	ProjectileMovement = { nullptr };
};
