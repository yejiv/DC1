// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/GameMenu/DC1SoundOptionEntryWidget.h"
#include "UI/Bar/DC1Slider.h"
#include "Components/TextBlock.h"
#include "Subsystem/DC1SoundSettingSubsystem.h"

void UDC1SoundOptionEntryWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Slider_Volume->OnValueChanged.AddDynamic(this, &ThisClass::HandleVolumeChanged);
}

void UDC1SoundOptionEntryWidget::SetSoundCategory(EDC1SoundCategory InCategory, const FText& InCategoryName)
{
	SoundCategory = InCategory;

	Text_SoundCategory->SetText(InCategoryName);
}

void UDC1SoundOptionEntryWidget::RefreshVolume()
{
	UDC1SoundSettingSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (false == ::IsValid(SoundSubsystem))
	{
		return;
	}

	const float Volume = SoundSubsystem->GetVolume(SoundCategory);

	Slider_Volume->SetValue(Volume);
	Slider_Volume->RefreshFill();
	UpdateVolumeText(Volume);
}

UDC1SoundSettingSubsystem* UDC1SoundOptionEntryWidget::GetSoundSubsystem() const
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (true == ::IsValid(LocalPlayer))
	{
		return LocalPlayer->GetSubsystem<UDC1SoundSettingSubsystem>();
	}
	return nullptr;
}

void UDC1SoundOptionEntryWidget::UpdateVolumeText(float InValue)
{
	const int32 Percent = FMath::RoundToInt(InValue * 100.f);

	Text_CategoryVolume->SetText(FText::AsNumber(Percent));
}

void UDC1SoundOptionEntryWidget::HandleVolumeChanged(float InValue)
{
	UDC1SoundSettingSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (false == ::IsValid(SoundSubsystem))
	{
		return;
	}

	SoundSubsystem->SetVolume(SoundCategory, InValue);

	UpdateVolumeText(InValue);
}
