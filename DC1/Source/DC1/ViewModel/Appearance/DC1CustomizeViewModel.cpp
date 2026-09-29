// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Appearance/DC1CustomizeViewModel.h"

#include "Save/DC1SaveSubsystem.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Appearance/DC1AppearanceComponent.h"

void UDC1CustomizeViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PlayerAppearanceComponent = InPlayerState->GetAppearanceComponent();
}

void UDC1CustomizeViewModel::ClearSource()
{
	PlayerAppearanceComponent.Reset();
	PreviewComponent.Reset();
}

void UDC1CustomizeViewModel::BeginEdit()
{
	if (true == PlayerAppearanceComponent.IsValid())
	{
		PendingAppearance = PlayerAppearanceComponent->CurrentAppearance;
	}
	else
	{
		UDC1SaveSubsystem* SaveSubsystem = GetSaveSubsystem();
		if (true == IsValid(SaveSubsystem) && true == SaveSubsystem->HasCustomizeLocalData())
		{
			SaveSubsystem->LoadCustomizeData(&PendingAppearance);
		}
	}

	ApplyPreview();
}

void UDC1CustomizeViewModel::BindPreviewComponent(UDC1AppearanceComponent* InPreviewComponent)
{
	PreviewComponent = InPreviewComponent;

	ApplyPreview();
}

void UDC1CustomizeViewModel::SetHair(const FGameplayTag& InTag)
{
	PendingAppearance.HairTag = InTag;
	ApplyPreview();
}

void UDC1CustomizeViewModel::SetHead(const FGameplayTag& InTag)
{
	PendingAppearance.HeadTag = InTag;
	ApplyPreview();
}

void UDC1CustomizeViewModel::SetEye(const FGameplayTag& InTag)
{
	PendingAppearance.EyeTag = InTag;
	ApplyPreview();
}

void UDC1CustomizeViewModel::SetHairColor(const FLinearColor& InColor)
{
	PendingAppearance.HairColor = InColor;
	ApplyPreview();
}

void UDC1CustomizeViewModel::SetSkinColor(const FLinearColor& InColor)
{
	PendingAppearance.SkinColor = InColor;
	ApplyPreview();
}

void UDC1CustomizeViewModel::SetEyeColor(const FLinearColor& InColor)
{
	PendingAppearance.EyeColor = InColor;
	ApplyPreview();
}

bool UDC1CustomizeViewModel::ConfirmAppearance()
{
	UDC1SaveSubsystem* SaveSubsystem = GetSaveSubsystem();
	if (false == IsValid(SaveSubsystem))
		return false;

	if (false == SaveSubsystem->SaveCustomizeData(PendingAppearance))
		return false;

	if (true == PlayerAppearanceComponent.IsValid())
		PlayerAppearanceComponent->ImportSaveData(PendingAppearance);

	return true;
}

UDC1SaveSubsystem* UDC1CustomizeViewModel::GetSaveSubsystem()
{
	return GetTypedOuter<ULocalPlayerSubsystem>()->GetLocalPlayer()->GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>();
}

void UDC1CustomizeViewModel::ApplyPreview()
{
	if (true == PreviewComponent.IsValid())
	{
		PreviewComponent->ApplyAppearance(PendingAppearance);
	}
}
