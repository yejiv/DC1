// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile/DC1Projectile.h"
#include "DC1PalProjectile.generated.h"

/**
 *
 *
 */
UCLASS()
class DC1_API ADC1PalProjectile : public ADC1Projectile
{
	GENERATED_BODY()

public:
	ADC1PalProjectile();

protected:
	UFUNCTION()
	void					OnPalProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
