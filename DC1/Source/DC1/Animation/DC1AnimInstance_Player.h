// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/DC1AnimInstance.h"
#include "DC1AnimInstance_Player.generated.h"

class ADC1Player;
class UDC1CharacterMovementComponent;

UCLASS()
class DC1_API UDC1AnimInstance_Player : public UDC1AnimInstance
{
	GENERATED_BODY()

public:
	UDC1AnimInstance_Player(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	UPROPERTY(BlueprintReadOnly)
	bool				IsCrouch = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsAiming = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsClimbing = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsGliding = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsOnGround = { false };
	UPROPERTY(BlueprintReadOnly)
	bool				IsDown = { false };
	// 팰 등에 붙어 있는지 (올라타기, 내리기 몽타주 중에도 true)
	UPROPERTY(BlueprintReadOnly)
	bool				IsRiding = { false };

	UPROPERTY(BlueprintReadOnly)
	FVector2D		MoveXY = {};
	UPROPERTY(BlueprintReadOnly)
	FVector2D		ClimbVelocity = {};

	// 위/아래 조준각. 척추에 나눠서 먹인다. 액터 정면 기준 상대값
	UPROPERTY(BlueprintReadOnly)
	float				AimPitch = {};

public:
	// 척추가 꺾이는 한계각
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim")
	float				MaxAimPitch = { 60.f };

	// 클수록 조준각이 빠르게 따라붙는다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Aim")
	float				AimPitchInterpSpeed = { 15.f };

private:
	void			Compute_Direction();
	void			Compute_AimPitch(float DeltaSeconds);
	void			Compute_ClimbVelocity();

private:
	UPROPERTY()
	TObjectPtr<ADC1Player>		Player = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1CharacterMovementComponent>		CMC = { nullptr };
};
