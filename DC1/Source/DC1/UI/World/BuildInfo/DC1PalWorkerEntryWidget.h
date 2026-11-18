// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalWorkerEntryWidget.generated.h"

class UImage;
class UTextBlock;
class UProgressBar;
class ADC1PalCharacter;
struct FOnAttributeChangeData;

UCLASS()
class DC1_API UDC1PalWorkerEntryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeDestruct() override;

public:
	void			SetPal(ADC1PalCharacter* InPal);
	void			ClearPal();

private:
	void			OnSanityChanged(const FOnAttributeChangeData& InData);
	void			RefreshSanity();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			PalIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar>	SanityBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>			StateIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>		SanityText;

	UPROPERTY(EditDefaultsOnly, Category = "Icon")
	FName						PalIconImageParamName = { "IconImage" };

private:
	TWeakObjectPtr<ADC1PalCharacter>	Pal;
	FDelegateHandle						SanityChangedHandle;
	FDelegateHandle						MaxSanityChangedHandle;
};
