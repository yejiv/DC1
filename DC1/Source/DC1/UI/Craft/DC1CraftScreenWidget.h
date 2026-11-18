// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Tab/DC1TabWidget.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1CraftScreenWidget.generated.h"

class ADC1CraftBuildActor;
class UDC1CraftSelectWidget;
class UDC1RecipeButton;
class UDynamicEntryBox;
class UTextBlock;

UCLASS()
class DC1_API UDC1CraftScreenWidget : public UDC1TabWidget
{
	GENERATED_BODY()

public:
	UDC1CraftScreenWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	void			OpenStation(ADC1CraftBuildActor* InStation);

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeOnDeactivated() override;
	virtual void	OnTabSelected(const FGameplayTag& InTab) override;
	virtual void	CloseWidget(const FKey& InKey) override;

private:
	void			CacheDataRows();
	void			BuildRecipeList();
	void			ApplyTabFilter();
	bool			IsRecipeInTab(const FDC1ItemRecipeRow& InRecipe) const;
	void			SelectRecipe(FName InRecipeID);
	void			ClearSelection();

	void			HandleCraftRequested(FName InRecipeID, int32 InCount);
	void			HandleProductionChanged();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_StationName = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>		RecipeListBox = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1CraftSelectWidget>	CraftSelectWidget = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Craft|Tab")
	TMap<FGameplayTag, FGameplayTagContainer>	TabItemTypes;

	UPROPERTY(EditAnywhere, Category = "Craft|Tab")
	FGameplayTag						DefaultTabTag = { FGameplayTag::EmptyTag };

	UPROPERTY(EditAnywhere, Category = "Craft")
	FDC1KeyList							CloseKeys;

private:
	TWeakObjectPtr<ADC1CraftBuildActor>	Station = { nullptr };

	TArray<FDC1ItemRecipeRow>			CachedRecipeRows;
	TMap<FGameplayTag, FText>			StationNames;

	TMap<FName, const FDC1ItemRecipeRow*>	ShownRecipes;

	UPROPERTY()
	TMap<FName, TObjectPtr<UDC1RecipeButton>>	RecipeButtons;

	FName								SelectedRecipeID = { NAME_None };
};
