// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DC1PalDisplayTypes.generated.h"

class UTexture2D;

/**
 * 
 */
USTRUCT()
struct FDC1PalDisplayInfo
{
	GENERATED_BODY()

	// 닉네임이 비어 있으면 종 이름
	UPROPERTY()
	FText					Name;

	UPROPERTY()
	int32					Level = { 1 };

	// 0~1
	UPROPERTY()
	float					HealthPercent = { 0.f };

	UPROPERTY()
	TObjectPtr<UTexture2D>	ElementIcon = { nullptr };
};