// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1RecipeButton.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnRecipeClicked, FName);

UCLASS()
class DC1_API UDC1RecipeButton : public UDC1BracketButton
{
	GENERATED_BODY()

public:
	FDC1OnRecipeClicked		OnRecipeClicked;

public:
	void			SetRecipe(const FDC1ItemRecipeRow& InRecipe, bool InCanCraft);
	void			SetSelected(bool InSelected);

protected:
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;
	virtual void	HandleClicked() override;
	virtual void	RefreshTooltip(UDC1TooltipWidget* InTooltip) override;

private:
	void			ApplyState();
	void			ApplySelectVisual(bool InShow);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		RecipeIconImage = { nullptr };

	// Inactive Image ( 비활성화 )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		InactiveImage = { nullptr };

	// Button Outline Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		ButtonImage = { nullptr };
	// Button Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		IconBackImage = { nullptr };

	// Button Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		ButtonBottomImage = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	ProductCountText = { nullptr };

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Hover")
	FSlateBrush				HoverButtonImageBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Hover")
	FSlateBrush				UnHoverButtonImageBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Recipe|Image|Opacity")
	float					InactiveImageOpacity = { 0.4f };

private:
	bool					CanCraft = { false };
	bool					IsSelected = { false };
	FName					RecipeID = { NAME_None };

	FDC1ItemRecipeRow		CachedRecipe;
};
