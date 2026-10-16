#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1MoneyWidget.generated.h"

class UImage;
class UTextBlock;
class UDC1InventoryViewModel;

UCLASS()
class DC1_API UDC1MoneyWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void HandleInventoryUpdated();

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Money;

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel = { nullptr };
};
