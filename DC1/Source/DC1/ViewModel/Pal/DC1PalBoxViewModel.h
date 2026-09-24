// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "DC1PalBoxViewModel.generated.h"

class UDC1PalPartyComponent;

DECLARE_MULTICAST_DELEGATE(FOnPalBoxPartyChanged);

/**
 * 팰박스 화면용 ViewModel. 파티 컴포넌트를 표시값으로 가공한다
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

	// false = 빈 칸
	bool			GetSlotInfo(int32 InSlotIndex, FDC1PalBoxSlotInfo& OutSlotInfo) const;
	bool			GetDetailInfo(int32 InSlotIndex, FDC1PalBoxDetailInfo& OutDetailInfo) const;

public:
	FOnPalBoxPartyChanged	OnPartyChanged;

private:
	void			HandlePalChanged();

private:
	TWeakObjectPtr<UDC1PalPartyComponent>	PalPartyComponent = { nullptr };
};
