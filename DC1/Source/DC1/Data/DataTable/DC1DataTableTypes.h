// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
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
	// 해당 테크 Tag - ex) Build_Crafting_CraftAndRepair_WorkBench00
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag TechTag;

	// 활성화 및 배치될 레벨 ( 플레이어 레벨 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 RequiredLevel;

	// 테크 이름 - ex) 원시적인 작업대
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	// 분류 이름 - ex) 건축물, 아이템
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText CategoryName;

	// Icon Image
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	// 해금 비용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 UnlockCost;
};

USTRUCT(BlueprintType)
struct FDC1ItemCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Count;
};

USTRUCT(BlueprintType)
struct FDC1BuildRow : public FTableRowBase
{
	GENERATED_BODY()

	// 해금 TechTag - 1:N 관계 ( ex) TechTag = Build_Building_Wood 인 경우 해금되는 Build는 Build_Building_Wood_Floor, Build_Building_Wood_Wall 등등 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag RequiredTechTag;

	//	4단계 고정 - ex) Build_Crafting_CraftAndRepair_WorkBench00
	//	Page = Build.Crafing 
	//	Row = BuildCrafting.CraftAndRepair
	//	Slot = Build_Crafting_CraftAndRepair_WorkBench00
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag BuildTag;

	// Row 내부 정렬 순서
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SortOrder;

	// Build 비용
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDC1ItemCost> BuildCosts;

	// Build 시간 ( 작업량 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float RequiredWork;

	// Mouse Hover UI 전용 Name
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText DisplayName;

	// Icon Image
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	// Build Actor
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<AActor> BuildActorClass;
};

USTRUCT(BlueprintType)
struct FDC1ItemRow : public FTableRowBase
{
	GENERATED_BODY()

	// DataTable에서 특정 테크를 찾기 위한 Key
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag CategoryTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText	ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1ItemRarity ItemRarity;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Weight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 MaxStack = 9999;

	// 귀중품 여부 ( true면 귀중품 탭 전용 - 상자 보관/버리기 불가, 무게 제외, 스택 제한 없음 )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsValuable = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Description = FText::FromString(TEXT("설명 텍스트"));

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> ItemIcon;
};