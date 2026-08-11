// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"
#include "Component/DC1AbilitySystemComponent.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionSystem.h"

ADC1PalCharacter::ADC1PalCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AIControllerClass = ADC1PalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	PalAttributeSet = CreateDefaultSubobject<UDC1PalAttributeSet>(TEXT("PalAttributeSet"));

	// 회전쓰지 않음
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	Movement->bOrientRotationToMovement = true;
	Movement->RotationRate = FRotator(0.f, 360.f, 0.f);
	Movement->MaxWalkSpeed = 400.f;
	Movement->bUseRVOAvoidance = true;
}

void ADC1PalCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	ApplyPalData();
}

void ADC1PalCharacter::ApplyPalData()
{
	if (false == SpeciesTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: SpeciesTag is not set."), *GetName());
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: Can't find PalRegistryData. Check PDA_AssetData."), *GetName());
		return;
	}

	// 나중에 스폰 전에 비동기로 미리 올려두는 쪽으로 바꾸기.
	PalData = Registry->FindPalDataByTag(SpeciesTag).LoadSynchronous();
	if (false == ::IsValid(PalData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: Failed to load PalData [%s]."), *GetName(), *SpeciesTag.ToString());
		return;
	}

	// Locomotion
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = PalData->RunSpeed;
		Movement->RotationRate = FRotator(0.f, PalData->RotationRateYaw, 0.f);
	}

	if (::IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilitySystem(this, this, PalData->AbilityDataTag);
	}

	// Stats
	if (::IsValid(PalAttributeSet))
	{
		PalAttributeSet->InitMaxHealth(PalData->MaxHealth);
		PalAttributeSet->InitHealth(PalData->MaxHealth);
	}

	// OnPossess때 PalData가 없어서 컨트롤러가 생성자 기본값을 씀. 다시 적용
	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->ApplyPerceptionSettings();
	}

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s: PalData applied [%s]."), *GetName(), *SpeciesTag.ToString());
}

UAbilitySystemComponent* ADC1PalCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADC1PalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADC1PalCharacter, Team);
}

ETeamAttitude::Type ADC1PalCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	return DC1Team::GetAttitude(Team, DC1Team::ResolveTeam(Other));
}

FGenericTeamId ADC1PalCharacter::GetGenericTeamId() const
{
	return DC1Team::ToGenericTeamId(Team);
}

void ADC1PalCharacter::SetTeam(EDC1Team NewTeam)
{
	if (false == HasAuthority())
	{
		return;
	}

	if (Team == NewTeam)
	{
		return;
	}

	Team = NewTeam;
	OnTeamChanged();
}

void ADC1PalCharacter::OnRep_Team()
{
	OnTeamChanged();
}

void ADC1PalCharacter::OnTeamChanged()
{
	// 판정은 sight query 생성 시점에 한 번만 + 진영이 바뀌면 재등록
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->RegisterSource(*this);
	}
}
