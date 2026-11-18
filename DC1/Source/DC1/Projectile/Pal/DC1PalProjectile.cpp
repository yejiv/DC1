// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalProjectile.h"

#include "Character/Pal/DC1PalCharacter.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/GameplayStatics.h"
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

void ADC1PalProjectile::BeginPlay()
{
	Super::BeginPlay();

	// bStopWhenAttachedToDestroyed 기본값 false - 안 켜면 투사체가 사라져도 루프가 계속 남는다
	UGameplayStatics::SpawnSoundAttached(LoopSound, GetRootComponent(), NAME_None, FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
}

void ADC1PalProjectile::OnPalProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == HasAuthority())
	{
		return;
	}

	// 서버가 맞자마자 Destroy 해서 뒤처진 클라 사본은 Hit 전에 지워진다 → 살아있는 쏜 팰의 Multicast 로 재생
	if (false == bHitSoundPlayed)
	{
		bHitSoundPlayed = true;

		USoundBase* HitSound = (nullptr != Cast<APawn>(OtherActor)) ? HitCharacterSound.Get() : HitObjectSound.Get();
		if (ADC1PalCharacter* ShooterPal = Cast<ADC1PalCharacter>(GetInstigator()))
		{
			ShooterPal->Multicast_PlaySkillSound(HitSound, Hit.ImpactPoint, false);
		}
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
