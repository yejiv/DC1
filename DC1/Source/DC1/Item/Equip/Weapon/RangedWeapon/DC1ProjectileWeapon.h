// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "GameplayEffectTypes.h"
#include "DC1ProjectileWeapon.generated.h"

class ADC1Projectile;
class UPrimitiveComponent;

UCLASS()
class DC1_API ADC1ProjectileWeapon : public ADC1Weapon
{
	GENERATED_BODY()

public:
	ADC1ProjectileWeapon();

public:
	virtual UPrimitiveComponent* GetAttackCollider() override;

	// 데미지는 쏘는 쪽(GA)이 발사 시점 값으로 굳혀서 넘긴다
	ADC1Projectile*		FireProjectile(const FGameplayEffectSpecHandle& DamageSpec);

	FTransform			GetMuzzleTransform() const;

public:
	// ItemMesh 기준 발사 지점. 손에 붙일 때 쓰는 AttackSocketName과는 별개다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
	FName									MuzzleSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Ranged")
	TSubclassOf<ADC1Projectile>		ProjectileClass;

private:
	FVector				GetAimDirection();
};
