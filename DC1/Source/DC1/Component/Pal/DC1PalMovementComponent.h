// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DC1PalMovementComponent.generated.h"

/*
	팰 이동 컴포넌트 - 탑승 질주 플래그를 이동 예측에 싣는다
	팀원 UDC1CharacterMovementComponent 의 SPRINT 플래그와 같은 방식 ( 플레이어 전용 로직이 섞여 있어 따로 둔다 )
*/
struct FSavedMove_DC1Pal : public FSavedMove_Character
{
	typedef FSavedMove_Character Super;

	enum EMoveFlag
	{
		RIDE_SPRINT = CompressedFlags::FLAG_Custom_0,
	};

	virtual void	Clear() override;
	virtual uint8	GetCompressedFlags() const override;
	virtual bool	CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void	SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void	PrepMoveFor(ACharacter* C) override;

	uint8			Saved_bWantsToRideSprint : 1;
};

struct FNetworkPredictionData_Client_DC1Pal : public FNetworkPredictionData_Client_Character
{
	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_DC1Pal(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class DC1_API UDC1PalMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UDC1PalMovementComponent();

public:
	virtual float							GetMaxSpeed() const override;
	virtual class FNetworkPredictionData_Client*	GetPredictionData_Client() const override;

	// 탄 사람 클라에서 켜고 끈다. 이동 예측 플래그로 서버에 같이 간다
	void									SetRideSprint(bool InSprint);

	// 종 데이터에서 넣는다 ( ApplyPalData )
	void									SetRideSprintSpeed(float InSpeed) { RideSprintSpeed = InSpeed; }

protected:
	virtual void							UpdateFromCompressedFlags(uint8 Flags) override;

public:
	uint8									Safe_bWantsToRideSprint : 1;

private:
	float									RideSprintSpeed = { 0.f };
};
