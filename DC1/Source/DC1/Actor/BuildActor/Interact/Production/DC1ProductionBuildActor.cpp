// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"
#include "Net/UnrealNetwork.h"

ADC1ProductionBuildActor::ADC1ProductionBuildActor()
{
	// EmptyTag 채우기 - 제작, 채광, 벌목
	CompletedOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Production", "생산"), EDC1InteractInputType::HoldAbility, FGameplayTag::EmptyTag);

	// 비워두기
	CollectOptions.Emplace(EDC1InteractKey::F, NSLOCTEXT("Interact", "Collect", "획득"), EDC1InteractInputType::Press, FGameplayTag::EmptyTag);
}

void ADC1ProductionBuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ProductionProgress);
	DOREPLIFETIME(ThisClass, StoredCount);
}

void ADC1ProductionBuildActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	Super::GetInteractOptions(OutOptions);

	if (BuildState != EDC1BuildState::Completed || StoredCount <= 0)
	{
		return;
	}

	for (const FDC1InteractOption& CollectOption : CollectOptions)
	{
		FDC1InteractOption* ChangeOption = OutOptions.FindByPredicate([&CollectOption](const FDC1InteractOption& Option)
			{
				return Option.Key == CollectOption.Key;
			});
		if (nullptr != ChangeOption)
		{
			*ChangeOption = CollectOption;
		}
		else
		{
			OutOptions.Add(CollectOption);
		}
	}
}

bool ADC1ProductionBuildActor::CanAcceptWorker() const
{
	if (true == Super::CanAcceptWorker())
	{
		return true;
	}

	return ( BuildState == EDC1BuildState::Completed ) && ( StoredCount <= 0 ) && 
			( false == ProductItemID.IsNone() ) && ( RequiredWorkPerItem > 0.f );
}

void ADC1ProductionBuildActor::OnWorkTick(float InWork)
{
	if (BuildState != EDC1BuildState::Completed)
	{
		Super::OnWorkTick(InWork);
		return;
	}

	if (RequiredWorkPerItem <= 0.f || StoredCount >= MaxStoredCount)
	{
		return;
	}

	CurrentProductionWork += InWork;

	int32 NewCount = StoredCount;

	while (CurrentProductionWork >= RequiredWorkPerItem && NewCount < MaxStoredCount)
	{
		CurrentProductionWork -= RequiredWorkPerItem;
		++NewCount;
	}

	if (NewCount >= MaxStoredCount)
	{
		CurrentProductionWork = 0.f;
		ClearWorkers();
	}

	ProductionProgress = CurrentProductionWork / RequiredWorkPerItem;

	if (NewCount > StoredCount)
	{
		OnProductionChanged.Broadcast();
		return;
	}


	SetStoredCount(NewCount);
}

void ADC1ProductionBuildActor::OnRep_ProductionProgress()
{
	OnProductionChanged.Broadcast();
}

void ADC1ProductionBuildActor::OnRep_StoredCount(int32 OldCount)
{
	ApplyStoredCount(OldCount);
}

void ADC1ProductionBuildActor::SetStoredCount(int32 InNewCount)
{
	if (false == HasAuthority())
	{
		return;
	}

	int32 OldCount = StoredCount;

	StoredCount = FMath::Clamp(InNewCount, 0, MaxStoredCount);

	if (OldCount != StoredCount)
	{
		// Standalone 표시용
		ApplyStoredCount(OldCount);
	}
}

void ADC1ProductionBuildActor::ApplyStoredCount(int32 InOldCount)
{
	bool IsEmptyChanged = (InOldCount <= 0) != (StoredCount <= 0);
	bool IsReachedMax = (InOldCount < MaxStoredCount) && (StoredCount >= MaxStoredCount);

	if (IsEmptyChanged || IsReachedMax)
	{
		OnInteractOptionsChanged.Broadcast();
	}

	OnProductionChanged.Broadcast();
}
