// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1SoundOptionEntryWidget.generated.h"

class UDC1Slider;
class UTextBlock;
class UDC1SoundSettingSubsystem;

UCLASS()
class DC1_API UDC1SoundOptionEntryWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void						SetSoundCategory(EDC1SoundCategory InCategory, const FText& InCategoryName);

	void						RefreshVolume();

protected:
	virtual void				NativeOnInitialized() override;

private:
	UDC1SoundSettingSubsystem*	GetSoundSubsystem() const;

	void						UpdateVolumeText(float InValue);

	UFUNCTION()
	void						HandleVolumeChanged(float InValue);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_SoundCategory;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1Slider>		Slider_Volume;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_CategoryVolume;

private:
	EDC1SoundCategory			SoundCategory = { EDC1SoundCategory::Master };
};
