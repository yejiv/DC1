// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "DC1CharacterMovementComponent.generated.h"

struct FSavedMove_DC1 : public FSavedMove_Character {

	typedef FSavedMove_Character Super;

	enum EMoveFlag {
		WALK = CompressedFlags::FLAG_Custom_0,
		SPRINT = CompressedFlags::FLAG_Custom_1
	};

	FSavedMove_DC1();

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;

	uint8 Saved_bWantsToWalk : 1;
	uint8 Saved_bWantsToSprint : 1;
};

struct FNetworkPredictionData_Client_DC1 : public FNetworkPredictionData_Client_Character {

	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_DC1(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;

};

UCLASS()
class DC1_API UDC1CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UDC1CharacterMovementComponent();

	virtual float GetMaxSpeed() const override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

public:
	void ToggleMove();
	void SetSprint(bool Sprint);

public:

	uint8 Safe_bWantsToWalk : 1;
	uint8 Safe_bWantsToSprint : 1;

private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	WalkSpeed = { 150.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	JogSpeed = { 300.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	SprintSpeed = { 650.f };
};
