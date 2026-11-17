// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Define/DC1Enums.h"
#include "DC1SoundSettingSubsystem.generated.h"

class UDC1SaveSubsystem;

UCLASS()
class DC1_API UDC1SoundSettingSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void		Initialize(FSubsystemCollectionBase& Collection) override;

public:
	float				GetVolume(EDC1SoundCategory InCategory) const;

	void				SetVolume(EDC1SoundCategory InCategory, float InVolume);

	void				ApplyAllVolumes();

	void				CommitVolumes();

	void				RevertVolumes();

private:
	UDC1SaveSubsystem*	GetSaveSubsystem() const;

	void				ApplyVolume(EDC1SoundCategory InCategory);

private:
	TMap<EDC1SoundCategory, float> Volumes;

	TMap<EDC1SoundCategory, float> SavedVolumes;
};
