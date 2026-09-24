// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalBoxScreenWidget.generated.h"

class UPanelWidget;
class UDC1PalBoxViewModel;
class UDC1PalBoxSlotWidget;
class UDC1PalBoxDetailWidget;

/**
 * 팰박스 화면. 보유 팰 + 상세 정보
 */
UCLASS()
class DC1_API UDC1PalBoxScreenWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	virtual void	HandleInputKey(const FKey& InKey) override;

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;
	virtual void	NativeOnActivated() override;

private:
	void			BindViewModel();
	void			UnbindViewModel();
	void			CreateSlots();
	void			RefreshSlots();
	void			RefreshDetail();

	void			HandlePartyChanged();
	void			HandleSlotHovered(int32 InSlotIndex);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget>			SlotContainer = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalBoxDetailWidget>	Detail = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "PalBox")
	TSubclassOf<UDC1PalBoxSlotWidget>	SlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "PalBox")
	FKey								CloseKey = EKeys::Zero;

private:
	TWeakObjectPtr<UDC1PalBoxViewModel>	PalBoxViewModel = { nullptr };

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDC1PalBoxSlotWidget>>	SlotWidgets;

	int32								HoveredSlot = { INDEX_NONE };
};
