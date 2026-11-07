// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"
#include "DC1CraftBuildActor.generated.h"

struct FDC1ItemRecipeRow;

class ADC1PlayerState;

UCLASS()
class DC1_API ADC1CraftBuildActor : public ADC1ProductionBuildActor
{
	GENERATED_BODY()

public:
	ADC1CraftBuildActor();

public:
	int32			GetMaxCraftCount() const { return MaxCraftCount; }

public:
	// 클라 목록 표시 판정 + 서버 검증
	bool			CanShowRecipe(const FDC1ItemRecipeRow& InRecipe, const ADC1PlayerState* InPlayerState) const;
	
	int32			GetCraftableCount(const FDC1ItemRecipeRow& InRecipe, const ADC1PlayerState* InPlayerState) const;
	void			RegisterRecipe(ADC1Character* InRegister, FName InRecipeID, int32 InCraftCount);

	void			CancelRecipe(ADC1Character* InCanceller);

	virtual void	GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;

private:
	virtual void	OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;

	virtual void	OnProductCollected(int32 InCollectedCount) override;

private:
	void			ClearRecipeOrder();

private:
	// 제작 중이 아닐때 변경할 옵션(F)
	UPROPERTY(EditDefaultsOnly, Category = "Production|Interact")
	TArray<FDC1InteractOption>		RecipeOptions;

	// 제작 중일때 추가할 취소 옵션(C)
	UPROPERTY(EditDefaultsOnly, Category = "Production|Interact")
	TArray<FDC1InteractOption>		CancelOptions;

	// 제작 가능 타입들
	UPROPERTY(EditDefaultsOnly, Category = "Production|Recipe", meta = (Categories = "Item.Type"))
	FGameplayTagContainer			CraftableCategoryTags;
	// 제작 가능 등급 ( 제작 타입 + 랭크로 제작대 등급 구분 )
	UPROPERTY(EditDefaultsOnly, Category = "Production|Recipe", meta = (ClampMin = "1"))
	int32							CraftableRank = { 1 };
	// 제작 최대 개수
	UPROPERTY(EditDefaultsOnly, Category = "Production|Recipe", meta = (ClampMin = "1", ClampMax = "9999"))
	int32							MaxCraftCount = { 9999 };

private:
	FName							CurrentRecipeID = { NAME_None };
};
