// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
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
	FGameplayTag ClassificationTag;

	// 활성화 및 배치될 레벨 ( 플레이어 레벨 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredLevel;

	// 테크 이름 - ex) 원시적인 작업대
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText	DisplayName;

	// 분류 이름 - ex) 건축물, 아이템
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ClassificationName;
};