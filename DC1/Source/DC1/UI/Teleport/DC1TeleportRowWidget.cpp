// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Teleport/DC1TeleportRowWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UDC1TeleportRowWidget::SetEntry(int32 InRowIndex, const FDC1TeleportEntry& InEntry)
{
	RowIndex = InRowIndex;

	Text_Name->SetText(InEntry.Name);

	// Type에 따른 아이콘 교체
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
