// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DC1GameplayEffect_EquipStat.generated.h"

/*
장비 한 칸이 플레이어에게 더하는 스탯
값은 SetByCaller로 ( Infinite )
 */
UCLASS()
class DC1_API UDC1GameplayEffect_EquipStat : public UGameplayEffect
{
	GENERATED_BODY()
	
public:
	UDC1GameplayEffect_EquipStat(const FObjectInitializer& ObjectInitializer);
};
