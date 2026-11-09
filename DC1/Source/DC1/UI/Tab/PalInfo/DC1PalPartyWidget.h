// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1PalPartyWidget.generated.h"

class UImage;
class UDynamicEntryBox;
class UDC1PalBoxViewModel;
class UDC1PalViewModel;
class UDC1PalBoxSlotWidget;

DECLARE_DELEGATE_OneParam(FOnChangedFocusingPal, const FGameplayTag& /*PalTag*/);

/**
 * 팰 파티 현황 (파티에 등록한 팰 목록 + 선택한 팰 미리보기)
 */
UCLASS()
class DC1_API UDC1PalPartyWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	FOnChangedFocusingPal	OnChangedFocusingPal;

public:
	void RefreshPalPartySlots();

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;

private:
	void BindViewModel();

	void RefreshPalInfo(UDC1PalBoxSlotWidget* InSlot, int32 SlotIndex);

private:
	void HandleSlotHovered(int32 SlotIndex);

private:
	// 파티 슬롯 목록 (Entry Widget Class = PalBoxSlotWidget)
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>		EntryBox_Slots = { nullptr };

private:
	UPROPERTY()
	TWeakObjectPtr<UDC1PalBoxViewModel>	CachedPalBoxViewModel = { nullptr };

	UPROPERTY()
	TWeakObjectPtr<UDC1PalViewModel>		CachedPalViewModel = { nullptr };
};
