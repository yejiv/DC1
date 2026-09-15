// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DC1PalDisplayTypes.generated.h"

class UTexture2D;

/**
 * PalStatus 위젯에 넘기는 표시용 값. VM이 PalInstance + PalData로 가공해서 채운다.
 * VM 헤더와 분리한 건 PalStatus가 VM을 몰라도 되게 하기 위함
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