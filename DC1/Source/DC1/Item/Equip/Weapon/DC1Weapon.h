// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/DC1Equip.h"
#include "GameplayTagContainer.h"
#include "DC1Weapon.generated.h"

class UTexture2D;

UCLASS()
class DC1_API ADC1Weapon : public ADC1Equip
{
	GENERATED_BODY()

public:
	ADC1Weapon();

public:
	virtual UPrimitiveComponent* GetAttackCollider();

	// HUD가 잔탄 카운터와 무한대 표시 중 무엇을 띄울지 정한다. 원거리 여부와는 별개다
	virtual bool UsesAmmo() const { return false; }

public:
	// Weapon State Tag (None, Melee, Bow, Etc)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FGameplayTag		WeaponStateTag;

	// 캐릭터 모델에 붙을 소켓 이름 (weapon_r, weapon_l)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	FName				AttackSocketName;

	// 장착 HUD 표시용. 인벤토리 아이콘(FDC1ItemRow::ItemIcon)과는 다른 에셋이다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSoftObjectPtr<UTexture2D>	EquipIcon;
};