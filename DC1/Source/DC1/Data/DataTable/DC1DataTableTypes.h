// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Enum/DC1Enums.h"
#include "DC1DataTableTypes.generated.h"

USTRUCT(BlueprintType)
struct FDC1LoadingTipRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = "true"))
	FText Tip;
};

USTRUCT(BlueprintType)
struct FDC1TechRow : public FTableRowBase
{
	GENERATED_BODY()

	// DataTable에서 특정 테크를 찾기 위한 Key
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName TechID;

	// 해당 테크가 어떤 분류에 속하는지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CategoryTag;

	// 활성화 및 배치될 레벨 ( 플레이어 레벨 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredLevel;

	// 테크 이름 - ex) 원시적인 작업대
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText	DisplayName;

	// 분류 이름 - ex) 건축물, 아이템
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText CategoryName;
};

USTRUCT(BlueprintType)
struct FDC1ItemRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1ItemType ItemType = EDC1ItemType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1ItemRarity ItemRarity = EDC1ItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStackCount = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemIcon;
};