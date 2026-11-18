// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/DC1Projectile.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ADC1Projectile::ADC1Projectile()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	SetReplicateMovement(true);
	InitialLifeSpan = 5.f;

	// ProjectileMovement가 스윕하는 건 루트다. 판정 콜라이더를 루트에 두어야 경로 전체가 검사된다
	HitCollider = CreateDefaultSubobject<USphereComponent>("HitCollider");
	SetRootComponent(HitCollider);
	HitCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	HitCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>("ProjectileMesh");
	ProjectileMesh->SetupAttachment(HitCollider);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ProjectileMesh->SetRelativeLocationAndRotation(FVector(-55.f, 0.f, 0.f), FRotator(-90.f, 0.f, 0.f));

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.5f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ADC1Projectile::BeginPlay()
{
	Super::BeginPlay();

	// 쏜 사람 몸 안에서 출발하니 자기 자신에 걸리는 걸 막는다
	if (APawn* ProjectileInstigator = GetInstigator())
	{
		HitCollider->IgnoreActorWhenMoving(ProjectileInstigator, true);
	}

	UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
}

void ADC1Projectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == ::IsValid(OtherActor) || OtherActor == GetInstigator() || OtherActor == GetOwner()) return;

	if (false == bHitSoundPlayed)
	{
		bHitSoundPlayed = true;
		UGameplayStatics::PlaySoundAtLocation(this, (nullptr != Cast<APawn>(OtherActor)) ? HitCharacterSound : HitObjectSound, Hit.ImpactPoint);
	}

	if (false == HasAuthority()) return;

	// TODO : 팀필터 거르기
	if (DamageSpec.IsValid())
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OtherActor))
		{
			// 박힌 지점을 Damage UI, (Resource - Item 드랍) 위치로 하기 위해 HitResult 추가
			DamageSpec.Data->GetContext().AddHitResult(Hit, true);
			TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpec.Data.Get());
		}
	}
}
