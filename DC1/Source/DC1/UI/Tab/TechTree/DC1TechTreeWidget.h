// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TechTreeWidget.generated.h"

class UScrollBox;
class UDC1TechTreeRowWidget;
class UDC1TechTreeButton;
class UDC1TechTreePointWidget;

class UDC1TechTreeViewModel;

UCLASS()
class DC1_API UDC1TechTreeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void			NativeOnInitialized() override;

public:
	void					SetViewModel(UDC1TechTreeViewModel* InViewModel);

private:
	void					InitRows(int32 InNewLevel);
	
	void					UpdateRows(int32 InNewLevel);
	void					UpdatePoint();
	void					UpdateSlots();

	// DataTable 읽어서 Row, Slot 생성
	void					InitTechTree();

	// TechTree Slot 상태 변화 ( Lock, Available, Unlock )
	void					RefreshAll();


private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox>						TechTreeScrollBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TechTreePointWidget>			TechPointWidget = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TechTreePointWidget>			AncientTechPointWidget = { nullptr };

	UPROPERTY(EditAnywhere, Category = "RowWidget")
	TSubclassOf<UDC1TechTreeRowWidget>			RowWidgetClass = { nullptr };

private:
	UPROPERTY()
	TArray<TObjectPtr<UDC1TechTreeRowWidget>>	TechRows;

	UPROPERTY()
	TArray<TObjectPtr<UDC1TechTreeButton>>		TechSlots;

	UPROPERTY()
	TObjectPtr<UDC1TechTreeViewModel>			CachedViewModel = { nullptr };

	int32										CachedLevel = { 0 };

	bool										IsInit = { false };
};
