// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "Components/WidgetSwitcher.h"
#include "DC1TabWidget.generated.h"

class UPanelWidget;
class USoundBase;
class UDC1TabHeaderButton;
class UDC1InventoryWidget;
class UDC1TechTreeWidget;

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

	// 페이지 활성 = 화면 활성 && 현재 탭 - 현재 페이지만 따로 넘긴다
	virtual void	NativeOnActivated() override;
	virtual void	NativeOnDeactivated() override;
	virtual bool	ShouldPropagateTo(const UDC1UserWidget* InChild) const override;

public:
	virtual void	HandleInputKey(const FKey& InKey) override;

public:
	bool			SelectTab(const FGameplayTag& InTab);
	void			SelectFirstInactiveTab();
	void			EnableTab(const FGameplayTag& InTab);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tab")
	UDC1InventoryWidget* GetInventoryWidget() const;
	
	UDC1TechTreeWidget* GetTechTreeWidget() const;

protected:
	virtual void	ButtonInitialize();
	virtual void	CloseWidget(const FKey& InKey);
	virtual void	OnTabSelected(const FGameplayTag& InTab) {}

private:
	UDC1UserWidget*	GetActivePage() const;
	bool			CanSelectTab(const FGameplayTag& InTab) const;
	void			ChangeTab(bool InIsNext);

protected:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1TabHeaderButton>> TabButtons;

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, int32>							TabToSwitchIndex;

	FGameplayTag										CurrentTab = { FGameplayTag::EmptyTag };

	// 닫기 키로 화면을 닫을 때
	UPROPERTY(EditDefaultsOnly, Category = "TAB|Sound")
	TObjectPtr<USoundBase>								CloseSound = { nullptr };

private:
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UWidgetSwitcher>		TabSwitcher = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget>		TabButtonContainer = { nullptr };

	UPROPERTY(EditAnywhere, Category = "TAB")
	FDC1KeyList						PrevTabKeys;

	UPROPERTY(EditAnywhere, Category = "TAB")
	FDC1KeyList						NextTabKeys;

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, FDC1KeyList> TabCloseKeys;

	// 이전 / 다음 탭 키로 전환될 때
	UPROPERTY(EditDefaultsOnly, Category = "TAB|Sound")
	TObjectPtr<USoundBase>			TabChangeSound = { nullptr };

private:
	TMap<int32, FGameplayTag>		SwitchIndexToTab;
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
