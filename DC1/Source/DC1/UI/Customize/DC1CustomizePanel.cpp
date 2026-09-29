// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Customize/DC1CustomizePanel.h"

#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"

#include "UI/Button/Customize/DC1CustomizeIcon.h"

#include "Data/DA/Appearance/DC1AppearanceRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/LocalPlayer.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Appearance/DC1CustomizeViewModel.h"

#include "UI/Customize/DC1ColorWheel.h"
#include "UI/Button/Customize/DC1CustomizeColorButton.h"

void UDC1CustomizePanel::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
	PopulateIcons();

	const FLinearColor CurrentColor = (true == CustomizeViewModel.IsValid())
		? GetColorFromData(CustomizeViewModel->GetPendingAppearance())
		: FLinearColor::White;

	if (true == IsValid(ColorWheel))
	{
		ColorWheel->OnColorChanged.AddDynamic(this, &ThisClass::HandleColorChanged);
		ColorWheel->OnPickClosed.AddDynamic(this, &ThisClass::HandleColorPickClosed);
		ColorWheel->SetSeedColor(CurrentColor);
		ColorWheel->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (true == IsValid(ColorButton))
	{
		ColorButton->OnClickedDelegate.AddWeakLambda(this, [this]() { HandleColorButtonClicked(); });
		ColorButton->SetPreviewColor(CurrentColor);
	}
}

void UDC1CustomizePanel::NativeDestruct()
{
	for (int32 i = 0; i < IconNum; ++i)
	{
		Icons[i]->OnClickedDelegate.RemoveAll(this);
	}
	Icons.Reset();

	if (true == IsValid(ColorWheel))
	{
		ColorWheel->OnColorChanged.RemoveAll(this);
		ColorWheel->OnPickClosed.RemoveAll(this);
	}

	if (true == IsValid(ColorButton))
	{
		ColorButton->OnClickedDelegate.RemoveAll(this);
	}

	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1CustomizePanel::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		CustomizeViewModel = VMSubsystem->GetViewModel<UDC1CustomizeViewModel>();
	}
}

void UDC1CustomizePanel::UnbindViewModel()
{
	CustomizeViewModel.Reset();
}

void UDC1CustomizePanel::PopulateIcons()
{
	const UDC1AppearanceRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1AppearanceRegistryData>(DC1AssetTags::Asset_Appearance_Registry);
	if (false == IsValid(Registry))
	{
		return;
	}

	for (int32 i = 0; i < IconNum; ++i)
	{
		const FGameplayTag PartTag = MakePartTag(i);

		bool bFound = false;
		TSoftObjectPtr<UTexture2D> IconSoftPtr;
		switch (MenuType)
		{
		case EDC1CustomizeMenu::Head:
			if (const FDC1AppearancePartDef* PartDef = Registry->GetHeadData(PartTag))
			{
				IconSoftPtr = PartDef->Icon;
				bFound = true;
			}
			break;
		case EDC1CustomizeMenu::Hair:
			if (const FDC1AppearancePartDef* PartDef = Registry->GetHairData(PartTag))
			{
				IconSoftPtr = PartDef->Icon;
				bFound = true;
			}
			break;
		case EDC1CustomizeMenu::Eye:
			if (const FDC1AppearanceMaterialDef* PartDef = Registry->GetEyeData(PartTag))
			{
				IconSoftPtr = PartDef->Icon;
				bFound = true;
			}
			break;
		}

		if (false == bFound)
		{
			continue;
		}

		UTexture2D* IconTexture = IconSoftPtr.LoadSynchronous();

		UDC1CustomizeIcon* Icon = CreateWidget<UDC1CustomizeIcon>(this, IconClass);
		Icon->Init(PartTag, IconTexture);
		Icons.Add(Icon);

		int32 Row = i / FixedCol;
		int32 Col = i % FixedCol;

		IconContainer->AddChildToUniformGrid(Icon, Row, Col);
		IconContainer->SetSlotPadding(FMargin(Interval, Interval));
		Icon->OnClickedDelegate.AddWeakLambda(this, [this, Icon]() {HandleIconClicked(Icon); });
	}
}

void UDC1CustomizePanel::HandleIconClicked(UDC1CustomizeIcon* ClickedIcon)
{
	if (false == IsValid(ClickedIcon))
		return;

	if (SelectedIcon != ClickedIcon && true == IsValid(SelectedIcon))
		SelectedIcon->SetSelected(false);

	ClickedIcon->SetSelected(true);

	SelectedIcon = ClickedIcon;

	if (false == CustomizeViewModel.IsValid())
	{
		return;
	}

	switch (MenuType)
	{
	case EDC1CustomizeMenu::Head:
		CustomizeViewModel->SetHead(ClickedIcon->GetPartTag());
		break;
	case EDC1CustomizeMenu::Hair:
		CustomizeViewModel->SetHair(ClickedIcon->GetPartTag());
		break;
	case EDC1CustomizeMenu::Eye:
		CustomizeViewModel->SetEye(ClickedIcon->GetPartTag());
		break;
	}
}

void UDC1CustomizePanel::HandleColorChanged(FLinearColor InColor)
{
	if (true == IsValid(ColorButton))
	{
		ColorButton->SetPreviewColor(InColor);
	}

	ApplyColorToViewModel(InColor);
}

void UDC1CustomizePanel::HandleColorPickClosed(bool bConfirmed)
{
	if (false == bConfirmed)
	{
		HandleColorChanged(ColorBeforeOpen);

		if (true == IsValid(ColorWheel))
		{
			ColorWheel->SetSeedColor(ColorBeforeOpen);
		}
	}

	if (true == IsValid(ColorWheel))
	{
		ColorWheel->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDC1CustomizePanel::HandleColorButtonClicked()
{
	if (false == IsValid(ColorWheel))
	{
		return;
	}

	if (true == CustomizeViewModel.IsValid())
	{
		ColorBeforeOpen = GetColorFromData(CustomizeViewModel->GetPendingAppearance());
	}

	ColorWheel->SetSeedColor(ColorBeforeOpen);
	ColorWheel->SetVisibility(ESlateVisibility::Visible);
}

FGameplayTag UDC1CustomizePanel::MakePartTag(int32 Index)
{
	const TCHAR* PartCategoryName = TEXT("Head");
	switch (MenuType)
	{
	case EDC1CustomizeMenu::Hair:
		PartCategoryName = TEXT("Hair");
		break;
	case EDC1CustomizeMenu::Eye:
		PartCategoryName = TEXT("Eye");
		break;
	default:
		break;
	}

	const FString TagString = FString::Printf(TEXT("Appearance.%s.%02d"), PartCategoryName, Index + 1);

	return FGameplayTag::RequestGameplayTag(FName(*TagString), false);
}

FLinearColor UDC1CustomizePanel::GetColorFromData(const FDC1AppearanceData& Data) const
{
	switch (MenuType)
	{
	case EDC1CustomizeMenu::Head:
		return Data.SkinColor;
	case EDC1CustomizeMenu::Hair:
		return Data.HairColor;
	case EDC1CustomizeMenu::Eye:
		return Data.EyeColor;
	default:
		return FLinearColor::White;
	}
}

void UDC1CustomizePanel::ApplyColorToViewModel(const FLinearColor& InColor)
{
	if (false == CustomizeViewModel.IsValid())
	{
		return;
	}

	switch (MenuType)
	{
	case EDC1CustomizeMenu::Head:
		CustomizeViewModel->SetSkinColor(InColor);
		break;
	case EDC1CustomizeMenu::Hair:
		CustomizeViewModel->SetHairColor(InColor);
		break;
	case EDC1CustomizeMenu::Eye:
		CustomizeViewModel->SetEyeColor(InColor);
		break;
	}
}
