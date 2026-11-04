// Fill out your copyright notice in the Description page of Project Settings.
// 디버그 콘솔 명령 ( ` 콘솔 ). Shipping 빌드에서는 등록되지 않는다

#include "CoreMinimal.h"

#if !UE_BUILD_SHIPPING

#include "HAL/IConsoleManager.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Data/Pal/DC1PalTypes.h"
#include "Define/DC1ItemHelper.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

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

	FAutoConsoleCommand AddItemCommand(
		TEXT("DC1.AddItem"),
		TEXT("DC1.AddItem <ItemID> [Count=1] - add item to local player's inventory"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&AddItem));

	FAutoConsoleCommand AddPalCommand(
		TEXT("DC1.AddPal"),
		TEXT("DC1.AddPal <Species> [Count=1] - add Lv.1 pals to local player's box (or party)"),
		FConsoleCommandWithWorldArgsAndOutputDeviceDelegate::CreateStatic(&AddPal));
}

#endif
