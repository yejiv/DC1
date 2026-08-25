// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetSwitcher.h"
#include "DC1TabWidget.generated.h"

class UPanelWidget;
class UDC1TabHeaderButton;
class UDC1InventoryWidget;

USTRUCT(BlueprintType)
struct FDC1KeyList
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FKey> Keys;
};

UCLASS()
class DC1_API UDC1TabWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	UDC1TabHeaderButton* GetTabButton(const FGameplayTag& InTab) const;

	template<typename TabType>
	TabType* GetTabWidget(const FGameplayTag& InTab) const;

protected:
	virtual void	NativeOnInitialized() override;
	
public:
	virtual void	HandleInputKey(const FKey& InKey) override;

public:
	bool			SelectTab(const FGameplayTag& InTab);
	void			SelectFirstInactiveTab();
	void			EnableTab(const FGameplayTag& InTab);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tab")
	UDC1InventoryWidget* GetInventoryWidget() const;

protected:
	virtual void	ButtonInitialize();
	virtual void	CloseWidget(const FKey& InKey);

private:
	bool			CanSelectTab(const FGameplayTag& InTab) const;
	void			ChangeTab(bool InIsNext);

protected:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1TabHeaderButton>> TabButtons;

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, int32>							TabToSwitchIndex;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		TabSwitcher = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>		TabButtonContainer = { nullptr };

	UPROPERTY(EditAnywhere, Category = "TAB")
	FDC1KeyList						PrevTabKeys;

	UPROPERTY(EditAnywhere, Category = "TAB")
	FDC1KeyList						NextTabKeys;

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, FDC1KeyList> TabCloseKeys;

private:
	TMap<int32, FGameplayTag>		SwitchIndexToTab;

	FGameplayTag					CurrentTab = { FGameplayTag::EmptyTag };
};


template<typename TabType>
inline TabType* UDC1TabWidget::GetTabWidget(const FGameplayTag& InTab) const
{
	const int32* Index = TabToSwitchIndex.Find(InTab);
	if (nullptr == Index)
	{
		return nullptr;
	}

	if (false == ::IsValid(TabSwitcher))
	{
		return nullptr;
	}

	return Cast<TabType>(TabSwitcher->GetChildAt(*Index));
}
