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
	virtual void			BeginPlay() override;

	UFUNCTION()
	void					OnPalProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	// 비행음. 각 클라 BeginPlay 에서 투사체에 붙여 재생, 투사체가 사라지면 같이 꺼진다 ( 웨이브 bLooping 필요 )
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>	LoopSound = { nullptr };
};
