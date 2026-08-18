// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tab/DC1TabWidget.h"
#include "DC1BuildTabWidget.generated.h"

class UDC1TechTreeComponent;
class UDC1BuildCategoryWidget;

UCLASS()
class DC1_API UDC1BuildTabWidget : public UDC1TabWidget
{
	GENERATED_BODY()
	
protected:
	virtual void			NativeOnInitialized() override;

protected:
	virtual void			ButtonInitialize() override;
	virtual void			CloseWidget(const FKey& InputKey) override;

private:
	UDC1TechTreeComponent*	GetTechTreeComponent();
	void					OnTechStateChanged();

private:
	UPROPERTY(EditAnywhere, Category = "Build")
	FDC1KeyList				BuildCloseKeys;
};
