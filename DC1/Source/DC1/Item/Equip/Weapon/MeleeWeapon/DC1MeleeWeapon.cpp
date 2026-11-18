// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Equip/Weapon/MeleeWeapon/DC1MeleeWeapon.h"

#include "Components/CapsuleComponent.h"

ADC1MeleeWeapon::ADC1MeleeWeapon()
	: Super()
{
	AttackCollider = CreateDefaultSubobject<UCapsuleComponent>("AttackCollider");
	AttackCollider->SetupAttachment(ItemMesh);
	AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UPrimitiveComponent* ADC1MeleeWeapon::GetAttackCollider()
{
	return AttackCollider;
}
