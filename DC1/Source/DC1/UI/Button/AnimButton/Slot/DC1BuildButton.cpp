// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Button/AnimButton/Slot/DC1BuildButton.h"

#include "Components/Image.h"

void UDC1BuildButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (true == ::IsValid(IconBackImage))
	{
		IconBackImage->SetBrush(UnHoverIconBackImageBrush);
	}

	if (true == ::IsValid(ButtonImage))
	{
		ButtonImage->SetBrush(UnHoverButtonImageBrush);
	}
}

void UDC1BuildButton::HandleHovered()
{
	Super::HandleHovered();

	if (true == ::IsValid(IconBackImage))
	{
		IconBackImage->SetBrush(HoverIconBackImageBrush);
	}

	if (true == ::IsValid(ButtonImage))
	{
		ButtonImage->SetBrush(HoverButtonImageBrush);
	}

}

void UDC1BuildButton::HandleUnhovered()
{
	Super::HandleUnhovered();

	if (true == ::IsValid(IconBackImage))
	{
		IconBackImage->SetBrush(UnHoverIconBackImageBrush);
	}

	if (true == ::IsValid(ButtonImage))
	{
		ButtonImage->SetBrush(UnHoverButtonImageBrush);
	}
}

void UDC1BuildButton::InitBuildButton(const FDC1BuildRow& InBuildRow, const FDC1TechRow& InTechRow)
{
	if (true == InBuildRow.BuildTag.IsValid())
	{
		BuildTag = InBuildRow.BuildTag;
	}
	
	SortOrder = InBuildRow.SortOrder;
	
	CachedCosts = InBuildRow.BuildCosts;

	if (true == ::IsValid(BuildIconImage) && false == InTechRow.Icon.IsNull())
	{
		BuildIconImage->SetBrushFromSoftTexture(InTechRow.Icon);
	}

	SetVisibility(ESlateVisibility::Collapsed);

	ApplyState();
}

void UDC1BuildButton::SetState(bool InCanBuild)
{
	if (CanBuild == InCanBuild)
	{
		return;
	}

	CanBuild = InCanBuild;

	ApplyState();
}

void UDC1BuildButton::ApplyState()
{
	if (true == ::IsValid(InactiveHoverImage))
	{
		float ImageOpacity = CanBuild ? 0.f : InactiveHoverImageOpacity;
		InactiveHoverImage->SetOpacity(ImageOpacity);
	}
}
