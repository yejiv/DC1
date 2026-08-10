// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1TabWidget.generated.h"

class UWidgetSwitcher;
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
	virtual void	NativeConstruct() override;
	
public:
	virtual void	HandleInputKey(const FKey& InputKey) override;

public:
	void			SelectTab(const FGameplayTag& Tab);

private:
	void			ChangeTab(bool IsNext);
	void			CloseWidget(const FKey& InputKey);

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

	UPROPERTY(EditAnywhere, Category = "TAB")
	TMap<FGameplayTag, int32>		TabToSwitchIndex;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1TabHeaderButton>> TabButtons;

	TMap<int32, FGameplayTag>		SwitchIndexToTab;

	FGameplayTag					CurrentTab = FGameplayTag();
};
