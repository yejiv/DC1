// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "Define/DC1Enums.h"
#include "DC1SoundData.generated.h"

class USoundMix;
class USoundClass;

UCLASS()
class DC1_API UDC1SoundData : public UDataAsset
{
	GENERATED_BODY()

public:
	// USoundMix = SoundClass 별 볼륨, 피치, 로우패스 등 을 모아둔 컨테이너 개념
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<USoundMix> VolumeMix = { nullptr };

	// Category별 SoundClass만 매핑 -> 수치는 UDC1SoundSettingSubsystem에서 관리
	UPROPERTY(EditDefaultsOnly)
	TMap<EDC1SoundCategory, TObjectPtr<USoundClass>> SoundClasses;
};
