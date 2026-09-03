// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/TechTree/DC1TechTreeWidget.h"

#include "UI/Tab/TechTree/DC1TechTreeRowWidget.h"
#include "UI/Button/AnimButton/Slot/DC1TechTreeButton.h"
#include "UI/Tab/TechTree/DC1TechTreePointWidget.h"

#include "Define/DC1Helper.h"

#include "Data/DataTable/DC1DataTableTypes.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Components/ScrollBox.h"

#include "ViewModel/TechTree/DC1TechTreeViewModel.h"
#include "Subsystem/DC1ViewModelSubsystem.h"

void UDC1TechTreeWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ULocalPlayer* OwnerLocalPlayer = GetOwningLocalPlayer();
	if (false == ::IsValid(OwnerLocalPlayer))
	{
		return;
	}

	// VM은 소스보다 먼저 존재한다. 늦게 오는 소스는 VM이 브로드캐스트로 알린다
	if (UDC1ViewModelSubsystem* ViewModelSubsystem = OwnerLocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		SetViewModel(ViewModelSubsystem->GetViewModel<UDC1TechTreeViewModel>());
	}
}

void UDC1TechTreeWidget::SetViewModel(UDC1TechTreeViewModel* InViewModel)
{
	if (CachedViewModel == InViewModel)
	{
		return;
	}

	if (true == ::IsValid(CachedViewModel))
	{
		CachedViewModel->OnTechStateChanged.RemoveAll(this);
		CachedViewModel->OnTechUnlocked.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (true == ::IsValid(CachedViewModel))
	{
		CachedViewModel->OnTechStateChanged.AddUObject(this, &ThisClass::RefreshAll);
		CachedViewModel->OnTechUnlocked.AddWeakLambda(this, [](const FGameplayTag& Tag)
			{
				UE_LOG(LogTemp, Warning, TEXT("Unlocked : %s "), *Tag.ToString());
			});
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1TechTreeWidget :: Can't Find TechTreeViewModel"));
		return;
	}

	RefreshAll();
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

void UDC1TechTreeWidget::UpdatePoint()
{
	int32 NumTechPoint = CachedViewModel->GetTechPoints();
	int32 NumAncientTechPoint = CachedViewModel->GetAncientTechPoints();

	TechPointWidget->SetPoint(NumTechPoint);
	AncientTechPointWidget->SetPoint(NumAncientTechPoint);
}

void UDC1TechTreeWidget::UpdateSlots()
{
	for (UDC1TechTreeButton* TechSlot : TechSlots)
	{
		const FGameplayTag& CurrentSlotTag = TechSlot->GetTechTag();
		TechSlot->SetState(CachedViewModel->GetTechState(CurrentSlotTag));
	}
}

void UDC1TechTreeWidget::InitTechTree()
{
	if (false == ::IsValid(TechTreeScrollBox) || false == ::IsValid(RowWidgetClass))
	{
		return;
	}
	
	TArray<FDC1TechRow> AllRows;
	if (false == DC1DataTableHelper::GetAllRows<FDC1TechRow>(DC1DataTableTags::DataTable_UI_TechTree, AllRows))
	{
		return;
	}

	TMap<int32, TArray<const FDC1TechRow>> RowsByLevel;

	for (const FDC1TechRow& Row : AllRows)
	{
		int32 Level = Row.RequiredLevel;
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

		for (const FDC1TechRow Row : RowsByLevel[AddLevel])
		{
			UDC1TechTreeButton* TechSlot = RowWidget->AddTechSlot(Row);
			if (true == ::IsValid(TechSlot) && true == ::IsValid(CachedViewModel))
			{
				const FGameplayTag& CurrentSlotTag = Row.TechTag;

				TechSlot->OnClickedDelegate.AddWeakLambda(this, [this, CurrentSlotTag]() {
					CachedViewModel->RequestUnlockTech(CurrentSlotTag);
					});

				TechSlot->SetState(CachedViewModel->GetTechState(CurrentSlotTag));

				TechSlots.Add(TechSlot);
			}
		}
	}
}

void UDC1TechTreeWidget::RefreshAll()
{
	if (false == ::IsValid(CachedViewModel) || false == CachedViewModel->HasModel())
	{
		return;
	}

	if (false == IsInit)
	{
		// Init TechTree Row + UpdateSlots + Init Slot State
		InitTechTree();
		InitRows(CachedViewModel->GetPlayerLevel());
		IsInit = true;
	}
	else
	{
		UpdateRows(CachedViewModel->GetPlayerLevel());
	}

	UpdatePoint();
	UpdateSlots();
}