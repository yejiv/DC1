// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DC1GameMode.generated.h"

class ADC1PickupItem;
class ADC1PlayerState;
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
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void StartPlay() override;

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	TSubclassOf<ADC1PickupItem>	GetPickupClass() const { return PickupClass; }

	// 기존 Pawn을 지우고 새 Pawn으로 부활. InSpawnTransform이 없으면 시작 지점 ( 서버 )
	bool						RespawnPlayer(AController* InController, const FTransform* InSpawnTransform);

private:
	void						LoadPlayer(ADC1PlayerState* InPS);

private:
	// 부활 시 체력 비율 ( MaxHealth 기준 )
	UPROPERTY(EditDefaultsOnly, Category = "Respawn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float						RespawnHealthRatio = { 0.1f };

	// 월드 드랍 픽업 클래스 - 메쉬는 아이템 행의 WorldMesh가 정하므로 하나면 된다 ( 스폰은 서버 전용 = GameMode가 있는 곳 )
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TSubclassOf<ADC1PickupItem>	PickupClass = { nullptr };

private:
	UPROPERTY()
	TArray<TObjectPtr<ADC1PlayerState>> PendingLoadedStates;
};
