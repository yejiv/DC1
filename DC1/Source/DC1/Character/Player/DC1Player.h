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

private:
	virtual void BeginPlay() override;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HeadMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairEquipMesh = { nullptr };

	TObjectPtr<USpringArmComponent>		SpringArm = { nullptr };
	TObjectPtr<UCameraComponent>			Camera = { nullptr };
};
