// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Chest/DC1ChestActor.h"

#include "Component/Inventory/Chest/DC1ChestInventoryComponent.h"
#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1Player.h"
#include "Define/DC1Helper.h"
#include "UI/Tab/Inventory/Chest/DC1ChestLootScreenWidget.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "GameMode/DC1GameState.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Component/Base/DC1BaseRegistryComponent.h"

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

	//AddDefaultItems();
}

void ADC1ChestActor::AddDefaultItems()
{
	if (false == HasAuthority() || false == ::IsValid(ChestInventoryComp))
	{
		return;
	}

	const UDC1DataTableData* DataTableAsset = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
	if (false == ::IsValid(DataTableAsset))
	{
		return;
	}

	const UDataTable* ItemDataTable = DataTableAsset->GetDataTable(DC1DataTableTags::DataTable_Item);
	if (false == ::IsValid(ItemDataTable))
	{
		return;
	}

	TArray<FDC1ItemRow*> AllRows;
	ItemDataTable->GetAllRows<FDC1ItemRow>(TEXT("ChestDefaultItems"), AllRows);

	// 테스트용: 일반 아이템 전부를 1~100개 랜덤 (귀중품은 상자 보관 불가라 제외)
	for (const FDC1ItemRow* Row : AllRows)
	{
		if (nullptr == Row)
		{
			continue;
		}

		// AddItem은 MaxStack을 보지 않고 한 칸에 누적하므로 무기처럼 1개짜리는 1개만
		const int32 MaxCount = FMath::Clamp(Row->MaxStack, 1, 100);

		// 꺼내기 전용 상자(CanStoreItems == false)에는 TryStoreItem이 실패하므로 들어가지 않음
		ChestInventoryComp->TryStoreItem(Row->ItemID, FMath::RandRange(1, MaxCount));
	}
}

void ADC1ChestActor::OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	if (InKey != EDC1InteractKey::F)
	{
		return;
	}

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

	LootScreen->OpenChest(ChestInventoryComp);

	if (ADC1Player* Player = Cast<ADC1Player>(InInteractOwner))
	{
		Player->PlayWorldSound(DC1SoundHelper::PickRandom(OpenSounds), GetActorLocation());
	}
}

void ADC1ChestActor::OnPlaced(const APlayerState* InBuilder)
{
	Super::OnPlaced(InBuilder);

	OnBuildCompleted.AddUObject(this, &ThisClass::HandleBuildCompleted);
}

void ADC1ChestActor::HandleBuildCompleted()
{
	ADC1GameState* GS = ADC1GameState::Get(this);
	ADC1PalBoxActor* PalBox = (true == ::IsValid(GS)) ? GS->FindBaseAt(GetActorLocation()) : nullptr;
	UDC1BaseRegistryComponent* BaseRegistryComp = (true == ::IsValid(PalBox)) ? PalBox->GetRegistryComponent() : nullptr;
	
	if (true == ::IsValid(BaseRegistryComp))
	{
		BaseRegistryComp->RegisterChest(this);
	}
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
