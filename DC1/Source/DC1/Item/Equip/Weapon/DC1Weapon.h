// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/DC1Equip.h"
#include "GameplayTagContainer.h"
#include "DC1Weapon.generated.h"

UCLASS()
class DC1_API ADC1Weapon : public ADC1Equip
{
	GENERATED_BODY()

public:
	ADC1Weapon();

public:
	virtual UPrimitiveComponent* GetAttackCollider();

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FGameplayTag		WeaponStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName				AttackSocketName;
};