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
#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"

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

void ADC1Player::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clear_Resource();
	Super::EndPlay(EndPlayReason);
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
	Clear_Resource();
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
	ASC->InitAbilitySystem(PS, this, DC1AssetTags::Asset_Ability_Player);

	// Weapon State Change Delegate Register
	if (WeaponStateTagHandles.Num() > 0) return;

	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		for (const FGameplayTag& Tag : AnimData->GetAnimInstanceKeys())
		{
			WeaponStateTagHandles.Add(Tag, 
				ASC->RegisterAndCallGameplayTagEvent(Tag, 
					FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(
				this, &ThisClass::OnWeaponStateChanged)));
		}
	}
}

void ADC1Player::Clear_Resource()
{
	if (ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>())
	{
		UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		ASC->SetAvatarActor(nullptr);

		for (TPair pair : WeaponStateTagHandles)
		{
			ASC->UnregisterGameplayTagEvent(pair.Value, pair.Key);
		}
		WeaponStateTagHandles.Reset();
	}
}

void ADC1Player::OnWeaponStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount <= 0) return;

	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		if (TSubclassOf<UAnimInstance> LayerClass = AnimData->FindAnimInstanceByTag(Tag))
		{
			if (UAnimInstance* Instance = GetMesh()->GetAnimInstance())
			{
				Instance->LinkAnimClassLayers(LayerClass);
			}
		}
	}
}
