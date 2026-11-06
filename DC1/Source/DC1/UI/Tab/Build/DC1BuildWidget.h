// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1BuildWidget.generated.h"

class ADC1PlayerState;
class UDC1BuildRowWidget;
class UDC1BuildViewModel;

class UScrollBox;
class UTextBlock;

USTRUCT(BlueprintType)
struct FDC1BuildRowInfo
{
	GENERATED_BODY()

	// Row Tag - ex) Build.Crafting.CraftAndRepair
	UPROPERTY(EditAnywhere)
	FGameplayTag RowTag;

	// Row Name - ex) 제작 및 수리
	UPROPERTY(EditAnywhere)
	FText RowName;
};

UCLASS()
class DC1_API UDC1BuildWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	void	InitBuild(UDC1BuildViewModel* InViewModel);

	bool	VisibleSlot(const FGameplayTag& InRowTag, const FGameplayTag& InSlotTag);

	void	RefreshCostState(const ADC1PlayerState* InPlayerState);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox>			BuildScrollBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			BuildTextLabel = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Build|Row")
	TArray<FDC1BuildRowInfo>		RowInfos;

	UPROPERTY(EditAnywhere, Category = "Build|Row")
	TSubclassOf<UDC1BuildRowWidget>	RowWidgetClass = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Build|Row")
	FText							BuildText = { FText::GetEmpty()};

private:
	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1BuildRowWidget>> BuildRowsByTag;

	// 행/슬롯 위젯 생성은 1회만. RefreshAll은 상태가 바뀔 때마다 불린다
	bool	IsInit = { false };
};
