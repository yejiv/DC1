// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile/DC1Arrow.h"

#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Character.h"

ADC1Arrow::ADC1Arrow()
	: Super()
{
}

void ADC1Arrow::OnRep_ReplicatedMovement()
{
	// 화살이 이미 박힌 후엔 서버 복제 무시
	// 서버 화살은 캡슐 표면에 멈춰 있음 -> 늦게 온 위치로 끌려가지 않게
	if (true == bStuckLocally)
		return;

	Super::OnRep_ReplicatedMovement();
}

void ADC1Arrow::OnProjectileHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (false == ::IsValid(OtherActor) || OtherActor == GetInstigator() || OtherActor == GetOwner()) return;
	
	Super::OnProjectileHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);

	HitCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (true == HasAuthority())
	{
		SetLifeSpan(StuckLifeSpan);
	}
	
	if (NM_DedicatedServer != GetNetMode())
	{
		if (ACharacter* Character = Cast<ACharacter>(OtherActor))
		{
			StickToCharacterMesh(Character, Hit);
		}
	}
}

void ADC1Arrow::StickToCharacterMesh(ACharacter* InCharacter, const FHitResult& InHit)
{
	USkeletalMeshComponent* Mesh = InCharacter->GetMesh();
	if (false == IsValid(Mesh))
		return;

	// bRotationFollowsVelocity라 Forward == 날아온 방향
	const FVector Direction = GetActorForwardVector();
	const FVector TraceStart = InHit.ImpactPoint - Direction * MeshTraceDistance;
	const FVector TraceEnd = InHit.ImpactPoint + Direction * MeshTraceDistance;

	FHitResult MeshHit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(ArrowStick), false);
	if (true == Mesh->LineTraceComponent(MeshHit, TraceStart, TraceEnd, Params))
	{
		SetActorLocation(MeshHit.ImpactPoint + Direction * PenetrationDepth);
		AttachToComponent(Mesh, FAttachmentTransformRules::KeepWorldTransform, MeshHit.BoneName);
	}
	else
	{
		// 팔다리 사이로 빠진 경우 가장 가까운 본에 부착
		AttachToComponent(Mesh, FAttachmentTransformRules::KeepWorldTransform, Mesh->FindClosestBone(InHit.ImpactPoint));
	}

	bStuckLocally = true;
}
