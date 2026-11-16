// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/GameMenu/DC1SoundOptionWidget.h"
#include "UI/Button/DC1UserButton.h"
#include "UI/Tab/GameMenu/DC1SoundOptionEntryWidget.h"
#include "Components/DynamicEntryBox.h"
#include "Subsystem/DC1SoundSettingSubsystem.h"

void UDC1SoundOptionWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	for (uint8 i = 0; i < static_cast<uint8>(EDC1SoundCategory::MAX); ++i)
	{
		UDC1SoundOptionEntryWidget* Entry = SoundListBox->CreateEntry<UDC1SoundOptionEntryWidget>();
		if (false == ::IsValid(Entry))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1SoundOptionWidget:: Failed Create UDC1SoundOptionEntryWidget"));
			break;
		}

		const EDC1SoundCategory SoundCategory = static_cast<EDC1SoundCategory>(i);
		Entry->SetSoundCategory(SoundCategory, SoundCategoryNames.FindRef(SoundCategory));
	}

	Button_Apply->OnClickedDelegate.AddUObject(this, &ThisClass::HandleApplyClicked);
}

void UDC1SoundOptionWidget::NativeOnActivated()
{
	Super::NativeOnActivated();

	for (UDC1SoundOptionEntryWidget* Entry : SoundListBox->GetTypedEntries<UDC1SoundOptionEntryWidget>())
	{
		Entry->RefreshVolume();
	}
}

void UDC1SoundOptionWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	UDC1SoundSettingSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (false == ::IsValid(SoundSubsystem))
	{
		return;
	}

	// 마지막 적용(Save)한 볼륨들로 세팅
	SoundSubsystem->RevertVolumes();
}

UDC1SoundSettingSubsystem* UDC1SoundOptionWidget::GetSoundSubsystem() const
{
	ULocalPlayer* LocalPlayer = GetOwningLocalPlayer();
	if (true == ::IsValid(LocalPlayer))
	{
		return LocalPlayer->GetSubsystem<UDC1SoundSettingSubsystem>();
	}
	return nullptr;
}

void UDC1SoundOptionWidget::HandleApplyClicked()
{
	UDC1SoundSettingSubsystem* SoundSubsystem = GetSoundSubsystem();
	if (nullptr == SoundSubsystem)
	{
		return;
	}

	// 볼륨 설정 Save 및 적용
	SoundSubsystem->CommitVolumes();
}
