// Fill out your copyright notice in the Description page of Project Settings.
// 디버그 콘솔 명령 ( ` 콘솔 ). Shipping 빌드에서는 등록되지 않는다

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "HAL/IConsoleManager.h"
#include "Misc/DelayedAutoRegister.h"
#include "ConsoleSettings.h"
#include "Engine/Console.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Data/Pal/DC1PalTypes.h"
#include "Define/DC1CraftHelper.h"
#include "Define/DC1Helper.h"
#include "Define/DC1ItemHelper.h"
#include "System/DC1AssetManager.h"
#include "Subsystem/DC1ExperienceSubsystem.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/DC1GamePlayTags.h"

namespace DC1DebugCommands
{
	// 명령을 친 로컬 플레이어의 서버 쪽 PlayerState
	// PIE는 서버 · 클라 월드가 한 프로세스라 클라 콘솔에서도 같은 PlayerId의 서버 PS를 찾아 바꾼다 ( 별도 프로세스 클라에서는 동작 안 함 )
	ADC1PlayerState* FindAuthorityPlayerState(UWorld* InWorld, FOutputDevice& Ar)
	{
		const UGameInstance* GameInstance = (true == ::IsValid(InWorld)) ? InWorld->GetGameInstance() : nullptr;
		const APlayerController* LocalPC = (nullptr != GameInstance) ? GameInstance->GetFirstLocalPlayerController() : nullptr;
		ADC1PlayerState* LocalPS = (nullptr != LocalPC) ? LocalPC->GetPlayerState<ADC1PlayerState>() : nullptr;
		if (false == ::IsValid(LocalPS))
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] No local player state."));
			return nullptr;
		}

		if (NM_Client != InWorld->GetNetMode())
		{
			return LocalPS;
		}

		const int32 PlayerId = LocalPS->GetPlayerId();

		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (EWorldType::PIE != Context.WorldType || false == ::IsValid(World) || NM_Client == World->GetNetMode())
			{
				continue;
			}

			const AGameStateBase* GameState = World->GetGameState();
			if (false == ::IsValid(GameState))
			{
				continue;
			}

			for (const TObjectPtr<APlayerState>& PS : GameState->PlayerArray)
			{
				ADC1PlayerState* DC1PS = Cast<ADC1PlayerState>(PS);
				if (true == ::IsValid(DC1PS) && PlayerId == DC1PS->GetPlayerId())
				{
					return DC1PS;
				}
			}
		}

		Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Server player state not found (PlayerId %d). Only works in PIE."), PlayerId);
		return nullptr;
	}

	// DC1.AddItem <ItemID> [Count]
	void AddItem(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& Ar)
	{
		if (InArgs.Num() < 1)
		{
			Ar.Logf(TEXT("Usage: DC1.AddItem <ItemID> [Count=1]   ex) DC1.AddItem 6101 30"));
			return;
		}

		const FName ItemID(*InArgs[0]);
		const int32 Count = (InArgs.Num() > 1) ? FCString::Atoi(*InArgs[1]) : 1;
		if (Count <= 0)
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Count must be > 0."));
			return;
		}

		if (nullptr == DC1ItemHelper::FindItemRow(ItemID))
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Unknown ItemID [%s]."), *ItemID.ToString());
			return;
		}

		ADC1PlayerState* PS = FindAuthorityPlayerState(InWorld, Ar);
		UDC1PlayerInventoryComponent* Inventory = (nullptr != PS) ? PS->GetInventoryComponent() : nullptr;
		if (false == ::IsValid(Inventory))
		{
			return;
		}

		if (false == Inventory->AddItem(ItemID, Count))
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] AddItem failed [%s] x%d (inventory full?)."), *ItemID.ToString(), Count);
			return;
		}

		Ar.Logf(TEXT("[DC1Debug] Added [%s] x%d."), *ItemID.ToString(), Count);
	}

	// DC1.AddAllItems - Item / ValuableItem 테이블의 모든 아이템. MaxStack 1이면 1개, 그 외는 최대 1000개
	void AddAllItems(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& Ar)
	{
		ADC1PlayerState* PS = FindAuthorityPlayerState(InWorld, Ar);
		UDC1PlayerInventoryComponent* Inventory = (nullptr != PS) ? PS->GetInventoryComponent() : nullptr;
		if (false == ::IsValid(Inventory))
		{
			return;
		}

		int32 AddedKinds = 0;
		int32 FailedKinds = 0;

		const FGameplayTag TableTags[] = { DC1DataTableTags::DataTable_Item, DC1DataTableTags::DataTable_ValuableItem };

		for (const FGameplayTag& TableTag : TableTags)
		{
			TArray<FDC1ItemRow> Rows;
			if (false == DC1DataTableHelper::GetAllRows<FDC1ItemRow>(TableTag, Rows))
			{
				continue;
			}

			for (const FDC1ItemRow& Row : Rows)
			{
				if (true == Row.ItemID.IsNone())
				{
					continue;
				}

				const int32 Count = (Row.MaxStack <= 1) ? 1 : FMath::Min(Row.MaxStack, 1000);
				const int32 RemainCount = Inventory->AddItemPartial(Row.ItemID, Count);
				if (RemainCount >= Count)
				{
					++FailedKinds;
					continue;
				}

				++AddedKinds;
			}
		}

		Ar.Logf(TEXT("[DC1Debug] AddAllItems: added %d kinds, failed %d kinds (inventory full?)."), AddedKinds, FailedKinds);
	}

	// DC1.AddPal <Species> [Count]
	void AddPal(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& Ar)
	{
		if (InArgs.Num() < 1)
		{
			Ar.Logf(TEXT("Usage: DC1.AddPal <Species> [Count=1]   ex) DC1.AddPal Alpaca 5  /  DC1.AddPal Pal.Species.Alpaca"));
			return;
		}

		const int32 Count = (InArgs.Num() > 1) ? FCString::Atoi(*InArgs[1]) : 1;
		if (Count <= 0)
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Count must be > 0."));
			return;
		}

		const FString TagName = (true == InArgs[0].Contains(TEXT("."))) ? InArgs[0] : FString(TEXT("Pal.Species.")) + InArgs[0];
		const FGameplayTag SpeciesTag = FGameplayTag::RequestGameplayTag(FName(*TagName), false);
		if (false == SpeciesTag.IsValid())
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Unknown species tag [%s]."), *TagName);
			return;
		}

		const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
		if (false == ::IsValid(Registry) || true == Registry->FindPalDataByTag(SpeciesTag).IsNull())
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] No pal data for [%s]."), *TagName);
			return;
		}

		ADC1PlayerState* PS = FindAuthorityPlayerState(InWorld, Ar);
		UDC1PalPartyComponent* Party = (nullptr != PS) ? PS->GetPalPartyComponent() : nullptr;
		if (false == ::IsValid(Party))
		{
			return;
		}

		int32 Added = 0;

		for (int32 Index = 0; Index < Count; ++Index)
		{
			FDC1PalInstance Pal;
			Pal.SpeciesTag = SpeciesTag;
			Pal.PalID = FGuid::NewGuid();
			Pal.Level = 1;
			Pal.TalentHealth = FMath::RandRange(0, 100);
			Pal.TalentAttack = FMath::RandRange(0, 100);
			Pal.TalentDefense = FMath::RandRange(0, 100);
			Pal.Gender = (true == FMath::RandBool()) ? EDC1PalGender::Male : EDC1PalGender::Female;
			Registry->RollPassives(Pal.PassiveTags);
			Pal.bPassivesRolled = true;
			Pal.CurrentHealth = UDC1PalPartyComponent::GetMaxHealthOf(Pal);

			if (false == Party->ReceivePal(Pal))
			{
				Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Box and party are full."));
				break;
			}

			++Added;
		}

		Ar.Logf(TEXT("[DC1Debug] Added pal [%s] Lv.1 x%d / %d (box first, then party)."), *TagName, Added, Count);
	}

	// DC1.AddExp <Amount> - 실제 경험치 경로 ( ExperienceSubsystem -> StatComponent::GainExperience ) 로 지급. 레벨업 소리 · 이펙트 · 테크 포인트까지 그대로 탄다
	void AddExp(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& Ar)
	{
		if (InArgs.Num() < 1)
		{
			Ar.Logf(TEXT("Usage: DC1.AddExp <Amount>   ex) DC1.AddExp 1000"));
			return;
		}

		const int32 Amount = FCString::Atoi(*InArgs[0]);
		if (Amount <= 0)
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Amount must be > 0."));
			return;
		}

		ADC1PlayerState* PS = FindAuthorityPlayerState(InWorld, Ar);
		UWorld* ServerWorld = (nullptr != PS) ? PS->GetWorld() : nullptr;
		UDC1ExperienceSubsystem* ExpSubsystem = (nullptr != ServerWorld) ? ServerWorld->GetSubsystem<UDC1ExperienceSubsystem>() : nullptr;
		if (nullptr == ExpSubsystem)
		{
			Ar.Logf(ELogVerbosity::Error, TEXT("[DC1Debug] Experience subsystem not found."));
			return;
		}

		ExpSubsystem->Grant(PS, DC1PlayerTags::Event_Exp_Player, Amount);
		Ar.Logf(TEXT("[DC1Debug] Granted %d exp."), Amount);
	}

	void CraftCount(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& Ar)
	{
		if (InArgs.Num() < 1)
		{
			Ar.Logf(TEXT("Usage: DC1.CraftCount <ItemID>   ex) DC1.CraftCount 6101"));
			return;
		}

		const FName ItemID(*InArgs[0]);

		const UGameInstance* GameInstance = (true == ::IsValid(InWorld)) ? InWorld->GetGameInstance() : nullptr;
		const APlayerController* LocalPC = (nullptr != GameInstance) ? GameInstance->GetFirstLocalPlayerController() : nullptr;
		const ADC1PlayerState* LocalPS = (nullptr != LocalPC) ? LocalPC->GetPlayerState<ADC1PlayerState>() : nullptr;
		const ADC1PlayerState* ServerPS = FindAuthorityPlayerState(InWorld, Ar);
		if (false == ::IsValid(LocalPS) || nullptr == ServerPS)
		{
			return;
		}

		const UDC1PlayerInventoryComponent* ServerInventory = ServerPS->GetInventoryComponent();
		const int32 ServerTotal = DC1CraftHelper::GetCraftItemCount(ServerPS, ItemID);
		const int32 ServerPlayer = (true == ::IsValid(ServerInventory)) ? ServerInventory->GetItemCount(ItemID) : 0;

		Ar.Logf(TEXT("[DC1Debug] CraftCount [%s] local %d / server %d (player %d + base chest %d)."),
			*ItemID.ToString(), DC1CraftHelper::GetCraftItemCount(LocalPS, ItemID), ServerTotal, ServerPlayer, ServerTotal - ServerPlayer);
	}

	void RemoveCraft(const TArray<FString>& InArgs, UWorld* InWorld, FOutputDevice& Ar)
	{
		if (InArgs.Num() < 2)
		{
			Ar.Logf(TEXT("Usage: DC1.RemoveCraft <ItemID> <Count>   ex) DC1.RemoveCraft 6101 5"));
			return;
		}

		const FName ItemID(*InArgs[0]);
		const int32 Count = FCString::Atoi(*InArgs[1]);

		ADC1PlayerState* PS = FindAuthorityPlayerState(InWorld, Ar);
		if (nullptr == PS)
		{
			return;
		}

		const bool IsRemoved = DC1CraftHelper::RemoveCraftItem(PS, ItemID, Count);

		const UDC1PlayerInventoryComponent* Inventory = PS->GetInventoryComponent();
		const int32 Total = DC1CraftHelper::GetCraftItemCount(PS, ItemID);
		const int32 Player = (true == ::IsValid(Inventory)) ? Inventory->GetItemCount(ItemID) : 0;

		Ar.Logf(TEXT("[DC1Debug] RemoveCraft [%s] x%d %s -> player %d + base chest %d."),
			*ItemID.ToString(), Count, (true == IsRemoved) ? TEXT("OK") : TEXT("FAILED"), Player, Total - Player);
	}

	void AppendItemRows(const FGameplayTag& InTableTag, TArray<FAutoCompleteCommand>& OutList)
	{
		TArray<FDC1ItemRow> Rows;
		if (false == DC1DataTableHelper::GetAllRows<FDC1ItemRow>(InTableTag, Rows))
		{
			return;
		}

		for (const FDC1ItemRow& Row : Rows)
		{
			if (true == Row.ItemID.IsNone())
			{
				continue;
			}

			FAutoCompleteCommand Entry;
			Entry.Command = FString::Printf(TEXT("DC1.AddItem %s"), *Row.ItemID.ToString());
			Entry.Desc = Row.ItemName.ToString();
			OutList.Add(MoveTemp(Entry));
		}
	}

	void AddItemAutoCompleteEntries(TArray<FAutoCompleteCommand>& OutList)
	{
		AppendItemRows(DC1DataTableTags::DataTable_Item, OutList);
		AppendItemRows(DC1DataTableTags::DataTable_ValuableItem, OutList);
	}

	FAutoConsoleCommand AddItemCommand(
		TEXT("DC1.AddItem"),
		TEXT("DC1.AddItem <ItemID> [Count=1] - add item to local player's inventory"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&AddItem));

	FAutoConsoleCommand AddAllItemsCommand(
		TEXT("DC1.AddAllItems"),
		TEXT("DC1.AddAllItems - add every item (MaxStack 1 -> 1, else up to 1000) to local player's inventory"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&AddAllItems));

	FAutoConsoleCommand AddPalCommand(
		TEXT("DC1.AddPal"),
		TEXT("DC1.AddPal <Species> [Count=1] - add Lv.1 pals to local player's box (or party)"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&AddPal));

	FAutoConsoleCommand AddExpCommand(
		TEXT("DC1.AddExp"),
		TEXT("DC1.AddExp <Amount> - grant player exp (level up sound / FX / tech points)"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&AddExp));

	FAutoConsoleCommand CraftCountCommand(
		TEXT("DC1.CraftCount"),
		TEXT("DC1.CraftCount <ItemID> - craft material count (player + base chests), local and server"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&CraftCount));

	FAutoConsoleCommand RemoveCraftCommand(
		TEXT("DC1.RemoveCraft"),
		TEXT("DC1.RemoveCraft <ItemID> <Count> - remove craft material (player first, then base chests)"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&RemoveCraft));

	FDelayedAutoRegisterHelper ItemAutoCompleteRegister(EDelayedRegisterRunPhase::EndOfEngineInit, []()
	{
		UConsole::RegisterConsoleAutoCompleteEntries.AddStatic(&AddItemAutoCompleteEntries);
	});
}

#endif
