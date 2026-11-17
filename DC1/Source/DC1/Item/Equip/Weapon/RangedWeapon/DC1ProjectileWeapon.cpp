// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Equip/Weapon/RangedWeapon/DC1ProjectileWeapon.h"

#include "Components/SkeletalMeshComponent.h"
#include "Projectile/DC1Projectile.h"

ADC1ProjectileWeapon::ADC1ProjectileWeapon()
	: Super()
{
	RangedMesh = CreateDefaultSubobject<USkeletalMeshComponent>("RangedMesh");
	RangedMesh->SetupAttachment(ItemMesh);
	RangedMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UPrimitiveComponent* ADC1ProjectileWeapon::GetAttackCollider()
{
	// 원거리는 휘두르는 판정 콜라이더가 없다
	return nullptr;
}

ADC1Projectile* ADC1ProjectileWeapon::FireProjectile(const FGameplayEffectSpecHandle& DamageSpec)
{
	if (false == HasAuthority()) return nullptr;
	if (nullptr == ProjectileClass) return nullptr;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FTransform SpawnTransform(GetAimDirection().Rotation(), GetMuzzleTransform().GetLocation());

	ADC1Projectile* Projectile = GetWorld()->SpawnActor<ADC1Projectile>(ProjectileClass, SpawnTransform, SpawnParams);
	if (nullptr != Projectile)
	{
		Projectile->DamageSpec = DamageSpec;
	}

	return Projectile;
}

FTransform ADC1ProjectileWeapon::GetMuzzleTransform() const
{
	if (false == MuzzleSocketName.IsNone() && RangedMesh->DoesSocketExist(MuzzleSocketName))
	{
		return RangedMesh->GetSocketTransform(MuzzleSocketName);
	}

	return RangedMesh->GetComponentTransform();
}

FVector ADC1ProjectileWeapon::GetAimDirection()
{
	const FTransform MuzzleTransform = GetMuzzleTransform();
	const FVector MuzzleLocation = MuzzleTransform.GetLocation();

	APawn* Pawn = GetInstigator();
	AController* Controller = (nullptr != Pawn) ? Pawn->GetController() : nullptr;
	if (nullptr == Controller)
	{
		return MuzzleTransform.GetRotation().GetForwardVector();
	}

	// 카메라 위치, 방향 갖고오기
	FVector ViewLocation;
	FRotator ViewRotation;
	Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector ViewDir = ViewRotation.Vector();

	// 카메라 방향벡터 위로 Muzzle - 카메라위치를 뺀 벡터를 내적
	// 방향벡터 위에서 Muzzle 위치와 동일 선상에 위치한 지점을 계산 (TraceStart)
	const float MuzzleDepth = FMath::Max(FVector::DotProduct(MuzzleLocation - ViewLocation, ViewDir), 0.f);
	const FVector TraceStart = ViewLocation + ViewDir * MuzzleDepth;
	const FVector TraceEnd = TraceStart + ViewDir * AimTraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(ArrowAimTrace), false);
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(Pawn);

	FHitResult Hit;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params);
	FVector AimPoint = bHit ? Hit.ImpactPoint : TraceEnd;

	const float AimDepth = FVector::DotProduct(AimPoint - ViewLocation, ViewDir);
	if (AimDepth < MuzzleDepth + MinConvergeDistance)
	{
		AimPoint = ViewLocation + ViewDir * (MuzzleDepth + MinConvergeDistance);
	}
	
	return (AimPoint - MuzzleLocation).GetSafeNormal();
}
