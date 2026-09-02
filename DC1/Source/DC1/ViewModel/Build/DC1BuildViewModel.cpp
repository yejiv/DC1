// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModel/Build/DC1BuildViewModel.h"

#include "Define/DC1Helper.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1PlayerController.h"
#include "Component/Build/DC1BuildPlacementComponent.h"

void UDC1BuildViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (false == ::IsValid(InPlayerState))
	{
		return;
	}

	// 배치 컴포넌트는 PlayerController에 붙어 있다
	UDC1BuildPlacementComponent* PlacementComponent = nullptr;
	if (ADC1PlayerController* PC = Cast<ADC1PlayerController>(InPlayerState->GetPlayerController()))
	{
		PlacementComponent = PC->GetBuildPlacementComponent();
	}

	SetModels(InPlayerState->GetTechTreeComponent(), PlacementComponent);
}

void UDC1BuildViewModel::ClearSource()
{
	SetModels(nullptr, nullptr);
}

void UDC1BuildViewModel::SetModels(UDC1TechTreeComponent* InTechComp, UDC1BuildPlacementComponent* InPlacementComp)
{
	if (true == ::IsValid(TechComp))
	{
		TechComp->OnTechStateChange.RemoveAll(this);
		TechComp->OnTechUnlocked.RemoveAll(this);
	}

	TechComp = InTechComp;
	PlacementComp = InPlacementComp;

	if (true == ::IsValid(TechComp))
	{
		TechComp->OnTechStateChange.AddUObject(this, &ThisClass::HandleTechStateChanged);
		TechComp->OnTechUnlocked.AddUObject(this, &ThisClass::HandleTechUnlocked);
	}

	CacheBuildRows();

	OnBuildStateChanged.Broadcast();
}

const FDC1TechRow* UDC1BuildViewModel::FindTechRow(const FGameplayTag& InBuildTag) const
{
	return ::IsValid(TechComp) ? TechComp->FindTechRow(InBuildTag) : nullptr;
}

bool UDC1BuildViewModel::IsUnlocked(const FGameplayTag& InTechTag) const
{
	return ::IsValid(TechComp) ? TechComp->HasUnlockedTag(InTechTag) : false;
}

bool UDC1BuildViewModel::RequestStartPlacement(const FGameplayTag& InBuildTag)
{
	return ::IsValid(PlacementComp) ? PlacementComp->StartPlacement(InBuildTag) : false;
}

void UDC1BuildViewModel::CacheBuildRows()
{
	SortedBuildRows.Reset();

	TArray<FDC1BuildRow> AllBuildRows;

	if (false == DC1DataTableHelper::GetAllRows<FDC1BuildRow>(DC1DataTableTags::DataTable_UI_Build, AllBuildRows))
	{
		UE_LOG(LogTemp, Warning, TEXT("Build DataTable is NULL"));
		return;
	}

	for (const FDC1BuildRow& BuildRow : AllBuildRows)
	{
		const FGameplayTag& BuildTag = BuildRow.BuildTag;
		if (true == BuildTag.IsValid())
		{
			SortedBuildRows.Add(BuildRow);
		}
	}

	SortedBuildRows.Sort([](const FDC1BuildRow& Src, const FDC1BuildRow& Dst)
		{
			return Src.SortOrder < Dst.SortOrder;
		});
}

void UDC1BuildViewModel::HandleTechStateChanged()
{
	OnBuildStateChanged.Broadcast();
}

void UDC1BuildViewModel::HandleTechUnlocked(const FGameplayTag& InTechTag)
{
	OnBuildUnlocked.Broadcast(InTechTag);
}