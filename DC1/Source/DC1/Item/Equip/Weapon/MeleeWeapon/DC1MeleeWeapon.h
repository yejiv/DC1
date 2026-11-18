// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "DC1MeleeWeapon.generated.h"

class UPrimitiveComponent;
class UCapsuleComponent;

UCLASS()
class DC1_API ADC1MeleeWeapon : public ADC1Weapon
{
	GENERATED_BODY()

public:
	ADC1MeleeWeapon();

public:
	virtual UPrimitiveComponent* GetAttackCollider() override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCapsuleComponent>			AttackCollider = { nullptr };
};