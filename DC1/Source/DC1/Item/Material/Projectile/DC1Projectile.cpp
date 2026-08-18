// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Material/Projectile/DC1Projectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ADC1Projectile::ADC1Projectile()
	: Super()
{
	SetReplicateMovement(true);
	InitialLifeSpan = 5.f;

	// ProjectileMovement는 루트를 스윕시키므로 명중 판정도 루트인 InteractCollider가 받는다.
	// 비행 중에는 명중 판정, 땅에 박힌 뒤에는 줍기 판정으로 역할이 바뀐다
	InteractCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractCollider->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
}

void ADC1Projectile::BeginPlay()
{
	Super::BeginPlay();

	// 활이 쏜 사람 몸 안에 있어서 발사 즉시 자기 자신에 박히는 걸 막는다
	if (APawn* ProjectileInstigator = GetInstigator())
	{
		InteractCollider->IgnoreActorWhenMoving(ProjectileInstigator, true);
	}
}

void ADC1Projectile::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == HasAuthority()) return;
	if (false == ::IsValid(OtherActor) || OtherActor == GetInstigator() || OtherActor == GetOwner()) return;

	// TODO : 데미지 적용 경로 확정 후 채운다 (근접은 GA_Player_Attack::OnHitReceived가 담당)

	ProjectileMovement->StopMovementImmediately();
	Destroy();
}
