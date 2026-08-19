// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildTabWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "UI/Tab/Build/DC1BuildWidget.h"


#include "Subsystem/DC1UIManagerSubsystem.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Data/DataTable/DC1DataTableTypes.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1TechTreeComponent.h"

void UDC1BuildTabWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		return;
	}

	TechComp->OnTechUnlocked.AddUObject(this, &ThisClass::OnTechUnlocked);
}

void UDC1BuildTabWidget::ButtonInitialize()
{
	UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		return;
	}

	TArray<const FDC1BuildRow*>	BuildRows;

	if (false == FindBuildData(BuildRows))
	{
		return;
	}

	for (const TPair<FGameplayTag, int32>& Pair : TabToSwitchIndex)
	{
		UDC1BuildWidget* BuildWidget = GetTabWidget<UDC1BuildWidget>(Pair.Key);
		if (false == ::IsValid(BuildWidget))
		{
			continue;
		}

		BuildWidget->InitBuild(BuildRows, TechComp);

		UDC1TabHeaderButton* TabButton = GetTabButton(Pair.Key);
		if (false == ::IsValid(TabButton))
		{
			continue;
		}

		bool HasUnlock = TechComp->HasUnlockedTag(Pair.Key);

		EDC1TabButtonState ButtonState = HasUnlock ? EDC1TabButtonState::Inactive : EDC1TabButtonState::Disable;
		
		TabButton->SetState(ButtonState);
	}
}

void UDC1BuildTabWidget::CloseWidget(const FKey& InKey)
{
	if (false == BuildCloseKeys.Keys.Contains(InKey))
	{
		return;
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Failed to Find UIManager"));
		return;
	}

	UIManager->PopScreen();
}

UDC1TechTreeComponent* UDC1BuildTabWidget::GetTechTreeComponent()
{
	ADC1PlayerState* PS = GetOwningPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PS))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Can't Find PS"));
		return nullptr;
	}

	UDC1TechTreeComponent* TechComp = PS->GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Can't Find TechComp"));
		return nullptr;
	}

	return TechComp;
}

bool UDC1BuildTabWidget::FindBuildData(TArray<const FDC1BuildRow*>& OutBuildRows)
{
	const UDC1DataTableData* DataTable = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);
	if (false == ::IsValid(DataTable))
	{
		return false;
	}

	// Find BuildData
	const UDataTable* BuildData = DataTable->GetDataTable(DC1DataTableTags::DataTable_UI_Build);
	if (false == ::IsValid(BuildData))
	{
		return false;
	}

	TArray<FDC1BuildRow*> AllBuildRows;
	BuildData->GetAllRows<FDC1BuildRow>(TEXT("TechTree"), AllBuildRows);
	if (true == AllBuildRows.IsEmpty())
	{
		return false;
	}

	for (const FDC1BuildRow* BuildRow : AllBuildRows)
	{
		const FGameplayTag& BuildTag = BuildRow->BuildTag;
		if (true == BuildTag.IsValid())
		{
			OutBuildRows.AddUnique(BuildRow);
		}
	}

	OutBuildRows.Sort([](const FDC1BuildRow& Src, const FDC1BuildRow& Dst)
		{
			return Src.SortOrder < Dst.SortOrder;
		});

	return OutBuildRows.Num() > 0;
}

void UDC1BuildTabWidget::OnTechUnlocked(const FGameplayTag& InUnlockedTag)
{
	if (false == InUnlockedTag.MatchesTag(DC1TechTags::Build))
	{
		return;
	}

	const FGameplayTag& RowTag = InUnlockedTag.RequestDirectParent();
	const FGameplayTag& TabTag = RowTag.RequestDirectParent();
	
	if (false == TabTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Invalid TabTag : %s"), *InUnlockedTag.ToString());
		return;
	}

	UDC1BuildWidget* BuildWidget = GetTabWidget<UDC1BuildWidget>(TabTag);
	if (false == ::IsValid(BuildWidget))
	{
		return;
	}

	if (false == BuildWidget->VisibleSlot(RowTag, InUnlockedTag))
	{
		return;
	}

	EnableTab(TabTag);
}