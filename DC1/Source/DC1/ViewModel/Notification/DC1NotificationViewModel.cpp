// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Notification/DC1NotificationViewModel.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Data/Pal/DC1PalTypes.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/Texture2D.h"

void UDC1NotificationViewModel::PushPalCaptureNotice(const FDC1PalInstance& InPal)
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	const UDC1PalData* PalData = IsValid(Registry) ? Registry->FindPalDataByTag(InPal.SpeciesTag).LoadSynchronous() : nullptr;
	if (false == IsValid(PalData))
	{
		return;
	}

	FDC1PalCaptureNoticeInfo Info;

	// 닉네임 없으면 종 이름
	Info.Name = InPal.Nickname.IsEmpty() ? PalData->DisplayName : FText::FromString(InPal.Nickname);
	Info.Level = InPal.Level;
	Info.Icon = PalData->Icon.LoadSynchronous();

	for (const FGameplayTag& PassiveTag : InPal.PassiveTags)
	{
		const FDC1PalPassiveDef* PassiveDef = Registry->FindPassiveDef(PassiveTag);
		if (nullptr == PassiveDef)
		{
			continue;
		}

		FDC1PalBoxPassiveInfo& Passive = Info.Passives.AddDefaulted_GetRef();
		Passive.Name = PassiveDef->DisplayName;
		Passive.RankIcon = Registry->FindPassiveRankIcon(PassiveDef->Rank).LoadSynchronous();
	}

	OnPalCaptureNotice.Broadcast(Info);
}

void UDC1NotificationViewModel::PushItemAcquiredNotice(FName InItemID, int32 InCount)
{
	const FDC1ItemRow* ItemRow = DC1ItemHelper::FindItemRow(InItemID);
	if (nullptr == ItemRow)
	{
		return;
	}

	FDC1FeedNoticeInfo Info;
	Info.Icon = ItemRow->ItemIcon.LoadSynchronous();
	Info.Message = FText::Format(NSLOCTEXT("Notification", "ItemAcquired", "{0} x {1}"), ItemRow->ItemName, FText::AsNumber(InCount));

	OnFeedNotice.Broadcast(Info);
}

void UDC1NotificationViewModel::PushPalDefeatNotice(const FGameplayTag& InKillerSpecies, const FString& InKillerNickname, const FGameplayTag& InVictimSpecies)
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		return;
	}

	const UDC1PalData* VictimData = Registry->FindPalDataByTag(InVictimSpecies).LoadSynchronous();
	if (false == IsValid(VictimData))
	{
		return;
	}

	FDC1FeedNoticeInfo Info;

	// 플레이어가 쓰러뜨림: 주어 없이, 쓰러진 팰 아이콘
	if (false == InKillerSpecies.IsValid())
	{
		Info.Icon = VictimData->Icon.LoadSynchronous();
		Info.Message = FText::Format(NSLOCTEXT("Notification", "PalDefeatedByPlayer", "{0}을(를) 쓰러뜨렸다!"), VictimData->DisplayName);

		OnFeedNotice.Broadcast(Info);
		return;
	}

	// 팰이 쓰러뜨림: 쓰러뜨린 팰이 주어, 그 팰 아이콘
	const UDC1PalData* KillerData = Registry->FindPalDataByTag(InKillerSpecies).LoadSynchronous();
	if (false == IsValid(KillerData))
	{
		return;
	}

	// 닉네임 없으면 종 이름
	const FText KillerName = InKillerNickname.IsEmpty() ? KillerData->DisplayName : FText::FromString(InKillerNickname);

	Info.Icon = KillerData->Icon.LoadSynchronous();
	Info.Message = FText::Format(NSLOCTEXT("Notification", "PalDefeatedByPal", "{0}이(가) {1}을(를) 쓰러뜨렸다!"), KillerName, VictimData->DisplayName);

	OnFeedNotice.Broadcast(Info);
}
