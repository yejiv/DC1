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

	// 배치 컴포넌트는 PlayerController에 존재
	UDC1BuildPlacementComponent* PlacementComponent = nullptr;
	ADC1PlayerController* PC = Cast<ADC1PlayerController>(InPlayerState->GetPlayerController());
	if (true == ::IsValid(PC))
	{
		PlacementComponent = PC->GetBuildPlacementComponent();
	}

	CacheBuildRows();
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

	OnBuildStateChanged.Broadcast();
}

bool UDC1BuildViewModel::IsTabUnlocked(const FGameplayTag& InTabTag) const
{
	return ::IsValid(TechComp) ? TechComp->HasUnlockedTag(InTabTag) : false;
}

bool UDC1BuildViewModel::IsBuildUnlocked(const FGameplayTag& InBuildTag) const
{
	if (false == ::IsValid(TechComp))
	{
		return false;
	}

	const FGameplayTag TechTag = TechTagByBuildTag.FindRef(InBuildTag);
	if (false == TechTag.IsValid())
	{
		return false;
	}

	return TechComp->IsTechUnlocked(TechTag);
}

void UDC1BuildViewModel::GetBuildTagsByTechTag(const FGameplayTag& InTechTag, TArray<FGameplayTag>& OutBuildTags) const
{
	OutBuildTags.Reset();

	BuildTagsByTechTag.MultiFind(InTechTag, OutBuildTags);
}

bool UDC1BuildViewModel::RequestStartPlacement(const FGameplayTag& InBuildTag)
{
	return ::IsValid(PlacementComp) ? PlacementComp->StartPlacement(InBuildTag) : false;
}

bool UDC1BuildViewModel::RequestStartDismantle()
{
	return ::IsValid(PlacementComp) ? PlacementComp->StartDismantle() : false;
}

void UDC1BuildViewModel::CacheBuildRows()
{
	SortedBuildRows.Reset();

	TechTagByBuildTag.Reset();
	BuildTagsByTechTag.Reset();

	TArray<FDC1BuildRow> AllBuildRows;

	if (false == DC1DataTableHelper::GetAllRows<FDC1BuildRow>(DC1DataTableTags::DataTable_UI_Build, AllBuildRows))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildViewModel :: Build DataTable is NULL"));
		return;
	}

	for (const FDC1BuildRow& BuildRow : AllBuildRows)
	{
		const FGameplayTag& BuildTag = BuildRow.BuildTag;
		const FGameplayTag& TechTag = BuildRow.RequiredTechTag;

		if (false == BuildTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildViewModel :: Build Tag is Invalid"));
			continue;
		}

		if (false == TechTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1BuildViewModel :: Build Data Tech Tag is Invalid"));
			continue;
		}

		SortedBuildRows.Add(BuildRow);

		BuildTagsByTechTag.Add(TechTag, BuildTag);
		TechTagByBuildTag.Add(BuildTag, TechTag);
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
	TArray<FGameplayTag> BuildTags;
	GetBuildTagsByTechTag(InTechTag, BuildTags);

	for (const FGameplayTag& BuildTag : BuildTags)
	{
		OnBuildUnlocked.Broadcast(BuildTag);
	}

}