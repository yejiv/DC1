// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "DC1PlayerExpData.generated.h"

// 플레이어 경험치 표 행. 행 이름 = 레벨
USTRUCT(BlueprintType)
struct FDC1PlayerExpRow : public FTableRowBase
{
	GENERATED_BODY()

	// 이전 레벨 → 이 레벨 필요량
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0"))
	int32	PlayerNextExp = { 0 };
};

/**
 * 플레이어 레벨업 수치
 */
UCLASS()
class DC1_API UDC1PlayerExpData : public UDataAsset
{
	GENERATED_BODY()

public:
	// 다음 레벨 필요량. 최대 레벨이면 0
	int32		GetExpToNextLevel(int32 InLevel) const;

public:
	// 레벨별 경험치 표
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Exp", meta = (RequiredAssetDataTags = "RowStructure=/Script/DC1.DC1PlayerExpRow"))
	TObjectPtr<UDataTable>	ExpTable = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Exp", meta = (ClampMin = "1"))
	int32					MaxLevel = { 50 };

	// 획득 경험치 배율
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Exp", meta = (ClampMin = "0.0"))
	float					ExpRate = { 1.f };

	// 레벨당 테크 포인트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player|Level", meta = (ClampMin = "0"))
	int32					TechPointsPerLevel = { 5 };
};
