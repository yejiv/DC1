// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "DC1AppearanceTypes.generated.h"

/**
 *
 *
 */
USTRUCT()
struct FDC1AppearanceData
{
	GENERATED_BODY()

	UPROPERTY()
	FGameplayTag HairTag = { };

	UPROPERTY()
	FGameplayTag HeadTag = { };

	// 눈은 Head 메쉬를 교체하지 않고, Head 메쉬의 눈 전용 머티리얼 슬롯을 교체한다
	UPROPERTY()
	FGameplayTag EyeTag = { };

	UPROPERTY()
	FLinearColor HairColor = { FLinearColor::White };

	// M_Player_Skin의 SkinColor 파라미터 자체 기본값(0.317 회색)과 맞춤 - 흰색으로 두면 텍스처가 약 3배 밝게 곱해짐
	UPROPERTY()
	FLinearColor SkinColor = { FLinearColor(0.317708f, 0.317708f, 0.317708f, 1.f) };

	UPROPERTY()
	FLinearColor EyeColor = { FLinearColor::White };
};
