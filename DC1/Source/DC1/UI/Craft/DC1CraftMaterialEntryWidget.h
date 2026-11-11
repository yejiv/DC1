// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1CraftMaterialEntryWidget.generated.h"

class UImage;
class UTextBlock;

UCLASS()
class DC1_API UDC1CraftMaterialEntryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetMaterial(FName InItemID, int32 InOwnedCount, int32 InRequiredCount);
	void			SetMaterialAmount(FName InItemID, int32 InCount);
	void			ClearMaterial();

private:
	void			SetItemInfo(FName InItemID);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Count = { nullptr };

	// Inactive Image ( 비활성화 )
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>		InactiveImage = { nullptr };

private:
	UPROPERTY(EditAnywhere, Category = "UserButton|Recipe|Image|Opacity")
	float					InactiveImageOpacity = { 0.4f };

	UPROPERTY(EditAnywhere, Category = "UserButton|Recipe|Text|Color")
	FLinearColor			InactiveFontColor = { FLinearColor::Red };
};
