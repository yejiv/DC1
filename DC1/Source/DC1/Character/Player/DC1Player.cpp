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
	BodyMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));

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
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	SpringArm->TargetArmLength = 200.f;
	SpringArm->bUsePawnControlRotation = true;
	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeLocation(FVector(0.f, 50.f, 0.f));
	Camera->bUsePawnControlRotation = false;
}

void ADC1Player::ToggleMove()
{
	IsWalk = !IsWalk;
	UpdateMoveSpeed();
}

void ADC1Player::SetSprint(bool Sprint)
{
	IsSprint = Sprint;
	UpdateMoveSpeed();
}

void ADC1Player::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HairMesh->SetLeaderPoseComponent(GetMesh());
	HairEquipMesh->SetLeaderPoseComponent(GetMesh());
}

void ADC1Player::BeginPlay()
{
	Super::BeginPlay();

	UpdateMoveSpeed();
}

void ADC1Player::UpdateMoveSpeed()
{
	float Speed = {};

	if (true == IsSprint)
	{
		Speed = SprintSpeed;
	}
	else
	{
		Speed = IsWalk ? WalkSpeed : JogSpeed;
	}

	GetCharacterMovement()->MaxWalkSpeed = Speed * SpeedRate;
}
