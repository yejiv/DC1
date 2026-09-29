// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalBoxViewModel.generated.h"

class UDC1PalPartyComponent;

DECLARE_MULTICAST_DELEGATE(FOnPalBoxPalsChanged);
DECLARE_MULTICAST_DELEGATE(FOnPalBoxPickChanged);

/**
 * 팰박스 화면용 ViewModel. 파티·상자를 표시값으로 가공하고 집기 상태를 든다
 */
UCLASS()
class DC1_API UDC1PalBoxViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	int32			GetCapacity() const;
	int32			GetBoxCapacity() const;

	// false = 빈 칸
	bool			GetSlotInfo(EDC1PalContainer InContainer, int32 InSlotIndex, FDC1PalBoxSlotInfo& OutSlotInfo) const;
	bool			GetDetailInfo(EDC1PalContainer InContainer, int32 InSlotIndex, FDC1PalBoxDetailInfo& OutDetailInfo) const;

public:
	// 집기
	bool				IsPicking() const { return bPicking; }
	EDC1PalContainer	GetPickContainer() const { return PickContainer; }
	int32				GetPickIndex() const { return PickIndex; }

	void			BeginPick(EDC1PalContainer InContainer, int32 InSlotIndex);
	void			RequestDrop(EDC1PalContainer InContainer, int32 InSlotIndex);
	void			CancelPick();

public:
	FOnPalBoxPalsChanged	OnPalsChanged;
	FOnPalBoxPickChanged	OnPickChanged;

private:
	void			HandlePalChanged();

private:
	TWeakObjectPtr<UDC1PalPartyComponent>	PalPartyComponent = { nullptr };

	bool				bPicking = { false };
	EDC1PalContainer	PickContainer = { EDC1PalContainer::Party };
	int32				PickIndex = { INDEX_NONE };
};
