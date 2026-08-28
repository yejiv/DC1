// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DC1AnimInstance_Player.h"

#include "Helper/DC1Macro.h"

#include "Character/Player/DC1Player.h"
#include "GameFramework/CharacterMovementComponent.h"

UDC1AnimInstance_Player::UDC1AnimInstance_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance_Player::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Player = Cast<ADC1Player>(Owner);
}

void UDC1AnimInstance_Player::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (false == IsValid(Player))
	{
		LOG(TEXT("Player X"));
		return;
	}
	if (false == IsValid(MovementComponent))
	{
		LOG(TEXT("MovementComponent X"));
		return;
	}

	IsCrouch = MovementComponent->IsCrouching();
	IsAiming = Player->GetAimingState();

	Compute_Direction();
}

void UDC1AnimInstance_Player::Compute_Direction()
{
	FVector LocalVelocity = Player->GetActorRotation().UnrotateVector(MovementComponent->Velocity);
	if (LocalVelocity.Size2D() == 0)
	{
		MoveXY = {};
		return;
	}
	
	MoveXY.X = LocalVelocity.X;
	MoveXY.Y = LocalVelocity.Y;
}
