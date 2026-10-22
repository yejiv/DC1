// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DC1GameMode.generated.h"

class ADC1PickupItem;

/**
 * 
 */
UCLASS()
class DC1_API ADC1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ADC1GameMode();

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	TSubclassOf<ADC1PickupItem>	GetPickupClass() const { return PickupClass; }

private:
	// 월드 드랍 픽업 클래스 - 메쉬는 아이템 행의 WorldMesh가 정하므로 하나면 된다 ( 스폰은 서버 전용 = GameMode가 있는 곳 )
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<ADC1PickupItem>	PickupClass = { nullptr };
};
