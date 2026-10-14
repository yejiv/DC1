// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DC1AnimInstance_Player.h"

#include "Define/DC1Macro.h"

#include "Character/Player/DC1Player.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Component/DC1CharacterMovementComponent.h"

UDC1AnimInstance_Player::UDC1AnimInstance_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance_Player::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Player = Cast<ADC1Player>(Owner);
	CMC = Cast<UDC1CharacterMovementComponent>(MovementComponent);
}

void UDC1AnimInstance_Player::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (false == IsValid(Player))
	{
		LOG(TEXT("Player X"));
		return;
	}
	if (false == IsValid(CMC))
	{
		LOG(TEXT("MovementComponent X"));
		return;
	}

	IsCrouch = CMC->IsCrouching();
	IsAiming = Player->GetAimingState();
	IsClimbing = CMC->IsClimbing();
	IsGliding = CMC->IsGliding();
	IsOnGround = CMC->IsMovingOnGround();

	Compute_Direction();
	Compute_AimPitch(DeltaSeconds);
	Compute_ClimbVelocity();
}

void UDC1AnimInstance_Player::Compute_AimPitch(float DeltaSeconds)
{
	// GetControlRotation은 시뮬레이티드 프록시에서 유효하지 않다.
	// GetBaseAimRotation은 프록시일 때 복제된 RemoteViewPitch를 대신 돌려준다
	const FRotator Delta = (Player->GetBaseAimRotation() - Player->GetActorRotation()).GetNormalized();
	const float TargetPitch = FMath::Clamp(Delta.Pitch, -MaxAimPitch, MaxAimPitch);

	// RemoteViewPitch는 1바이트로 압축돼 오므로 프록시에서는 값이 계단처럼 튄다. 보간으로 눌러준다
	AimPitch = FMath::FInterpTo(AimPitch, TargetPitch, DeltaSeconds, AimPitchInterpSpeed);
}

void UDC1AnimInstance_Player::Compute_ClimbVelocity()
{
	FVector LocalVelocity = Player->GetActorTransform().InverseTransformVectorNoScale(CMC->Velocity);

	ClimbVelocity.X = LocalVelocity.Y;
	ClimbVelocity.Y = LocalVelocity.Z;
}

void UDC1AnimInstance_Player::Compute_Direction()
{
	FVector LocalVelocity = Player->GetActorRotation().UnrotateVector(CMC->Velocity);
	if (LocalVelocity.Size2D() == 0)
	{
		MoveXY = {};
		return;
	}
	
	MoveXY.X = LocalVelocity.X;
	MoveXY.Y = LocalVelocity.Y;
}
