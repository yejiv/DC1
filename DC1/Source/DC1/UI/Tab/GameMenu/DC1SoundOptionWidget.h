// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1SoundOptionWidget.generated.h"

class UDC1UserButton;
class UDC1SoundSettingSubsystem;
class UDynamicEntryBox;

UCLASS()
class DC1_API UDC1SoundOptionWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeOnActivated() override;
	virtual void	NativeOnDeactivated() override;

private:
	UDC1SoundSettingSubsystem* GetSoundSubsystem() const;

	void			HandleApplyClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	SoundListBox;

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1UserButton>		Button_Apply;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TMap<EDC1SoundCategory, FText>	SoundCategoryNames;
};
