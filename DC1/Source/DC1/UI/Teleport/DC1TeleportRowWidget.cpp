// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Teleport/DC1TeleportRowWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

void UDC1TeleportRowWidget::SetEntry(int32 InRowIndex, const FDC1TeleportEntry& InEntry)
{
	RowIndex = InRowIndex;

	Text_Name->SetText(InEntry.Name);

	// Type에 따른 아이콘 교체
	if (nullptr == Image_Type)
		return;

	UTexture2D* Icon = nullptr;
	switch (InEntry.Type)
	{
	case EDC1TeleportPointType::Teleport:
		Icon = TeleportIcon;
		break;
	case EDC1TeleportPointType::Base:
		Icon = BaseIcon;
		break;
	default:
		break;
	}

	// 아이콘이 없으면 숨긴다 ( 이전 Entry 아이콘이 남지 않도록 )
	if (nullptr == Icon)
	{
		Image_Type->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	Image_Type->SetBrushFromTexture(Icon);
	Image_Type->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UDC1TeleportRowWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Select->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
}

void UDC1TeleportRowWidget::HandleClicked()
{
	OnRowClicked.Broadcast(RowIndex);
}
