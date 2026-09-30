// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxScreenWidget.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/Pal/PalBox/DC1PalBoxCursorWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxDetailWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxSlotWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxStorageSlotWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxStorageWidget.h"
#include "ViewModel/Pal/DC1PalBoxViewModel.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/PanelWidget.h"

void UDC1PalBoxScreenWidget::HandleInputKey(const FKey& InKey)
{
	if (InKey != CloseKey)
	{
		return;
	}

	PopScreen();
}

void UDC1PalBoxScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1PalBoxScreenWidget::NativeDestruct()
{
	if (true == PalBoxViewModel.IsValid())
	{
		PalBoxViewModel->CancelPick();
	}

	UnbindViewModel();

	if (nullptr != CursorWidget)
	{
		CursorWidget->RemoveFromParent();
	}

	bPressPending = false;

	Super::NativeDestruct();
}

void UDC1PalBoxScreenWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	// 열 때는 NoData
	HoveredSlot = INDEX_NONE;
	HandlePalsChanged();
}

void UDC1PalBoxScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (nullptr != CursorWidget && true == CursorWidget->IsInViewport())
	{
		CursorWidget->SetPositionInViewport(UWidgetLayoutLibrary::GetMousePositionOnViewport(this) + CursorOffset, false);
	}
}

FReply UDC1PalBoxScreenWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (false == PalBoxViewModel.IsValid())
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	const FKey Button = InMouseEvent.GetEffectingButton();

	// 우클릭 취소
	if (EKeys::RightMouseButton == Button && true == PalBoxViewModel->IsPicking())
	{
		PalBoxViewModel->CancelPick();
		return FReply::Handled();
	}

	if (EKeys::LeftMouseButton != Button)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	EDC1PalContainer Container = EDC1PalContainer::Party;
	int32 SlotIndex = INDEX_NONE;
	const bool bOnSlot = FindSlotAt(InMouseEvent.GetScreenSpacePosition(), Container, SlotIndex);

	// 집은 상태: 칸이면 놓기, 아니면 취소
	if (true == PalBoxViewModel->IsPicking())
	{
		if (true == bOnSlot)
		{
			PalBoxViewModel->RequestDrop(Container, SlotIndex);
		}
		else
		{
			PalBoxViewModel->CancelPick();
		}

		return FReply::Handled();
	}

	if (false == bOnSlot)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// 빈칸이면 안 집힌다
	PalBoxViewModel->BeginPick(Container, SlotIndex);
	if (false == PalBoxViewModel->IsPicking())
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	bPressPending = true;
	PressPosition = InMouseEvent.GetScreenSpacePosition();

	return FReply::Handled();
}

FReply UDC1PalBoxScreenWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (EKeys::LeftMouseButton != InMouseEvent.GetEffectingButton() || false == bPressPending)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	bPressPending = false;

	// 안 끌었으면 클릭 - 집은 채 유지
	if (false == PalBoxViewModel.IsValid() || false == PalBoxViewModel->IsPicking()
		|| FVector2D::Distance(PressPosition, InMouseEvent.GetScreenSpacePosition()) < DragThreshold)
	{
		return FReply::Handled();
	}

	// 드래그: 다른 칸이면 놓기, 아니면 취소
	EDC1PalContainer Container = EDC1PalContainer::Party;
	int32 SlotIndex = INDEX_NONE;
	const bool bOnOtherSlot = FindSlotAt(InMouseEvent.GetScreenSpacePosition(), Container, SlotIndex)
		&& (Container != PalBoxViewModel->GetPickContainer() || SlotIndex != PalBoxViewModel->GetPickIndex());

	if (true == bOnOtherSlot)
	{
		PalBoxViewModel->RequestDrop(Container, SlotIndex);
	}
	else
	{
		PalBoxViewModel->CancelPick();
	}

	return FReply::Handled();
}

void UDC1PalBoxScreenWidget::BindViewModel()
{
	UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>();
	if (nullptr == VMSubsystem)
	{
		return;
	}

	PalBoxViewModel = VMSubsystem->GetViewModel<UDC1PalBoxViewModel>();
	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	PalBoxViewModel->OnPalsChanged.AddUObject(this, &ThisClass::HandlePalsChanged);
	PalBoxViewModel->OnPickChanged.AddUObject(this, &ThisClass::HandlePickChanged);

	Storage->OnSlotHovered.AddUObject(this, &ThisClass::HandleStorageSlotHovered);
	BaseStorage->OnSlotHovered.AddUObject(this, &ThisClass::HandleBaseSlotHovered);

	CreateSlots();
	HandlePalsChanged();
}

void UDC1PalBoxScreenWidget::UnbindViewModel()
{
	Storage->OnSlotHovered.RemoveAll(this);
	BaseStorage->OnSlotHovered.RemoveAll(this);

	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	PalBoxViewModel->OnPalsChanged.RemoveAll(this);
	PalBoxViewModel->OnPickChanged.RemoveAll(this);
	PalBoxViewModel.Reset();
}

void UDC1PalBoxScreenWidget::CreateSlots()
{
	Storage->CreateSlots(PalBoxViewModel->GetBoxCapacity());
	BaseStorage->CreateSlots(PalBoxViewModel->GetBaseCapacity());

	const int32 Capacity = PalBoxViewModel->GetCapacity();
	if (SlotWidgets.Num() == Capacity || nullptr == SlotWidgetClass)
	{
		return;
	}

	SlotContainer->ClearChildren();
	SlotWidgets.Reset();

	for (int32 Index = 0; Index < Capacity; ++Index)
	{
		UDC1PalBoxSlotWidget* SlotWidget = CreateWidget<UDC1PalBoxSlotWidget>(this, SlotWidgetClass);
		if (nullptr == SlotWidget)
		{
			continue;
		}

		SlotWidget->SetSlotIndex(Index);
		SlotWidget->OnSlotHovered.AddUObject(this, &ThisClass::HandleSlotHovered);

		SlotContainer->AddChild(SlotWidget);
		SlotWidgets.Add(SlotWidget);
	}
}

void UDC1PalBoxScreenWidget::RefreshSlots()
{
	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	for (int32 Index = 0; Index < SlotWidgets.Num(); ++Index)
	{
		FDC1PalBoxSlotInfo SlotInfo;
		if (true == PalBoxViewModel->GetSlotInfo(EDC1PalContainer::Party, Index, SlotInfo))
		{
			SlotWidgets[Index]->SetSlotInfo(SlotInfo);
		}
		else
		{
			SlotWidgets[Index]->SetEmpty();
		}
	}
}

void UDC1PalBoxScreenWidget::RefreshStorage(UDC1PalBoxStorageWidget* InStorage, EDC1PalContainer InContainer)
{
	if (false == PalBoxViewModel.IsValid())
	{
		return;
	}

	for (int32 Index = 0; Index < InStorage->GetSlotWidgets().Num(); ++Index)
	{
		FDC1PalBoxSlotInfo SlotInfo;
		if (true == PalBoxViewModel->GetSlotInfo(InContainer, Index, SlotInfo))
		{
			InStorage->SetSlotInfo(Index, SlotInfo);
		}
		else
		{
			InStorage->SetEmpty(Index);
		}
	}
}

void UDC1PalBoxScreenWidget::RefreshDetail()
{
	FDC1PalBoxDetailInfo DetailInfo;
	if (true == PalBoxViewModel.IsValid() && true == PalBoxViewModel->GetDetailInfo(HoveredContainer, HoveredSlot, DetailInfo))
	{
		Detail->SetDetailInfo(DetailInfo);
	}
	else
	{
		Detail->ShowNoData();
	}
}

bool UDC1PalBoxScreenWidget::FindSlotAt(const FVector2D& InScreenPosition, EDC1PalContainer& OutContainer, int32& OutSlotIndex) const
{
	for (int32 Index = 0; Index < SlotWidgets.Num(); ++Index)
	{
		if (true == SlotWidgets[Index]->GetCachedGeometry().IsUnderLocation(InScreenPosition))
		{
			OutContainer = EDC1PalContainer::Party;
			OutSlotIndex = Index;
			return true;
		}
	}

	const TPair<const UDC1PalBoxStorageWidget*, EDC1PalContainer> Storages[] =
	{
		{ Storage, EDC1PalContainer::Box },
		{ BaseStorage, EDC1PalContainer::Base },
	};

	for (const TPair<const UDC1PalBoxStorageWidget*, EDC1PalContainer>& Pair : Storages)
	{
		// 거점이 없으면 ③ 은 칸이 아니다
		if (false == Pair.Key->GetIsEnabled())
		{
			continue;
		}

		const TArray<TObjectPtr<UDC1PalBoxStorageSlotWidget>>& StorageSlots = Pair.Key->GetSlotWidgets();
		for (int32 Index = 0; Index < StorageSlots.Num(); ++Index)
		{
			if (true == StorageSlots[Index]->GetCachedGeometry().IsUnderLocation(InScreenPosition))
			{
				OutContainer = Pair.Value;
				OutSlotIndex = Index;
				return true;
			}
		}
	}

	return false;
}

void UDC1PalBoxScreenWidget::HandlePalsChanged()
{
	RefreshSlots();
	RefreshStorage(Storage, EDC1PalContainer::Box);
	RefreshStorage(BaseStorage, EDC1PalContainer::Base);
	BaseStorage->SetIsEnabled(true == PalBoxViewModel.IsValid() && true == PalBoxViewModel->HasBase());
	RefreshDetail();
}

void UDC1PalBoxScreenWidget::HandlePickChanged()
{
	if (false == PalBoxViewModel.IsValid() || false == PalBoxViewModel->IsPicking())
	{
		if (nullptr != CursorWidget)
		{
			CursorWidget->RemoveFromParent();
		}

		bPressPending = false;
		return;
	}

	if (nullptr == CursorWidget && nullptr != CursorWidgetClass)
	{
		CursorWidget = CreateWidget<UDC1PalBoxCursorWidget>(GetOwningPlayer(), CursorWidgetClass);
	}

	if (nullptr == CursorWidget)
	{
		return;
	}

	FDC1PalBoxSlotInfo SlotInfo;
	PalBoxViewModel->GetSlotInfo(PalBoxViewModel->GetPickContainer(), PalBoxViewModel->GetPickIndex(), SlotInfo);
	CursorWidget->SetIcon(SlotInfo.Icon);

	if (false == CursorWidget->IsInViewport())
	{
		CursorWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		CursorWidget->AddToViewport(999);
		CursorWidget->SetPositionInViewport(UWidgetLayoutLibrary::GetMousePositionOnViewport(this) + CursorOffset, false);
	}
}

void UDC1PalBoxScreenWidget::HandleSlotHovered(int32 InSlotIndex)
{
	HoveredContainer = EDC1PalContainer::Party;
	HoveredSlot = InSlotIndex;
	RefreshDetail();
}

void UDC1PalBoxScreenWidget::HandleStorageSlotHovered(int32 InSlotIndex)
{
	HoveredContainer = EDC1PalContainer::Box;
	HoveredSlot = InSlotIndex;
	RefreshDetail();
}

void UDC1PalBoxScreenWidget::HandleBaseSlotHovered(int32 InSlotIndex)
{
	HoveredContainer = EDC1PalContainer::Base;
	HoveredSlot = InSlotIndex;
	RefreshDetail();
}
