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
	bool FindSpawnLocation(FVector& OutLocation, int32& OutCellIndex) const;
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

	// 기존 BP 직렬화 호환용. 일반 스폰은 이제 플레이어 거리가 아니라 볼륨 전체 그리드를 사용한다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distance", meta = (ClampMin = "0.0", Units = "cm"))
	float MaxSpawnDistance = { 7000.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distance", meta = (ClampMin = "0.0", Units = "cm"))
	float DespawnDistance = { 18000.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Timing", meta = (ClampMin = "0.0", Units = "s"))
	float RespawnDelay = { 60.f };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn", meta = (ClampMin = "1"))
	int32 SpawnLocationAttempts = { 15 };

	// 최초 배치 셀을 기준으로 적은 셀부터 채운다. 팰이 배회해도 소속 셀은 바뀌지 않는다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distribution", meta = (ClampMin = "1", ClampMax = "8"))
	int32 SpawnGridColumns = { 3 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distribution", meta = (ClampMin = "1", ClampMax = "8"))
	int32 SpawnGridRows = { 3 };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Distribution", meta = (ClampMin = "0.0", Units = "cm"))
	float MinPalSpawnDistance = { 800.f };

private:
	// Value는 최초 스폰된 그리드 셀 번호다. 현재 위치가 바뀌어도 갱신하지 않는다.
	TMap<TWeakObjectPtr<ADC1PalCharacter>, int32> SpawnedPals;
	double NextSpawnTime = { 0.0 };

	// 서버 시작 시 MinAlive~MaxAlive에서 한 번 뽑고, 이 수까지 계속 보충한다.
	int32 TargetAliveCount = { 0 };
};
