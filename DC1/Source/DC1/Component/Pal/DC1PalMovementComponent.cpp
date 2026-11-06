// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalMovementComponent.h"
#include "GameFramework/Character.h"

#pragma region SavedMove
void FSavedMove_DC1Pal::Clear()
{
	Super::Clear();

	Saved_bWantsToRideSprint = false;
}

uint8 FSavedMove_DC1Pal::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (Saved_bWantsToRideSprint)
	{
		Result |= RIDE_SPRINT;
	}

	return Result;
}

bool FSavedMove_DC1Pal::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	if (GetCompressedFlags() != NewMove->GetCompressedFlags())
	{
		return false;
	}

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_DC1Pal::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	if (const UDC1PalMovementComponent* CMC = Cast<UDC1PalMovementComponent>(C->GetCharacterMovement()))
	{
		Saved_bWantsToRideSprint = CMC->Safe_bWantsToRideSprint;
	}
}

void FSavedMove_DC1Pal::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	if (UDC1PalMovementComponent* CMC = Cast<UDC1PalMovementComponent>(C->GetCharacterMovement()))
	{
		CMC->Safe_bWantsToRideSprint = Saved_bWantsToRideSprint;
	}
}
#pragma endregion

#pragma region NetworkPredictionData
FNetworkPredictionData_Client_DC1Pal::FNetworkPredictionData_Client_DC1Pal(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_DC1Pal::AllocateNewMove()
{
	return MakeShared<FSavedMove_DC1Pal>();
}
#pragma endregion

UDC1PalMovementComponent::UDC1PalMovementComponent()
	: Super()
{
	Safe_bWantsToRideSprint = false;
}

float UDC1PalMovementComponent::GetMaxSpeed() const
{
	// 땅 위에서만 질주. 점프 중엔 질주 속도를 유지한다
	const bool bGroundOrAir = (MOVE_Walking == MovementMode || MOVE_NavWalking == MovementMode || MOVE_Falling == MovementMode);
	if (true == Safe_bWantsToRideSprint && true == bGroundOrAir && RideSprintSpeed > 0.f)
	{
		return RideSprintSpeed;
	}

	return Super::GetMaxSpeed();
}

FNetworkPredictionData_Client* UDC1PalMovementComponent::GetPredictionData_Client() const
{
	if (nullptr == PawnOwner)
	{
		return nullptr;
	}

	if (nullptr == ClientPredictionData)
	{
		UDC1PalMovementComponent* CMC = const_cast<UDC1PalMovementComponent*>(this);
		CMC->ClientPredictionData = new FNetworkPredictionData_Client_DC1Pal(*this);
	}

	return ClientPredictionData;
}

void UDC1PalMovementComponent::SetRideSprint(bool InSprint)
{
	Safe_bWantsToRideSprint = InSprint;
}

void UDC1PalMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToRideSprint = (0 != (Flags & FSavedMove_DC1Pal::EMoveFlag::RIDE_SPRINT));
}
