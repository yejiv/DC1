// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "GenericTeamAgentInterface.h"

ADC1PalProjectile::ADC1PalProjectile()
	: Super()
{
	HitCollider->OnComponentHit.RemoveDynamic(this, &ThisClass::OnProjectileHit);
	HitCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnPalProjectileHit);
}

void ADC1PalProjectile::OnPalProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == HasAuthority())
	{
		return;
	}

	// 투사체 팀은 없음. -> 쏜 팰이 같은 팀이면 여기서 걸러진다
	const IGenericTeamAgentInterface* Shooter = Cast<IGenericTeamAgentInterface>(GetInstigator());

	if (nullptr != Shooter && true == ::IsValid(OtherActor)
		&& ETeamAttitude::Hostile == Shooter->GetTeamAttitudeTowards(*OtherActor)
		&& true == DamageSpec.IsValid())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor))
		{
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
		}
	}

	Destroy();
}
