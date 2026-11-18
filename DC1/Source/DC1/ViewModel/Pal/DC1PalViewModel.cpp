// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalViewModel.h"

#include "Character/Pal/DC1PalCharacter.h"
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
	UnbindDeployedPal();

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

	return PalPartyComponent->IsPalAlive(Index) || (false == PalPartyComponent->HasPalAt(Index));
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

	OutDisplayInfo.PalID = PalInfo->PalID;
	OutDisplayInfo.ElementIcon = Registry->FindElementIcon(PalData->Element).LoadSynchronous();
	OutDisplayInfo.ElementIcon2 = (EDC1PalElement::Neutral == PalData->Element2) ? nullptr : Registry->FindElementIcon(PalData->Element2).LoadSynchronous();
	OutDisplayInfo.Level = PalInfo->Level;
	OutDisplayInfo.Name = PalInfo->Nickname.IsEmpty() ? PalData->DisplayName : FText::FromString(PalInfo->Nickname);

	// OutDisplayInfo.HealthPercent = PalInfo->CurrentHealth / FMath::Max(0.1f, PalData->MaxHealth);
	// 최대 체력은 개체마다 다르다(레벨·개체값). 종 데이터를 직접 읽으면 어긋나므로 게터를 거친다.
	// 소환 중엔 Pals의 CurrentHealth가 회수 전까지 갱신되지 않으므로 소환 팰 체력을 반영하는 게터를 쓴다
	const float MaxHealth = PalPartyComponent->GetPalMaxHealth(PrevSlot);
	OutDisplayInfo.HealthPercent = PalPartyComponent->GetPalHealth(PrevSlot) / FMath::Max(0.1f, MaxHealth);
}

FGameplayTag UDC1PalViewModel::GetSpeciesTag(int32 InSlotIndex) const
{
	if (false == PalPartyComponent.IsValid())
		return FGameplayTag::EmptyTag;

	const FDC1PalInstance* PalInfo = PalPartyComponent->GetPalInfo(InSlotIndex);

	return (nullptr != PalInfo) ? PalInfo->SpeciesTag : FGameplayTag::EmptyTag;
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

	// 소환 / 회수 / 사망 모두 여기로 들어오므로 체력 구독 대상을 맞춘다
	BindDeployedPal();

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

void UDC1PalViewModel::BindDeployedPal()
{
	ADC1PalCharacter* DeployedPal = PalPartyComponent.IsValid() ? PalPartyComponent->GetDeployedPal() : nullptr;
	if (BoundDeployedPal.Get() == DeployedPal)
		return;

	UnbindDeployedPal();

	if (false == IsValid(DeployedPal))
		return;

	DeployedPal->OnPalHealthChanged.AddUObject(this, &ThisClass::HandleDeployedPalHealthChanged);
	BoundDeployedPal = DeployedPal;
}

void UDC1PalViewModel::UnbindDeployedPal()
{
	if (ADC1PalCharacter* OldPal = BoundDeployedPal.Get())
	{
		OldPal->OnPalHealthChanged.RemoveAll(this);
	}

	BoundDeployedPal.Reset();
}

void UDC1PalViewModel::HandleDeployedPalHealthChanged(float InCurrent, float InMax)
{
	OnInfoChanged.Broadcast();
}
