// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "DC1Player.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
/**
 * 
 */
UCLASS()
class DC1_API ADC1Player : public ADC1Character
{
	GENERATED_BODY()

public:
	ADC1Player();

public:
	void ToggleMove();
	void SetSprint(bool Sprint);

private:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HeadMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairEquipMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent>		SpringArm = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCameraComponent>			Camera = { nullptr };

private:
	void	UpdateMoveSpeed();

private:
	bool	IsWalk = { false };
	bool	IsSprint = { false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	WalkSpeed = { 150.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	JogSpeed = { 300.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	SprintSpeed = { 650.f };

	float	SpeedRate = { 1.f };
};
