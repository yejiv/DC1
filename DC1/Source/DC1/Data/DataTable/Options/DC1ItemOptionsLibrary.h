// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/PropertyText.h"
#include "DC1ItemOptionsLibrary.generated.h"

struct FDC1ItemRow;

// 레시피 테이블 편집용 ItemID 드롭다운 / 아이템 이름 조회 - 에디터 전용, 게임 로직에선 안 씀
UCLASS()
class DC1_API UDC1ItemOptionsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// ProductItemID, RequiredItems 드롭다운 ( 값 6101 / 표시 "6101  목재" )
	UFUNCTION()
	static TArray<FPropertyTextFName> GetItemIDOptions();

	// RequiredSchematicItemID 드롭다운 ( None + 설계도 )
	UFUNCTION()
	static TArray<FPropertyTextFName> GetSchematicItemIDOptions();

	UFUNCTION()
	static TArray<FPropertyTextFName> GetFoodItemIDOptions();

#if WITH_EDITOR
	// 레시피 요약에 이름 표시용
	static const FDC1ItemRow* FindItemRowInEditor(FName InItemID);
#endif
};
