// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Chest/DC1ChestActor.h"

#include "Component/Inventory/DC1ChestInventoryComponent.h"
#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerController.h"
#include "UI/Tab/Inventory/DC1ChestLootScreenWidget.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

ADC1ChestActor::ADC1ChestActor()
{
	ChestInventoryComp = CreateDefaultSubobject<UDC1ChestInventoryComponent>("ChestInventoryComp");
}

void ADC1ChestActor::BeginPlay()
{
	Super::BeginPlay();

	if (true == ::IsValid(ChestInventoryComp))
	{
		ChestInventoryComp->SetCapacityAndColumns(ChestCapacity, ChestColumns);
	}
}

void ADC1ChestActor::Interact(ADC1Character* InInteractOwner)
{
	if (false == ::IsValid(InInteractOwner) || false == ::IsValid(ChestInventoryComp))
	{
		return;
	}

	ADC1PlayerController* PC = Cast<ADC1PlayerController>(InInteractOwner->GetController());
	if (false == ::IsValid(PC))
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	UDC1ChestLootScreenWidget* LootScreen = Cast<UDC1ChestLootScreenWidget>(UIManager->PushScreen(DC1UITags::UI_Chest_Loot));
	if (false == ::IsValid(LootScreen))
	{
		return;
	}

	LootScreen->OpenChest(PC->GetInventoryViewModel(), ChestInventoryComp);
}

bool ADC1ChestActor::StoreItem(FName InItemID, int32 InCount)
{
	if (false == HasAuthority() || false == ::IsValid(ChestInventoryComp))
	{
		return false;
	}

	return ChestInventoryComp->TryStoreItem(InItemID, InCount);
}

bool ADC1ChestActor::TakeItem(int32 InSlotIndex, int32 InCount)
{
	if (false == HasAuthority() || false == ::IsValid(ChestInventoryComp))
	{
		return false;
	}

	return ChestInventoryComp->RemoveItem(InSlotIndex, InCount);
}
