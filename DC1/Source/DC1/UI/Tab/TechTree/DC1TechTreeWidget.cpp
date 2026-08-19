// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeWidget.h"

#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"
#include "UI/Button/AnimButton/Slot/DC1TechTreeButton.h"
#include "UI/Tab/TechTree/DC1TechTreePointWidget.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/UI/DC1UIData.h"
#include "Data/DA/DataTable/DC1DataTableData.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1TechTreeComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"

#include "Components/ScrollBox.h"

void UDC1TechTreeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
	if (true == ::IsValid(TechComp))
	{
		TechComp->OnTechStateChange.AddUObject(this, &ThisClass::OnTechStateChanged);

		TechComp->OnTechUnlocked.AddWeakLambda(this, [](const FGameplayTag& Tag)
			{
				UE_LOG(LogTemp, Warning, TEXT("Unlocked : %s "), *Tag.ToString());
			});
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeWidget :: Can't Find TechTreeComponent"));
		return;
	}

	// Init TechTree Row + Slot + Init Slot State
	InitTechTree();

	// Init Rows
	InitRows(TechComp->GetPlayerLevel());

	// Init Point
	UpdatePoint(TechComp);
}

UDC1TechTreeComponent* UDC1TechTreeWidget::GetTechTreeComponent()
{
	ADC1PlayerState* PS = GetOwningPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PS))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeWidget :: Can't Find PS"));
		return nullptr;
	}

	UDC1TechTreeComponent* TechComp = PS->GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeWidget :: Can't Find TechComp"));
		return nullptr;
	}

	return TechComp;
}

void UDC1TechTreeWidget::InitRows(int32 InNewLevel)
{
	for (UDC1TechTreeRowWidget* Row : TechRows)
	{
		if (false == ::IsValid(Row))
		{
			continue;
		}

		Row->SetState(InNewLevel);
	}

	CachedLevel = InNewLevel;
}

void UDC1TechTreeWidget::UpdateRows(int32 InNewLevel)
{
	int32 Start = FMath::Max(CachedLevel - 1, 0);
	int32 End = FMath::Min(InNewLevel, TechRows.Num());

	CachedLevel = InNewLevel;

	for (int32 i = Start; i < End; ++i)
	{
		UDC1TechTreeRowWidget* Row = TechRows[i];

		if (false == ::IsValid(Row))
		{
			continue;
		}

		Row->SetState(InNewLevel);
	}
}

void UDC1TechTreeWidget::UpdatePoint(const UDC1TechTreeComponent* InTechComp)
{
	int32 NumTechPoint = InTechComp->GetTechPoints();
	int32 NumAncientTechPoint = InTechComp->GetAncientTechPoints();

	TechPointWidget->SetPoint(NumTechPoint);
	AncientTechPointWidget->SetPoint(NumAncientTechPoint);
}

void UDC1TechTreeWidget::UpdateSlots(const UDC1TechTreeComponent* InTechComp)
{
	for (UDC1TechTreeButton* TechSlot : TechSlots)
	{
		const FGameplayTag& CurrentSlotTag = TechSlot->GetTechTag();
		TechSlot->SetState(InTechComp->GetTechState(CurrentSlotTag));
	}
}

void UDC1TechTreeWidget::InitTechTree()
{
	if (false == ::IsValid(TechTreeScrollBox) || false == ::IsValid(RowWidgetClass))
	{
		return;
	}
	const UDC1DataTableData* DataTable = UDC1AssetManager::GetAssetByTag<UDC1DataTableData>(DC1AssetTags::Asset_DataTableData);

	if (false == ::IsValid(DataTable))
	{
		return;
	}

	const UDataTable* TechTreeData = DataTable->GetDataTable(DC1DataTableTags::DataTable_UI_TechTree);
	if (false == ::IsValid(TechTreeData))
	{
		return;
	}
	
	TArray<FDC1TechRow*> AllRows;
	TechTreeData->GetAllRows<FDC1TechRow>(TEXT("BuildTechTree"), AllRows);
	if (true == AllRows.IsEmpty())
	{
		return;
	}

	TMap<int32, TArray<FDC1TechRow*>> RowsByLevel;

	for (FDC1TechRow* Row : AllRows)
	{
		int32 Level = Row->RequiredLevel;
		RowsByLevel.FindOrAdd(Level).Add(Row);
	}

	TArray<int32> Levels;
	RowsByLevel.GetKeys(Levels);
	Levels.Sort();

	for (int32 AddLevel : Levels)
	{
		UDC1TechTreeRowWidget* RowWidget = CreateWidget<UDC1TechTreeRowWidget>(this, RowWidgetClass);
		if (false == ::IsValid(RowWidget))
		{
			continue;
		}

		RowWidget->InitTier(AddLevel);
		TechRows.Add(RowWidget);

		TechTreeScrollBox->AddChild(RowWidget);

		for (const FDC1TechRow* Row : RowsByLevel[AddLevel])
		{
			UDC1TechTreeButton* TechSlot = RowWidget->AddTechSlot(*Row);
			if (true == ::IsValid(TechSlot))
			{

				UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
				if (true == ::IsValid(TechComp))
				{
					const FGameplayTag& CurrentSlotTag = Row->TechTag;

					TechSlot->OnClickedDelegate.AddWeakLambda(this, [this, CurrentSlotTag, TechComp]() {
							TechComp->Server_UnlockTech(CurrentSlotTag);
						});

					TechSlot->SetState(TechComp->GetTechState(CurrentSlotTag));
				}

				TechSlots.Add(TechSlot);
			}
		}
	}
}

void UDC1TechTreeWidget::OnTechStateChanged()
{
	UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeWidget :: Can't Find TechTreeComponent"));
		return;
	}

	UpdateSlots(TechComp);
	UpdatePoint(TechComp);

	int32 NewLevel = TechComp->GetPlayerLevel();
	if (CachedLevel != NewLevel)
	{
		UpdateRows(NewLevel);
	}
}
