// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "DC1Projectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class USoundBase;

UCLASS(Abstract)
class DC1_API ADC1Projectile : public AActor
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
	TObjectPtr<USphereComponent>					HitCollider = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent>				ProjectileMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UProjectileMovementComponent>	ProjectileMovement = { nullptr };

	// 발사 시점에 굳어진 데미지. 쏘는 쪽이 채워준다
	FGameplayEffectSpecHandle						DamageSpec;

protected:
	// 발사음. 서버 스폰 후 각 클라 BeginPlay에서 재생
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							LaunchSound = { nullptr };

	// 명중음. 클라도 자체 이동 시뮬레이션으로 Hit가 와서 권한 검사 전에 재생
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							HitCharacterSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							HitObjectSound = { nullptr };

	// 튕김 등으로 Hit가 여러 번 와도 한 번만
	bool											bHitSoundPlayed = { false };
};
