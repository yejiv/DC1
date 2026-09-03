// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tab/DC1TabWidget.h"
#include "DC1BuildTabWidget.generated.h"

class UDC1BuildViewModel;

UCLASS()
class DC1_API UDC1BuildTabWidget : public UDC1TabWidget
{
	GENERATED_BODY()

protected:
	virtual void			NativeOnInitialized() override;
	virtual void			CloseWidget(const FKey& InKey) override;

public:
	void					SetViewModel(UDC1BuildViewModel* InViewModel);

private:
	void					RefreshAll();
	void					HandleBuildUnlocked(const FGameplayTag& InUnlockedTag);

private:
	UPROPERTY(EditAnywhere, Category = "Build")
	FDC1KeyList				BuildCloseKeys;

private:
	UPROPERTY()
	TObjectPtr<UDC1BuildViewModel>	CachedViewModel;

	bool							IsInit = false;
};
