// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tab/DC1TabWidget.h"
#include "DC1BuildTabWidget.generated.h"

struct FDC1BuildRow;
struct FDC1TechRow;

class UDC1TechTreeComponent;
class UDC1BuildWidget;

UCLASS()
class DC1_API UDC1BuildTabWidget : public UDC1TabWidget
{
	GENERATED_BODY()
	
protected:
	virtual void			NativeOnInitialized() override;

protected:
	virtual void			ButtonInitialize() override;
	virtual void			CloseWidget(const FKey& InKey) override;

private:
	UDC1TechTreeComponent*	GetTechTreeComponent();

	bool					FindBuildData(TArray<const FDC1BuildRow*>& OutBuildRows);

	void					OnTechUnlocked(const FGameplayTag& InUnlockedTag);

private:
	UPROPERTY(EditAnywhere, Category = "Build")
	FDC1KeyList				BuildCloseKeys;
};
