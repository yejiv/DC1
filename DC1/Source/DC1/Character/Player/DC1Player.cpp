// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1Player.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Camera/CameraComponent.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1CharacterMovementComponent.h"

#include "Tags/DC1GamePlayTags.h"

ADC1Player::ADC1Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDC1CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
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

UAbilitySystemComponent* ADC1Player::GetAbilitySystemComponent() const
{
	if (ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
}

void ADC1Player::ToggleMove()
{
	CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement())->ToggleMove();
}

void ADC1Player::SetSprint(bool Sprint)
{
	CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement())->SetSprint(Sprint);
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
}

void ADC1Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilitySystem();

	// Temp(Equip None)
	Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent())->SetExclusiveStateTag(DC1StateTags::State_Weapon, DC1StateTags::State_Weapon_None);
}

void ADC1Player::UnPossessed()
{
	if (ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>())
	{
		PS->GetAbilitySystemComponent()->SetAvatarActor(nullptr);
	}
	Super::UnPossessed();
}

void ADC1Player::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
}

void ADC1Player::InitializeAbilitySystem()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS))
	{
		return;
	}
	UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
	//ASC->InitAbilityActorInfo(PS, this);
	ASC->InitAbilitySystem(PS, this, DC1AssetTags::Asset_Ability_Player);
}
