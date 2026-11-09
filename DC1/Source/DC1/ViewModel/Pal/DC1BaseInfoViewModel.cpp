// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1BaseInfoViewModel.h"

#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalBaseComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "GameMode/DC1GameState.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

namespace DC1BaseInfoViewModel
{
	// 범위 확인 주기. 들어온 뒤 표시까지 이만큼 늦을 수 있다
	static constexpr float InBaseCheckInterval = 0.25f;
}

void UDC1BaseInfoViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PlayerState = InPlayerState;

	// 거점 칸 · 거점 팰 상태가 바뀌면 GameState 가 모든 파티에 알린다
	PalPartyComponent = InPlayerState->GetPalPartyComponent();
	if (true == PalPartyComponent.IsValid())
	{
		PalPartyComponent->OnPalBoxChanged.AddUObject(this, &ThisClass::Refresh);
	}

	if (UWorld* World = InPlayerState->GetWorld())
	{
		World->GetTimerManager().SetTimer(InBaseCheckTimer, FTimerDelegate::CreateUObject(this, &ThisClass::CheckInBase), DC1BaseInfoViewModel::InBaseCheckInterval, true);
	}

	CheckInBase();
	Refresh();
}

void UDC1BaseInfoViewModel::ClearSource()
{
	if (true == PlayerState.IsValid())
	{
		if (UWorld* World = PlayerState->GetWorld())
		{
			World->GetTimerManager().ClearTimer(InBaseCheckTimer);
		}
	}

	if (true == PalPartyComponent.IsValid())
	{
		PalPartyComponent->OnPalBoxChanged.RemoveAll(this);
	}

	PlayerState.Reset();
	PalPartyComponent.Reset();

	bInBase = false;
	PalCount = 0;
	Capacity = 0;
	Rows.Reset();
}

void UDC1BaseInfoViewModel::CheckInBase()
{
	// 탑승 중이면 탄 팰 위치 ( PlayerState 의 폰 )
	const APawn* Pawn = PlayerState.IsValid() ? PlayerState->GetPawn() : nullptr;
	const ADC1GameState* GS = ADC1GameState::Get(PlayerState.Get());
	const bool bNowInBase = (nullptr != Pawn) && (nullptr != GS) && (nullptr != GS->FindBaseAt(Pawn->GetActorLocation()));

	if (bNowInBase == bInBase)
		return;

	bInBase = bNowInBase;
	Refresh();
}

void UDC1BaseInfoViewModel::Refresh()
{
	PalCount = 0;
	Capacity = 0;
	Rows.Reset();

	const ADC1GameState* GS = ADC1GameState::Get(PlayerState.Get());
	const ADC1PalBoxActor* PalBox = (true == bInBase && nullptr != GS) ? GS->GetBasePalBox() : nullptr;
	const UDC1PalBaseComponent* BaseComp = (nullptr != PalBox) ? PalBox->GetPalBaseComponent() : nullptr;
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);

	if (nullptr != BaseComp && nullptr != Registry)
	{
		Capacity = BaseComp->GetBaseCapacity();

		for (const FDC1PalInstance& Pal : BaseComp->GetBasePals())
		{
			if (false == Pal.IsValidPal())
				continue;

			++PalCount;

			const UDC1PalData* PalData = Registry->FindPalDataByTag(Pal.SpeciesTag).LoadSynchronous();
			const EDC1BasePalActivity Activity = BaseComp->FindActivity(Pal.PalID);

			FDC1BaseInfoRow& Row = Rows.AddDefaulted_GetRef();
			Row.Icon = (nullptr != PalData) ? PalData->Icon.LoadSynchronous() : nullptr;
			Row.Condition = FindCondition(Pal, EDC1BasePalActivity::Sleeping == Activity);
			Row.bWorking = (EDC1BasePalActivity::Working == Activity);
		}
	}

	OnBaseInfoChanged.Broadcast();
}

EDC1BasePalCondition UDC1BaseInfoViewModel::FindCondition(const FDC1PalInstance& InPal, bool InSleeping) const
{
	// 기절 ( 체력 0 ) 은 회복될 때까지 안 나와 쉬는 것으로 친다
	if (true == InSleeping || InPal.CurrentHealth <= 0.f)
		return EDC1BasePalCondition::Resting;

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (nullptr == Registry)
		return EDC1BasePalCondition::Healthy;

	// -1 = 아직 안 정함 ( 가득 )
	const float Sanity = (InPal.CurrentSanity < 0.f) ? GetDefault<UDC1PalAttributeSet>()->GetMaxSanity() : InPal.CurrentSanity;
	if (Sanity < Registry->TiredSanityThreshold)
		return EDC1BasePalCondition::Tired;

	for (const FGameplayTag& PassiveTag : InPal.PassiveTags)
	{
		const FDC1PalPassiveDef* PassiveDef = Registry->FindPassiveDef(PassiveTag);
		if (nullptr != PassiveDef && PassiveDef->WorkSpeedMultiplier > 1.f)
			return EDC1BasePalCondition::Capable;
	}

	return EDC1BasePalCondition::Healthy;
}
