// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalProjectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Define/DC1Enums.h"

ADC1PalProjectile::ADC1PalProjectile()
	: Super()
{
	HitCollider->OnComponentHit.RemoveDynamic(this, &ThisClass::OnProjectileHit);
	HitCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnPalProjectileHit);

	// Projectile 프리셋 변경 -> Player Ignore, WorldStatic Block 부분
	HitCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitCollider->SetCollisionObjectType(ECC_WorldDynamic);
	HitCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitCollider->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Player), ECR_Block);
	HitCollider->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Pal), ECR_Block);
	HitCollider->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	// 부모 변경 필요
	ProjectileMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);
	InitialLifeSpan = 5.f;
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
			// 박힌 지점을 Damage UI, (Resource - Item 드랍) 위치로 하기 위해 HitResult 추가
			DamageSpec.Data->GetContext().AddHitResult(Hit, true);
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
		}
	}

	Destroy();
}
