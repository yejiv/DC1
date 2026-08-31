// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/Pal/DC1PalSphere.h"

#include "Character/Pal/DC1PalCharacter.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ADC1PalSphere::ADC1PalSphere()
	: Super()
{
	// 데미지를 주는 기본 명중 처리 대신 포획 판정을 건다
	HitCollider->OnComponentHit.RemoveDynamic(this, &ThisClass::OnProjectileHit);
	HitCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnSphereHit);

	HitCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitCollider->SetCollisionObjectType(ECC_WorldDynamic);
	HitCollider->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitCollider->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	HitCollider->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	ProjectileMesh->SetRelativeLocationAndRotation(FVector::ZeroVector, FRotator::ZeroRotator);

	//ProjectileMovement->InitialSpeed = 1500.f;
	//ProjectileMovement->MaxSpeed = 1500.f;
	//ProjectileMovement->ProjectileGravityScale = 1.f;
}

float ADC1PalSphere::ComputeCaptureChance(float InHealthRatio, float InCaptureRate) const
{
	const float HealthFactor = 1.f - (InHealthRatio * HealthWeight);

	return FMath::Clamp(InCaptureRate * HealthFactor * CaptureMultiplier, 0.f, 1.f);
}

void ADC1PalSphere::OnSphereHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == HasAuthority())
	{
		return;
	}

	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(OtherActor);

	if (false == IsValid(Pal))
	{
		Destroy();
		return;
	}

	// 회수
	if (EDC1Team::Wild != Pal->GetTeam())
	{
		UE_LOG(LogTemp, Log, TEXT("[Pal] Sphere hit %s but it is not wild (Team=%s). Ignored."),
			*GetNameSafe(Pal), *UEnum::GetValueAsString(Pal->GetTeam()));

		Destroy();
		return;
	}

	const float Chance = ComputeCaptureChance(Pal->GetHealthRatio(), Pal->GetCaptureRate());
	const bool bSuccess = (FMath::FRand() < Chance);

	UE_LOG(LogTemp, Log, TEXT("[Pal] Sphere hit %s. Health=%.0f%% Rate=%.2f Mult=%.1f -> Chance=%.0f%% (%s)"),
		*GetNameSafe(Pal),
		Pal->GetHealthRatio() * 100.f,
		Pal->GetCaptureRate(),
		CaptureMultiplier,
		Chance * 100.f,
		bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));

	if (true == bSuccess)
	{
		const FDC1PalInstance Captured = Pal->Fold();
		OnPalCaptured.Broadcast(Captured);
	}
	else
	{
		Pal->OnCaptureFailed(GetInstigator());
	}

	Destroy();
}
