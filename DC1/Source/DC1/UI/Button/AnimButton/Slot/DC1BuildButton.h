// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"

#include "GameplayTagContainer.h"
#include "Data/DataTable/DC1DataTableTypes.h"

#include "DC1BuildButton.generated.h"



UCLASS()
class DC1_API UDC1BuildButton : public UDC1BracketButton
{
	GENERATED_BODY()
	
public:
	const FGameplayTag&			GetBuildTag() const { return BuildTag; }
	const TArray<FDC1ItemCost>& GetBuildCost() const { return CachedCosts; }
	int32						GetSortOrder() const { return SortOrder; }

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

public:
	void			InitBuildButton(const FDC1BuildRow& InBuildRow, const FDC1TechRow& InTechRow);
	void			SetState(bool InCanBuild);

private:
	void			ApplyState();

private:
	// Icon Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		BuildIconImage = { nullptr };

	// Inactive Hover Image ( Blend Black )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		InactiveHoverImage = { nullptr };

	// Button Outline Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		ButtonImage = { nullptr };

	// Button Outline Image
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		IconBackImage = { nullptr };

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Hover")
	FSlateBrush				HoverButtonImageBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Hover")
	FSlateBrush				UnHoverButtonImageBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Hover")
	FSlateBrush				HoverIconBackImageBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Hover")
	FSlateBrush				UnHoverIconBackImageBrush;

	UPROPERTY(EditAnywhere, Category = "UserButton|Build|Image|Opacity")
	float					InactiveHoverImageOpacity = { 0.4f };


private:
	bool					CanBuild = { false };

	FGameplayTag			BuildTag = { FGameplayTag::EmptyTag };
	int32					SortOrder = { 0 };

	TArray<FDC1ItemCost>	CachedCosts;
};
