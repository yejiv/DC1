// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1WeightBarWidget.generated.h"

class UBorder;
class UProgressBar;
class UTextBlock;
class UDC1InventoryViewModel;

UCLASS()
class DC1_API UDC1WeightBarWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	void Refresh();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UBorder>			WeightBarBox = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UProgressBar>	ProgressBar_Weight = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		TextBlock_Weight = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Weight Bar")
	FLinearColor				OverweightColor = { FLinearColor::Red };
	
	UPROPERTY(EditAnywhere, Category = "Weight Bar")
	FLinearColor				DefaultColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "Weight Bar")
	bool						bShowOnlyWhenOverweight = { false };

private:
	TWeakObjectPtr<UDC1InventoryViewModel> InventoryViewModel = { nullptr };
};
