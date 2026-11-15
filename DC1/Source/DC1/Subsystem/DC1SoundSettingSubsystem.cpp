// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/DC1SoundSettingSubsystem.h"
#include "Save/DC1SaveSubsystem.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Sound/DC1SoundData.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Kismet/GameplayStatics.h"
#include "Define/DC1Macro.h"

void UDC1SoundSettingSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	// LoadSoundData가 실패 대비 1로 초기화
	for (uint8 i = 0; i < static_cast<uint8>(EDC1SoundCategory::MAX); ++i)
	{
		SavedVolumes.Add(static_cast<EDC1SoundCategory>(i), 1.f);
	}
	
	UDC1SaveSubsystem* SaveSubsystem = GetSaveSubsystem();
	if (true == ::IsValid(SaveSubsystem))
	{
		SaveSubsystem->LoadSoundData(&SavedVolumes);
	}

	Volumes = SavedVolumes;
}

float UDC1SoundSettingSubsystem::GetVolume(EDC1SoundCategory InCategory) const
{
	const float* Volume = Volumes.Find(InCategory);
	if (nullptr == Volume)
	{
		return 1.f;
	}

	return *Volume;
}

void UDC1SoundSettingSubsystem::SetVolume(EDC1SoundCategory InCategory, float InVolume)
{
	Volumes.Add(InCategory, FMath::Clamp(InVolume, 0.f, 1.f));

	ApplyVolume(InCategory);
}

void UDC1SoundSettingSubsystem::ApplyAllVolumes()
{
	const UDC1SoundData* SoundData = UDC1AssetManager::GetAssetByTag<UDC1SoundData>(DC1AssetTags::Asset_SoundData);
	if (nullptr == SoundData || nullptr == SoundData->VolumeMix)
	{
		return;
	}

	for (const TPair<EDC1SoundCategory, TObjectPtr<USoundClass>>& Pair : SoundData->SoundClasses)
	{
		ApplyVolume(Pair.Key);
	}
	
	// Category별 볼륨 설정 후 VolumeMix에 세팅 -> Push ( Push 미실행 시 소리 변경 X )
	UGameplayStatics::PushSoundMixModifier(GetWorld(), SoundData->VolumeMix);
}

void UDC1SoundSettingSubsystem::CommitVolumes()
{
	SavedVolumes = Volumes;

	UDC1SaveSubsystem* SaveSubsystem = GetSaveSubsystem();
	if (false == ::IsValid(SaveSubsystem))
	{
		return;
	}

	SaveSubsystem->SaveSoundData(SavedVolumes);
}

void UDC1SoundSettingSubsystem::RevertVolumes()
{
	Volumes = SavedVolumes;

	for (const TPair<EDC1SoundCategory, float>& Pair : Volumes)
	{
		ApplyVolume(Pair.Key);
	}
}

UDC1SaveSubsystem* UDC1SoundSettingSubsystem::GetSaveSubsystem() const
{
	ULocalPlayer* LocalPlayer = GetLocalPlayer();
	if (true == ::IsValid(LocalPlayer))
	{
		UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
		if (true == ::IsValid(GameInstance))
		{
			return GameInstance->GetSubsystem<UDC1SaveSubsystem>();
		}
	}

	return nullptr;
}

void UDC1SoundSettingSubsystem::ApplyVolume(EDC1SoundCategory InCategory)
{
	const UDC1SoundData* SoundData = UDC1AssetManager::GetAssetByTag<UDC1SoundData>(DC1AssetTags::Asset_SoundData);
	if (nullptr == SoundData || nullptr == SoundData->VolumeMix)
	{
		return;
	}

	USoundClass* SoundClass = SoundData->SoundClasses.FindRef(InCategory);
	if (nullptr == SoundClass)
	{
		return;
	}
	// VolumeMix 내부 SoundClass의 볼륨 설정 ( Mix 내부에 SoundClass 없으면 런타임 추가 )
	UGameplayStatics::SetSoundMixClassOverride(GetWorld(), SoundData->VolumeMix, SoundClass, GetVolume(InCategory), 1.f, 0.f, true);
}
