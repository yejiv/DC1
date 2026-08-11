// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DC1AnimInstance.h"

#include "Helper/DC1Macro.h"

#include "Character/DC1Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDC1AnimInstance::UDC1AnimInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Owner = Cast<ADC1Character>(TryGetPawnOwner());
	if (IsValid(Owner))
	{
		MovementComponent = Owner->GetCharacterMovement();
	}
}

void UDC1AnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (false == IsValid(Owner))
	{ 
		LOG(TEXT("Owner X"));
		return;
	}
	if (false == IsValid(MovementComponent))
	{
		LOG(TEXT("MovementComponent X"));
		return;
	}

	FVector Velocity = MovementComponent->Velocity;
	GroundSpeed = Velocity.Size2D();
	VelocityZ = Velocity.Z;

	IsMove = GroundSpeed > 0.f && MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector;
	IsFalling = MovementComponent->IsFalling();
}
