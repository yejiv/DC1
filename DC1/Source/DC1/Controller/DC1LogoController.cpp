// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/DC1LogoController.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"
#include "Save/DC1SaveSubsystem.h"

void ADC1LogoController::BeginPlay()
{
	Super::BeginPlay();

	if (false == IsLocalController())
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	UIManager->FadeIn(0.f, EDC1FadeType::Spinner);
	UIManager->PushScreen(DC1UITags::UI_Logo_Title);

	FTimerHandle FadeOutTimerHandle;
	GetWorldTimerManager().SetTimer(FadeOutTimerHandle, FTimerDelegate::CreateWeakLambda(UIManager, [UIManager]()
		{
			UIManager->FadeOut(2.f);
		}), 2.f, false);
}

void ADC1LogoController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (nullptr != InputComponent)
	{
		InputComponent->BindKey(EKeys::NumPadNine, IE_Pressed, this, &ThisClass::OnDebugToggleUI);
	}
}

void ADC1LogoController::OnDebugToggleUI()
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	if (true == UIManager->HasActiveScreen())
	{
		UIManager->PopScreen();
	}
	else
	{
		UIManager->PushScreen(DC1UITags::UI_Logo_Title);
	}
}

void ADC1LogoController::TravelToServer(const FString& IP)
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	UDC1SaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>();
	if (true == ::IsValid(SaveSubsystem) && false == SaveSubsystem->HasCustomizeLocalData())
	{
		if (true == ::IsValid(UIManager))
		{
			CachedIP = IP;

			UIManager->PushScreen(DC1UITags::UI_Customize, true);
			return;
		}
	}
	else
	{
		UIManager->BeginLoadingTransition(1.f, FSimpleDelegate::CreateWeakLambda(this, [this, IP]
			{
				ClientTravel(IP, TRAVEL_Absolute);
			}));
	}
}

void ADC1LogoController::TravelToServerFromCachedIP()
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	UIManager->BeginLoadingTransition(1.f, FSimpleDelegate::CreateWeakLambda(this, [this]
		{
			ClientTravel(CachedIP, TRAVEL_Absolute);
		}));
}
