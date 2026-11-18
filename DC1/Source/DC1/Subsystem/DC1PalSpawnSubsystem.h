// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DC1PalSpawnSubsystem.generated.h"

class ADC1PalSpawnVolume;

/** 서버에서 야생 팰 스폰 볼륨과 월드 전체 개체 예산을 관리한다. */
UCLASS()
class DC1_API UDC1PalSpawnSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	void RegisterVolume(ADC1PalSpawnVolume* InVolume);
	void UnregisterVolume(ADC1PalSpawnVolume* InVolume);

private:
	void UpdateSpawns();
	int32 GetTotalLivingPalCount() const;

private:
	TArray<TWeakObjectPtr<ADC1PalSpawnVolume>> Volumes;
	float UpdateAccumulator = { 0.f };
	int32 NextVolumeIndex = { 0 };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Spawn", meta = (ClampMin = "1"))
	int32 GlobalPalBudget = { 60 };

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Spawn", meta = (ClampMin = "0.05", Units = "s"))
	float UpdateInterval = { 0.25f };
};
