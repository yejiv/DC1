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

	// 표시 대상 개체. 다른 팰로 바뀌었는지 판단해 HP바 감소 연출 여부를 고른다
	UPROPERTY()
	FGuid					PalID;

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

	// 두 번째 속성이 없으면(Neutral) nullptr
	UPROPERTY()
	TObjectPtr<UTexture2D>	ElementIcon2 = { nullptr };
};