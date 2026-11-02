// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Notification/DC1NotificationViewModel.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Data/Pal/DC1PalTypes.h"
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
