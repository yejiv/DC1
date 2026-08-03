// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1Player.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"

ADC1Player::ADC1Player()
	: Super()
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	USkeletalMeshComponent* BodyMesh = GetMesh();

	// HeadMesh
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HeadMesh");
	HeadMesh->SetupAttachment(BodyMesh);

	// HairMesh & HairEquipMesh
	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HairMesh");
	HairMesh->SetupAttachment(BodyMesh);
	HairEquipMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HairEquipMesh");
	HairEquipMesh->SetupAttachment(BodyMesh);

	// SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(BodyMesh);
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 90.f));
	SpringArm->TargetArmLength = 800.f;
	SpringArm->bUsePawnControlRotation = true;
	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->bUsePawnControlRotation = false;
}

void ADC1Player::BeginPlay()
{
	Super::BeginPlay();
}
