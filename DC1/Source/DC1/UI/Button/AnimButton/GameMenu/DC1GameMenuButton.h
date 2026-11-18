// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/AnimButton/DC1BracketButton.h"
#include "DC1GameMenuButton.generated.h"

class UBorder;

UCLASS()
class DC1_API UDC1GameMenuButton : public UDC1BracketButton
{
	GENERATED_BODY()

protected:
	virtual void	NativeConstruct() override;

protected:
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UBorder> ButtonBox = { nullptr };

private:
	UPROPERTY(EditAnywhere, Category = "ButtonBox|Color")
	FLinearColor		HoverColor = { FLinearColor::Blue };

	UPROPERTY(EditAnywhere, Category = "ButtonBox|Color")
	FLinearColor		UnhoverColor = { FLinearColor::Blue };
};
