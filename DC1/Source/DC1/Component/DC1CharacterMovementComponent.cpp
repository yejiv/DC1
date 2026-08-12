// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DC1CharacterMovementComponent.h"

#include "GameFramework/Character.h"

#pragma region SavedMove
FSavedMove_DC1::FSavedMove_DC1()
	: Super()
{
}

void FSavedMove_DC1::Clear()
{
	Super::Clear();

	Saved_bWantsToWalk = false;
	Saved_bWantsToSprint = false;
}

uint8 FSavedMove_DC1::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (Saved_bWantsToWalk) Result |= WALK;
	if (Saved_bWantsToSprint) Result |= SPRINT;
	return Result;
}

bool FSavedMove_DC1::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	if (GetCompressedFlags() != NewMove->GetCompressedFlags()) { return false; }

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_DC1::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(C->GetCharacterMovement());

	if (nullptr != CMC)
	{
		Saved_bWantsToWalk = CMC->Safe_bWantsToWalk;
		Saved_bWantsToSprint = CMC->Safe_bWantsToSprint;
	}
}

void FSavedMove_DC1::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(C->GetCharacterMovement());

	if (nullptr != CMC)
	{
		CMC->Safe_bWantsToWalk = Saved_bWantsToWalk;
		CMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
	}
}
#pragma endregion

#pragma region NetworkPredictionData
FNetworkPredictionData_Client_DC1::FNetworkPredictionData_Client_DC1(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_DC1::AllocateNewMove()
{
	return MakeShared<FSavedMove_DC1>();
}
#pragma endregion

UDC1CharacterMovementComponent::UDC1CharacterMovementComponent()
	: Super()
{
}

float UDC1CharacterMovementComponent::GetMaxSpeed() const
{
	if (MOVE_Walking != MovementMode && MOVE_NavWalking != MovementMode && MOVE_Falling != MovementMode)
	{
		return Super::GetMaxSpeed();
	}

	float Speed = {};

	if (Safe_bWantsToSprint)
	{
		Speed = SprintSpeed;
	}
	else
	{
		Speed = Safe_bWantsToWalk ? WalkSpeed : JogSpeed;
	}

	return Speed;
}

FNetworkPredictionData_Client* UDC1CharacterMovementComponent::GetPredictionData_Client() const
{
	if (nullptr == PawnOwner) { return nullptr; }

	if (nullptr == ClientPredictionData)
	{
		UDC1CharacterMovementComponent* CMC = const_cast<UDC1CharacterMovementComponent*>(this);

		CMC->ClientPredictionData = new FNetworkPredictionData_Client_DC1(*this);
		CMC->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		CMC->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UDC1CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToWalk = (0 != (Flags & FSavedMove_DC1::EMoveFlag::WALK));
	Safe_bWantsToSprint = (0 != (Flags & FSavedMove_DC1::EMoveFlag::SPRINT));
}

void UDC1CharacterMovementComponent::ToggleMove()
{
	Safe_bWantsToWalk = ~Safe_bWantsToWalk;
}

void UDC1CharacterMovementComponent::SetSprint(bool Sprint)
{
	Safe_bWantsToSprint = Sprint;
}
