#include "UI/Tab/Inventory/Status/DC1MoneyWidget.h"

#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Inventory/DC1InventoryViewModel.h"

void UDC1MoneyWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (UDC1ViewModelSubsystem* ViewModelSubsystem = GetOwningLocalPlayer() ? GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>() : nullptr)
	{
		CachedViewModel = ViewModelSubsystem->GetViewModel<UDC1InventoryViewModel>();
	}
}

void UDC1MoneyWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
		CachedViewModel->OnInventoryUpdated.AddUObject(this, &ThisClass::HandleInventoryUpdated);
	}

	HandleInventoryUpdated();
}

void UDC1MoneyWidget::NativeDestruct()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnInventoryUpdated.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1MoneyWidget::HandleInventoryUpdated()
{
	if (false == ::IsValid(TextBlock_Money))
	{
		return;
	}

	FNumberFormattingOptions Options;
	Options.UseGrouping = false;
	TextBlock_Money->SetText(FText::AsNumber(::IsValid(CachedViewModel) ? CachedViewModel->GetMoneyCount() : 0, &Options));
}
