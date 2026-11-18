// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"
#include "DC1CraftTabHeaderButton.generated.h"

UCLASS()
class DC1_API UDC1CraftTabHeaderButton : public UDC1TabHeaderButton
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;

protected:
	virtual void	ApplyState() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			ButtonIconImage = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			ButtonBottomImage = { nullptr };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Craft|Image|Icon")
	TSoftObjectPtr<UTexture2D>	IconTexture = { nullptr };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Craft|Image|Color")
	FLinearColor				ActiveIconColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Craft|Image|Color")
	FLinearColor				InactiveIconColor = { FLinearColor::Gray };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Craft|Image|Color")
	FLinearColor				ActiveBottomColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "UserButton|Tab|Craft|Image|Color")
	FLinearColor				InactiveBottomColor = { FLinearColor::Gray };
};
