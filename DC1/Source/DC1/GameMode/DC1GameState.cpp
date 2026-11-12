// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DC1GameState.h"

#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Save/DC1SaveSubsystem.h"

#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"

void ADC1GameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BasePalBox);
}

ADC1GameState* ADC1GameState::Get(const UObject* InWorldContext)
{
	if (nullptr == InWorldContext)
	{
		return nullptr;
	}

	UWorld* World = InWorldContext->GetWorld();
	if (false == ::IsValid(World))
	{
		return nullptr;
	}

	return World->GetGameState<ADC1GameState>();
}

bool ADC1GameState::HasBase() const
{
	return true == ::IsValid(BasePalBox);
}

ADC1PalBoxActor* ADC1GameState::FindBaseAt(const FVector& InLocation) const
{
	if (false == HasBase() || false == BasePalBox->IsInBase(InLocation))
	{
		return nullptr;
	}

	return BasePalBox;
}

void ADC1GameState::SetBase(ADC1PalBoxActor* InPalBox)
{
	if (false == HasAuthority() || false == ::IsValid(InPalBox))
	{
		return;
	}

	if (true == HasBase())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Base] SetBase rejected: %s is already the base."), *GetNameSafe(BasePalBox));
		return;
	}

	BasePalBox = InPalBox;
	BasePalBox->OnEndPlay.AddDynamic(this, &ThisClass::HandleBaseEndPlay);
}

void ADC1GameState::NotifyBaseChanged()
{
	for (const TObjectPtr<APlayerState>& PS : PlayerArray)
	{
		ADC1PlayerState* DC1PS = Cast<ADC1PlayerState>(PS);
		if (false == ::IsValid(DC1PS))
		{
			continue;
		}

		UDC1PalPartyComponent* PalPartyComp = DC1PS->GetPalPartyComponent();
		if (false == ::IsValid(PalPartyComp))
		{
			continue;
		}

		PalPartyComp->HandleBaseChanged();
	}
}

UDC1PalPartyComponent* ADC1GameState::FindPalPartyBySlotName(const FString& InSlotName) const
{
	if (true == InSlotName.IsEmpty())
	{
		return nullptr;
	}

	const UGameInstance* GI = GetGameInstance();
	UDC1SaveSubsystem* SaveSubsystem = (nullptr != GI) ? GI->GetSubsystem<UDC1SaveSubsystem>() : nullptr;
	if (false == ::IsValid(SaveSubsystem))
	{
		return nullptr;
	}

	for (const TObjectPtr<APlayerState>& PS : PlayerArray)
	{
		ADC1PlayerState* DC1PS = Cast<ADC1PlayerState>(PS);
		if (false == ::IsValid(DC1PS) || InSlotName != SaveSubsystem->MakeSlotName(DC1PS))
		{
			continue;
		}

		return DC1PS->GetPalPartyComponent();
	}

	return nullptr;
}

void ADC1GameState::OnRep_BasePalBox()
{
	NotifyBaseChanged();
}

void ADC1GameState::HandleBaseEndPlay(AActor* InActor, EEndPlayReason::Type InEndPlayReason)
{
	BasePalBox = nullptr;

	NotifyBaseChanged();
}
