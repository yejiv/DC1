// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1ChestLootScreenWidget.h"
#include "UI/Tab/Inventory/DC1PlayerLootInventoryWidget.h"
#include "UI/Tab/Inventory/DC1ChestInventoryWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"


UDC1ChestLootScreenWidget::UDC1ChestLootScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1ChestLootScreenWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 공유 ViewModel(PlayerController 소유)에 남은 상자 참조를 정리 - Esc가 아닌 다른 경로로 화면이
	// 닫히는 경우(예: BeginLoadingTransition의 전체 PopScreen)에도 항상 호출되도록 여기서 처리
	if (CachedViewModel)
	{
		CachedViewModel->SetTargetChest(nullptr);
	}
}

void UDC1ChestLootScreenWidget::OpenChest(UDC1InventoryViewModel* InViewModel, UDC1ChestInventoryComponent* InChestComp)
{
	CachedViewModel = InViewModel;

	if (!CachedViewModel)
	{
		return;
	}

	if (PlayerPanel)
	{
		PlayerPanel->SetViewModel(CachedViewModel);
	}

	if (ChestPanel)
	{
		ChestPanel->SetViewModel(CachedViewModel);
		ChestPanel->OpenChest(InChestComp);
	}
}

void UDC1ChestLootScreenWidget::HandleInputKey(const FKey& InKey)
{
	if (InKey != CloseKey)
	{
		return;
	}

	PopScreen();
}
