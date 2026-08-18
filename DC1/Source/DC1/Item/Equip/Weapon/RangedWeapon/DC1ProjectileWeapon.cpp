// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Equip/Weapon/RangedWeapon/DC1ProjectileWeapon.h"

#include "Components/StaticMeshComponent.h"
#include "Item/Material/Projectile/DC1Projectile.h"

ADC1ProjectileWeapon::ADC1ProjectileWeapon()
	: Super()
{
}

UPrimitiveComponent* ADC1ProjectileWeapon::GetAttackCollider()
{
	// 원거리는 휘두르는 판정 콜라이더가 없다
	return nullptr;
}

FTransform ADC1ProjectileWeapon::GetMuzzleTransform() const
{
	if (false == MuzzleSocketName.IsNone() && ItemMesh->DoesSocketExist(MuzzleSocketName))
	{
		return ItemMesh->GetSocketTransform(MuzzleSocketName);
	}

	return ItemMesh->GetComponentTransform();
}

ADC1Projectile* ADC1ProjectileWeapon::FireProjectile(const FVector& Direction)
{
	if (false == HasAuthority()) return nullptr;
	if (nullptr == ProjectileClass) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform(Direction.Rotation(), GetMuzzleTransform().GetLocation());

	return GetWorld()->SpawnActor<ADC1Projectile>(ProjectileClass, SpawnTransform, SpawnParams);
}
