// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "Data/Appearance/DC1AppearanceTypes.h"

#include "DC1CustomizationSaveGame.generated.h"

// 클라 로컬 전용. 서버로는 절대 저장하지 않는다 (외형은 어느 서버에서든 동일하게 쓰는 개인 데이터)
UCLASS()
class DC1_API UDC1CustomizationSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FDC1AppearanceData Appearance = {};
};
