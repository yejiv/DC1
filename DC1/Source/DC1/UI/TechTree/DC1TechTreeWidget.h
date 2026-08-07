// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TechTreeWidget.generated.h"

class UDataTable;
class UScrollBox;
class UDC1TechTreeRowWidget;

UCLASS()
class DC1_API UDC1TechTreeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeConstruct() override;

private:
	void			BuildTechTree();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox>				TechTreeScrollBox;

	TObjectPtr<const UDataTable>		CachedDataTable;
	TSubclassOf<UDC1TechTreeRowWidget>	RowWidgetClass;
};
