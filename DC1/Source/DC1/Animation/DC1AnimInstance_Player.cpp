// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DC1AnimInstance_Player.h"

#include "Helper/DC1Macro.h"

#include "Character/DC1Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UDC1AnimInstance_Player::UDC1AnimInstance_Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance_Player::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
}

void UDC1AnimInstance_Player::NativeUpdateAnimation(float DeltaSeconds)
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

	IsCrouch = MovementComponent->IsCrouching();
}
