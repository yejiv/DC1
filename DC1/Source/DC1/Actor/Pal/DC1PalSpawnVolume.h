// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "DC1PalSpawnVolume.generated.h"

class ADC1PalCharacter;
class UBoxComponent;

USTRUCT(BlueprintType)
struct FDC1PalSpawnEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (Categories = "Pal.Species"))
	FGameplayTag SpeciesTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0.0"))
	float Weight = { 1.f };
};

/** 맵에 배치하는 야생 팰 스폰 영역. 실제 스폰 시점과 전체 예산은 PalSpawnSubsystem이 관리한다. */
UCLASS(Blueprintable)
class DC1_API ADC1PalSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	ADC1PalSpawnVolume();

	int32 GetLivingPalCount() const;
	int32 GetDesiredPalCount() const;
	bool HasPlayerInActivationRange() const;
	bool CanSpawnNow(double InWorldTime) const;
	ADC1PalCharacter* SpawnOne();
	void CullDistantPals();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	const FDC1PalSpawnEntry* PickSpawnEntry() const;
	bool FindSpawnLocation(FVector& OutLocation) const;
	bool IsAnyPlayerWithin(const FVector& InLocation, float InDistance) const;
	void HandleSpawnedPalRemoved(TWeakObjectPtr<ADC1PalCharacter> InPal);
	void CompactSpawnedPals();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UBoxComponent> SpawnBounds = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	TArray<FDC1PalSpawnEntry> SpawnEntries;

	// 한 마리를 스포너 중심의 고정 지점에 생성한다. 일반 지역 스폰은 기존 플레이어 주변 방식을 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Boss")
	bool bBossSpawner = { false };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Boss", meta = (EditCondition = "bBossSpawner"))
	bool bSpawnAsAlpha = { true };

	// 스포너 트랜스폼 기준 로컬 오프셋. 최종 위치는 NavMesh 위로 투영한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Boss", meta = (EditCondition = "bBossSpawner", Units = "cm"))
	FVector BossSpawnOffset = { FVector::ZeroVector };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "0"))
	int32 MinAlive = { 3 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 MaxAlive = { 8 };

	// 이 지역에서 스폰되는 야생 팰의 레벨 범위.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Level", meta = (ClampMin = "1"))
	int32 MinSpawnLevel = { 1 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Level", meta = (ClampMin = "1"))
	int32 MaxSpawnLevel = { 1 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distance", meta = (ClampMin = "0.0", Units = "cm"))
	float ActivationDistance = { 12000.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distance", meta = (ClampMin = "0.0", Units = "cm"))
	float MinSpawnDistance = { 3000.f };

	// 활성 플레이어 주변에서 스폰할 최대 거리. 큰 지역 볼륨에서도 팰이 멀리 흩어지지 않게 한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distance", meta = (ClampMin = "0.0", Units = "cm"))
	float MaxSpawnDistance = { 7000.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distance", meta = (ClampMin = "0.0", Units = "cm"))
	float DespawnDistance = { 18000.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float RespawnDelay = { 60.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnLocationAttempts = { 12 };

private:
	TArray<TWeakObjectPtr<ADC1PalCharacter>> SpawnedPals;
	double NextSpawnTime = { 0.0 };

	// 서버 시작 시 MinAlive~MaxAlive에서 한 번 뽑고, 이 수까지 계속 보충한다.
	int32 TargetAliveCount = { 0 };
};
