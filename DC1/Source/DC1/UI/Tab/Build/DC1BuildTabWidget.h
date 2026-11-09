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
	virtual void			NativeOnActivated() override;
	virtual void			CloseWidget(const FKey& InKey) override;

public:
	virtual	void			HandleInputKey(const FKey& InKey) override;
	void					SetViewModel(UDC1BuildViewModel* InViewModel);
private:
	void					RefreshAll();
	void					RefreshCostState();
	void					HandleBuildUnlocked(const FGameplayTag& InUnlockedTag);


private:
	UPROPERTY(EditAnywhere, Category = "Build")
	FDC1KeyList				BuildCloseKeys;

	UPROPERTY(EditAnywhere, Category = "Build")
	FDC1KeyList				DismantleKeys;

private:
	UPROPERTY()
	TObjectPtr<UDC1BuildViewModel>	CachedViewModel = { nullptr };

	bool							IsInit = { false };
};
