// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Pal/DC1PalSpawnVolume.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "Subsystem/DC1PalSpawnSubsystem.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

ADC1PalSpawnVolume::ADC1PalSpawnVolume()
{
	SpawnBounds = CreateDefaultSubobject<UBoxComponent>(TEXT("SpawnBounds"));
	SetRootComponent(SpawnBounds);
	SpawnBounds->SetBoxExtent(FVector(2000.f, 2000.f, 500.f));
	SpawnBounds->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnBounds->SetCanEverAffectNavigation(false);
}

void ADC1PalSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (true == bBossSpawner)
		{
			TargetAliveCount = 1;
		}
		else
		{
			const int32 FinalMinAlive = FMath::Max(0, MinAlive);
			const int32 FinalMaxAlive = FMath::Max(FinalMinAlive, MaxAlive);
			TargetAliveCount = FMath::RandRange(FinalMinAlive, FinalMaxAlive);
		}

		if (UDC1PalSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UDC1PalSpawnSubsystem>())
		{
			Subsystem->RegisterVolume(this);
		}
	}
}

void ADC1PalSpawnVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1PalSpawnSubsystem* Subsystem = GetWorld()->GetSubsystem<UDC1PalSpawnSubsystem>())
	{
		Subsystem->UnregisterVolume(this);
	}

	Super::EndPlay(EndPlayReason);
}

int32 ADC1PalSpawnVolume::GetLivingPalCount() const
{
	int32 Count = 0;
	for (const TPair<TWeakObjectPtr<ADC1PalCharacter>, int32>& Pair : SpawnedPals)
	{
		Count += Pair.Key.IsValid() ? 1 : 0;
	}
	return Count;
}

int32 ADC1PalSpawnVolume::GetDesiredPalCount() const
{
	return (true == bBossSpawner) ? 1 : ((0 < TargetAliveCount) ? TargetAliveCount : FMath::Max(0, MinAlive));
}

bool ADC1PalSpawnVolume::IsAnyPlayerWithin(const FVector& InLocation, float InDistance) const
{
	const UWorld* World = GetWorld();
	if (nullptr == World)
	{
		return false;
	}

	const float DistanceSq = FMath::Square(InDistance);
	for (FConstPlayerControllerIterator Iter = World->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		const APlayerController* PC = Iter->Get();
		const APawn* Pawn = (nullptr != PC) ? PC->GetPawn() : nullptr;
		if (nullptr != Pawn && FVector::DistSquared2D(InLocation, Pawn->GetActorLocation()) <= DistanceSq)
		{
			return true;
		}
	}

	return false;
}

bool ADC1PalSpawnVolume::HasPlayerInActivationRange() const
{
	const UWorld* World = GetWorld();
	if (nullptr == World || false == IsValid(SpawnBounds))
	{
		return false;
	}

	const FBox WorldBounds = SpawnBounds->Bounds.GetBox();
	// 플레이어는 볼륨을 활성화하기만 한다. 실제 스폰 위치는 볼륨 전체에서 고른다.
	const float ActivationDistanceSq = FMath::Square(FMath::Max(0.f, ActivationDistance));
	for (FConstPlayerControllerIterator Iter = World->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		const APlayerController* PC = Iter->Get();
		const APawn* Pawn = (nullptr != PC) ? PC->GetPawn() : nullptr;
		if (nullptr == Pawn)
		{
			continue;
		}

		const FVector PlayerLocation = Pawn->GetActorLocation();
		const FVector ClosestPoint(
			FMath::Clamp(PlayerLocation.X, WorldBounds.Min.X, WorldBounds.Max.X),
			FMath::Clamp(PlayerLocation.Y, WorldBounds.Min.Y, WorldBounds.Max.Y),
			PlayerLocation.Z);

		if (FVector::DistSquared2D(PlayerLocation, ClosestPoint) <= ActivationDistanceSq)
		{
			return true;
		}
	}

	return false;
}

bool ADC1PalSpawnVolume::CanSpawnNow(double InWorldTime) const
{
	return InWorldTime >= NextSpawnTime && GetLivingPalCount() < GetDesiredPalCount() && 0 < SpawnEntries.Num();
}

const FDC1PalSpawnEntry* ADC1PalSpawnVolume::PickSpawnEntry() const
{
	float TotalWeight = 0.f;
	for (const FDC1PalSpawnEntry& Entry : SpawnEntries)
	{
		if (Entry.SpeciesTag.IsValid())
		{
			TotalWeight += FMath::Max(0.f, Entry.Weight);
		}
	}

	if (TotalWeight <= 0.f)
	{
		return nullptr;
	}

	float Pick = FMath::FRandRange(0.f, TotalWeight);
	for (const FDC1PalSpawnEntry& Entry : SpawnEntries)
	{
		Pick -= Entry.SpeciesTag.IsValid() ? FMath::Max(0.f, Entry.Weight) : 0.f;
		if (Pick <= 0.f)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool ADC1PalSpawnVolume::FindSpawnLocation(FVector& OutLocation, int32& OutCellIndex) const
{
	OutCellIndex = INDEX_NONE;
	const UNavigationSystemV1* Navigation = UNavigationSystemV1::GetCurrent(GetWorld());
	if (nullptr == Navigation || false == IsValid(SpawnBounds))
	{
		return false;
	}

	if (true == bBossSpawner)
	{
		// XY 오프셋으로 고정 지점을 고른 뒤 NavMesh 지면을 찾고, Z는 수동 높이 보정으로 적용한다.
		const FVector FixedLocation = GetActorTransform().TransformPositionNoScale(FVector(BossSpawnOffset.X, BossSpawnOffset.Y, 0.f));
		const FVector ProjectionExtent(200.f, 200.f, SpawnBounds->GetScaledBoxExtent().Z);
		FNavLocation NavLocation;
		if (true == Navigation->ProjectPointToNavigation(FixedLocation, NavLocation, ProjectionExtent))
		{
			OutLocation = NavLocation.Location + FVector::UpVector * BossSpawnOffset.Z;
			OutCellIndex = 0;
			return true;
		}

		return false;
	}

	const float FinalMinDistance = FMath::Max(0.f, MinSpawnDistance);
	const FTransform BoundsTransform = SpawnBounds->GetComponentTransform();
	const FVector UnscaledExtent = SpawnBounds->GetUnscaledBoxExtent();
	const int32 GridColumns = FMath::Clamp(SpawnGridColumns, 1, 8);
	const int32 GridRows = FMath::Clamp(SpawnGridRows, 1, 8);
	const int32 CellCount = GridColumns * GridRows;
	TArray<int32> CellOccupancy;
	CellOccupancy.Init(0, CellCount);
	for (const TPair<TWeakObjectPtr<ADC1PalCharacter>, int32>& Pair : SpawnedPals)
	{
		if (Pair.Key.IsValid() && CellOccupancy.IsValidIndex(Pair.Value))
		{
			++CellOccupancy[Pair.Value];
		}
	}

	TArray<int32> CellCandidates;
	TArray<float> CellTieBreakers;
	CellCandidates.Reserve(CellCount);
	CellTieBreakers.Reserve(CellCount);
	for (int32 CellIndex = 0; CellIndex < CellCount; ++CellIndex)
	{
		CellCandidates.Add(CellIndex);
		CellTieBreakers.Add(FMath::FRand());
	}
	CellCandidates.Sort([&CellOccupancy, &CellTieBreakers](const int32 A, const int32 B)
	{
		return CellOccupancy[A] == CellOccupancy[B]
			? CellTieBreakers[A] < CellTieBreakers[B]
			: CellOccupancy[A] < CellOccupancy[B];
	});

	const float CellSizeX = (UnscaledExtent.X * 2.f) / GridColumns;
	const float CellSizeY = (UnscaledExtent.Y * 2.f) / GridRows;
	const float MinPalDistanceSq = FMath::Square(FMath::Max(0.f, MinPalSpawnDistance));
	// 기존 BP에 직렬화된 12회 값이 남아 있어도 최소 15회는 보장한다.
	const int32 FinalAttemptCount = FMath::Max(15, SpawnLocationAttempts);
	for (int32 Attempt = 0; Attempt < FinalAttemptCount; ++Attempt)
	{
		const int32 SelectedCellIndex = CellCandidates[Attempt % CellCandidates.Num()];
		const int32 SelectedCellX = SelectedCellIndex % GridColumns;
		const int32 SelectedCellY = SelectedCellIndex / GridColumns;
		const float LocalMinX = -UnscaledExtent.X + SelectedCellX * CellSizeX;
		const float LocalMinY = -UnscaledExtent.Y + SelectedCellY * CellSizeY;
		const FVector CandidateLocation = BoundsTransform.TransformPosition(FVector(
			FMath::FRandRange(LocalMinX, LocalMinX + CellSizeX),
			FMath::FRandRange(LocalMinY, LocalMinY + CellSizeY),
			0.f));

		FNavLocation NavLocation;
		// 월드에 배치된 볼륨의 Z 스케일까지 반영해야 높낮이가 큰 지형에서도 지면 NavMesh를 찾는다.
		const FVector ProjectionExtent(200.f, 200.f, SpawnBounds->GetScaledBoxExtent().Z);
		if (false == Navigation->ProjectPointToNavigation(CandidateLocation, NavLocation, ProjectionExtent))
		{
			continue;
		}

		const FVector LocalLocation = BoundsTransform.InverseTransformPosition(NavLocation.Location);
		const bool IsInsideBounds = FMath::Abs(LocalLocation.X) <= UnscaledExtent.X &&
			FMath::Abs(LocalLocation.Y) <= UnscaledExtent.Y && FMath::Abs(LocalLocation.Z) <= UnscaledExtent.Z;
		if (false == IsInsideBounds)
		{
			continue;
		}

		if (true == IsAnyPlayerWithin(NavLocation.Location, FinalMinDistance))
		{
			continue;
		}

		bool bIsTooCloseToPal = false;
		for (const TPair<TWeakObjectPtr<ADC1PalCharacter>, int32>& Pair : SpawnedPals)
		{
			const ADC1PalCharacter* ExistingPal = Pair.Key.Get();
			if (nullptr != ExistingPal && FVector::DistSquared2D(NavLocation.Location, ExistingPal->GetActorLocation()) < MinPalDistanceSq)
			{
				bIsTooCloseToPal = true;
				break;
			}
		}
		if (true == bIsTooCloseToPal)
		{
			continue;
		}

		const int32 CellX = FMath::Clamp(FMath::FloorToInt((LocalLocation.X + UnscaledExtent.X) / CellSizeX), 0, GridColumns - 1);
		const int32 CellY = FMath::Clamp(FMath::FloorToInt((LocalLocation.Y + UnscaledExtent.Y) / CellSizeY), 0, GridRows - 1);
		OutLocation = NavLocation.Location;
		OutCellIndex = CellY * GridColumns + CellX;
		return true;
	}

	return false;
}

ADC1PalCharacter* ADC1PalSpawnVolume::SpawnOne()
{
	if (false == HasAuthority())
	{
		return nullptr;
	}

	CompactSpawnedPals();
	const FDC1PalSpawnEntry* Entry = PickSpawnEntry();
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	const UDC1PalData* PalData = (nullptr != Entry && nullptr != Registry)
		? Registry->FindPalDataByTag(Entry->SpeciesTag).LoadSynchronous() : nullptr;
	UClass* PalClass = (nullptr != PalData) ? PalData->PalClass.LoadSynchronous() : nullptr;
	if (nullptr == PalClass)
	{
		return nullptr;
	}

	FVector SpawnLocation;
	int32 SpawnCellIndex = INDEX_NONE;
	if (false == FindSpawnLocation(SpawnLocation, SpawnCellIndex))
	{
		return nullptr;
	}

	// NavMesh 지점은 바닥이다. 캡슐 중심을 바닥 위로 올려야 땅에 묻혀 스폰이 취소되지 않는다 ( 보스는 BossSpawnOffset.Z 로 직접 맞춤 )
	if (false == bBossSpawner)
	{
		if (const ADC1PalCharacter* PalCDO = PalClass->GetDefaultObject<ADC1PalCharacter>())
		{
			SpawnLocation.Z += PalCDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		}
	}

	const FTransform SpawnTransform(FRotator(0.f, FMath::FRandRange(-180.f, 180.f), 0.f), SpawnLocation);
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = (true == bBossSpawner)
		? ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn
		: ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	ADC1PalCharacter* Pal = GetWorld()->SpawnActorDeferred<ADC1PalCharacter>(PalClass, SpawnTransform, nullptr, nullptr,
		SpawnParameters.SpawnCollisionHandlingOverride);
	if (false == IsValid(Pal))
	{
		return nullptr;
	}

	const int32 FinalMinLevel = FMath::Max(1, MinSpawnLevel);
	const int32 FinalMaxLevel = FMath::Max(FinalMinLevel, MaxSpawnLevel);
	Pal->SetInitialLevel(FMath::RandRange(FinalMinLevel, FinalMaxLevel));
	if (true == bBossSpawner)
	{
		Pal->SetInitialAlpha(bSpawnAsAlpha);
	}
	Pal->FinishSpawning(SpawnTransform);

	const TWeakObjectPtr<ADC1PalCharacter> WeakPal = Pal;
	SpawnedPals.Add(WeakPal, SpawnCellIndex);
	Pal->OnPalDied.AddWeakLambda(this, [this, WeakPal]() { HandleSpawnedPalRemoved(WeakPal); });
	return Pal;
}

void ADC1PalSpawnVolume::HandleSpawnedPalRemoved(TWeakObjectPtr<ADC1PalCharacter> InPal)
{
	SpawnedPals.Remove(InPal);
	if (const UWorld* World = GetWorld())
	{
		NextSpawnTime = World->GetTimeSeconds() + RespawnDelay;
	}
}

void ADC1PalSpawnVolume::CompactSpawnedPals()
{
	for (auto Iter = SpawnedPals.CreateIterator(); Iter; ++Iter)
	{
		if (false == Iter.Key().IsValid())
		{
			Iter.RemoveCurrent();
		}
	}
}

void ADC1PalSpawnVolume::CullDistantPals()
{
	CompactSpawnedPals();
	for (auto Iter = SpawnedPals.CreateIterator(); Iter; ++Iter)
	{
		ADC1PalCharacter* Pal = Iter.Key().Get();
		if (nullptr != Pal && false == IsAnyPlayerWithin(Pal->GetActorLocation(), DespawnDistance))
		{
			Iter.RemoveCurrent();
			Pal->Destroy();
		}
	}
}
