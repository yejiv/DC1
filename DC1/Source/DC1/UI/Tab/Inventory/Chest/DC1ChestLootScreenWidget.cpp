// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/Chest/DC1ChestLootScreenWidget.h"
#include "UI/Tab/Inventory/Chest/DC1PlayerLootInventoryWidget.h"
#include "UI/Tab/Inventory/Chest/DC1ChestInventoryWidget.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Subsystem/DC1ViewModelSubsystem.h"


UDC1ChestLootScreenWidget::UDC1ChestLootScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeys.Keys = { EKeys::Escape, EKeys::F };
}

void UDC1ChestLootScreenWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 공유 ViewModel(VMSubsystem 소유)에 남은 상자 참조를 정리 - Esc가 아닌 다른 경로로 화면이
	// 닫히는 경우(예: BeginLoadingTransition의 전체 PopScreen)에도 항상 호출되도록 여기서 처리
	if (CachedViewModel)
	{
		CachedViewModel->SetTargetChest(nullptr);
	}
}

void UDC1ChestLootScreenWidget::OpenChest(UDC1ChestInventoryComponent* InChestComp)
{
	// PushScreen(NativeConstruct) 이후에 호출되므로 패널 주입 시점은 기존과 동일하게 유지
	if (false == ::IsValid(CachedViewModel))
	{
		ULocalPlayer* OwnerLocalPlayer = GetOwningLocalPlayer();
		if (false == ::IsValid(OwnerLocalPlayer))
		{
			return;
		}

		if (UDC1ViewModelSubsystem* ViewModelSubsystem = OwnerLocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
		{
			CachedViewModel = ViewModelSubsystem->GetViewModel<UDC1InventoryViewModel>();
		}
	}

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
	if (false == CloseKeys.Keys.Contains(InKey))
	{
		return;
	}

	PopScreen();
}
