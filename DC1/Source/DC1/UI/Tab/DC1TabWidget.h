// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetSwitcher.h"
#include "DC1TabWidget.generated.h"

class UPanelWidget;
class UDC1TabHeaderButton;

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
	virtual void	NativeOnInitialized() override;
	
protected:
	virtual void			ButtonInitialize();
	virtual void			CloseWidget(const FKey& InputKey);

	UDC1TabHeaderButton*	GetTabButton(const FGameplayTag& Tab) const;

	template<typename TabType>
	TabType*			GetTabWidget(const FGameplayTag& Tab) const;

public:
	virtual void	HandleInputKey(const FKey& InputKey) override;

public:
	bool			SelectTab(const FGameplayTag& Tab);
	void			SelectFirstInactiveTab();

	void			EnableTab(const FGameplayTag& Tab);

private:
	bool			CanSelectTab(const FGameplayTag& Tab) const;
	void			ChangeTab(bool IsNext);

protected:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1TabHeaderButton>> TabButtons;

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, int32>							TabToSwitchIndex;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>		TabSwitcher;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>		TabButtonContainer;

	UPROPERTY(EditAnywhere, Category = "TAB")
	FDC1KeyList						PrevTabKeys;

	UPROPERTY(EditAnywhere, Category = "TAB")
	FDC1KeyList						NextTabKeys;

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, FDC1KeyList> TabCloseKeys;


	TMap<int32, FGameplayTag>		SwitchIndexToTab;

	FGameplayTag					CurrentTab = FGameplayTag();
};


template<typename TabType>
inline TabType* UDC1TabWidget::GetTabWidget(const FGameplayTag& Tab) const
{
	const int32* Index = TabToSwitchIndex.Find(Tab);
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
