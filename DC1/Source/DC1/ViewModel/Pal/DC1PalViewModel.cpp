// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/Texture2D.h"

void UDC1PalViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PalPartyComponent = InPlayerState->GetPalPartyComponent();

	if (false == PalPartyComponent.IsValid())
		return;

	Capacity = PalPartyComponent->GetCapacity();
	PrevSlot = PalPartyComponent->GetSelectedSlot();
	PalPartyComponent->OnPalSlotTranslated.AddUObject(this, &ThisClass::HandleSelectedSlotChanged);
	PalPartyComponent->OnPalSlotChanged.AddUObject(this, &ThisClass::HandlePalChanged);

	HandlePalChanged();
}

void UDC1PalViewModel::ClearSource()
{
	if (false == PalPartyComponent.IsValid())
		return;

	PalPartyComponent->OnPalSlotTranslated.RemoveAll(this);
	PalPartyComponent->OnPalSlotChanged.RemoveAll(this);
	PalPartyComponent.Reset();

	CachedPalIcons.Reset();
}

UTexture2D* UDC1PalViewModel::GetPalIconTexture(int32 Offset)
{
	if (0 == Capacity)
		return nullptr;

	int32 Index = (PrevSlot + Offset + Capacity) % FMath::Max(Capacity, 1);

	if (false == CachedPalIcons.IsValidIndex(Index))
		return nullptr;

	return CachedPalIcons[Index];
}

bool UDC1PalViewModel::GetPalAlive(int32 Offset)
{
	if (0 == Capacity)
		return true;

	if (false == PalPartyComponent.IsValid())
		return true;

	int32 Index = (PrevSlot + Offset + Capacity) % FMath::Max(Capacity, 1);

	return PalPartyComponent->IsPalAlive(Index);
}

void UDC1PalViewModel::GetPalInfo(FDC1PalDisplayInfo& OutDisplayInfo)
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
		return;

	const FDC1PalInstance* PalInfo = PalPartyComponent->GetPalInfo(PrevSlot);

	if (nullptr == PalInfo)
	{
		OutDisplayInfo.ElementIcon = nullptr;
		return;
	}

	const UDC1PalData* PalData = Registry->FindPalDataByTag(PalInfo->SpeciesTag).LoadSynchronous();
	if (false == IsValid(PalData))
		return;

	OutDisplayInfo.ElementIcon = Registry->FindElementIcon(PalData->Element).LoadSynchronous();
	OutDisplayInfo.Level = PalInfo->Level;
	OutDisplayInfo.Name = PalInfo->Nickname.IsEmpty() ? PalData->DisplayName : FText::FromString(PalInfo->Nickname);
	// 추후 PalInfo의 MaxHealth로 교체
	OutDisplayInfo.HealthPercent = PalInfo->CurrentHealth / FMath::Max(0.1f, PalData->MaxHealth);
}

bool UDC1PalViewModel::IsDeployed()
{
	if (false == PalPartyComponent.IsValid())
		return false;

	return PalPartyComponent->IsDeployed(PrevSlot);
}

void UDC1PalViewModel::HandleSelectedSlotChanged()
{
	if (false == PalPartyComponent.IsValid())
		return;

	int32 SelectedIndex = PalPartyComponent->GetSelectedSlot();

	int32 Move = (PrevSlot - SelectedIndex + Capacity) % FMath::Max(Capacity, 1);

	PrevSlot = SelectedIndex;

	EDC1PalSlotShift Shift = (Move == 1) ? EDC1PalSlotShift::Prev : ((Move == 0) ? EDC1PalSlotShift::None : EDC1PalSlotShift::Next);
	OnSlotShifted.Broadcast(Shift);
}

void UDC1PalViewModel::HandlePalChanged()
{
	if (false == PalPartyComponent.IsValid())
		return;

	// 인덱스 = 슬롯 번호. 빈 칸은 nullptr로 남겨야 오프셋 계산이 맞는다
	CachedPalIcons.Init(nullptr, Capacity);

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
		return;

	for (int32 i = 0; i < Capacity; ++i)
	{
		const FDC1PalInstance* PalInfo = PalPartyComponent->GetPalInfo(i);
		if (nullptr == PalInfo || false == PalInfo->IsValidPal())
			continue;

		const UDC1PalData* PalData = Registry->FindPalDataByTag(PalInfo->SpeciesTag).LoadSynchronous();
		if (false == IsValid(PalData))
			continue;

		CachedPalIcons[i] = PalData->Icon.LoadSynchronous();
	}
	
	OnInfoChanged.Broadcast();
}
