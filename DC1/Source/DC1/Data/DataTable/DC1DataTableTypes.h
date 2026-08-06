// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "DC1DataTableTypes.generated.h"

USTRUCT(BlueprintType)
struct FDC1LoadingTipRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Tip;
};