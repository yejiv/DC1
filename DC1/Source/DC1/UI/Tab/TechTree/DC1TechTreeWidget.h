// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TechTreeWidget.generated.h"

struct FOnAttributeChangeData;
class UDataTable;
class UScrollBox;
class UDC1TechTreeRowWidget;
class UDC1TechTreeButton;
class UDC1TechTreePointWidget;
class UDC1TechTreeComponent;


UCLASS()
class DC1_API UDC1TechTreeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;

private:
	UDC1TechTreeComponent*	GetTechTreeComponent();
	void					InitRows(int32 InNewLevel);
	
	void					UpdateRows(int32 InNewLevel);
	void					UpdatePoint(const UDC1TechTreeComponent* InTechComp);
	void					UpdateSlots(const UDC1TechTreeComponent* InTechComp);

	// DataTable 읽어서 Row, Slot 생성
	void					InitTechTree();

	// TechTree Slot 상태 변화 ( Lock, Available, Unlock )
	void					OnTechStateChanged();


private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox>						TechTreeScrollBox;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TechTreePointWidget>			TechPointWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1TechTreePointWidget>			AncientTechPointWidget;

	UPROPERTY(EditAnywhere, Category = "UserButton")
	TSubclassOf<UDC1TechTreeRowWidget>			RowWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UDC1TechTreeRowWidget>>	TechRows;

	UPROPERTY()
	TArray<TObjectPtr<UDC1TechTreeButton>>		TechSlots;

	int32										CachedLevel = 0;
};
