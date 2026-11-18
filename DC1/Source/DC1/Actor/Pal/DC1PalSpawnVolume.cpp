// Fill out your copyright notice in the Description page of Project Settings.

#include "Actor/Pal/DC1PalSpawnVolume.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Components/BoxComponent.h"
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
	for (const TWeakObjectPtr<ADC1PalCharacter>& Pal : SpawnedPals)
	{
		Count += Pal.IsValid() ? 1 : 0;
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
	// 플레이어 주변 실제 스폰 반경보다 멀리서 볼륨만 활성화되면,
	// Active=Yes 인데도 볼륨 내부 후보를 절대 만들 수 없는 구간이 생긴다.
	const float FinalMinSpawnDistance = FMath::Max(0.f, MinSpawnDistance);
	const float FinalMaxSpawnDistance = FMath::Max(FinalMinSpawnDistance, MaxSpawnDistance);
	const float EffectiveActivationDistance = FMath::Min(ActivationDistance, FinalMaxSpawnDistance);
	const float ActivationDistanceSq = FMath::Square(EffectiveActivationDistance);
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

bool ADC1PalSpawnVolume::FindSpawnLocation(FVector& OutLocation) const
{
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
			return true;
		}

		return false;
	}

	const float FinalMinDistance = FMath::Max(0.f, MinSpawnDistance);
	const float FinalMaxDistance = FMath::Max(FinalMinDistance, MaxSpawnDistance);
	const float EffectiveActivationDistance = FMath::Min(ActivationDistance, FinalMaxDistance);

	TArray<const APawn*> NearbyPlayers;
	const FBox WorldBounds = SpawnBounds->Bounds.GetBox();
	for (FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
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
		if (FVector::DistSquared2D(PlayerLocation, ClosestPoint) <= FMath::Square(EffectiveActivationDistance))
		{
			NearbyPlayers.Add(Pawn);
		}
	}

	if (0 == NearbyPlayers.Num())
	{
		return false;
	}

	const FTransform BoundsTransform = SpawnBounds->GetComponentTransform();
	const FVector UnscaledExtent = SpawnBounds->GetUnscaledBoxExtent();
	const int32 FinalAttemptCount = FMath::Max(32, SpawnLocationAttempts);
	for (int32 Attempt = 0; Attempt < FinalAttemptCount; ++Attempt)
	{
		const APawn* Player = NearbyPlayers[FMath::RandRange(0, NearbyPlayers.Num() - 1)];
		FNavLocation NavLocation;
		if (false == Navigation->GetRandomReachablePointInRadius(Player->GetActorLocation(), FinalMaxDistance, NavLocation))
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

		OutLocation = NavLocation.Location;
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
	if (false == FindSpawnLocation(SpawnLocation))
	{
		return nullptr;
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
	SpawnedPals.Add(WeakPal);
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
	SpawnedPals.RemoveAll([](const TWeakObjectPtr<ADC1PalCharacter>& Pal) { return false == Pal.IsValid(); });
}

void ADC1PalSpawnVolume::CullDistantPals()
{
	CompactSpawnedPals();
	for (int32 Index = SpawnedPals.Num() - 1; Index >= 0; --Index)
	{
		ADC1PalCharacter* Pal = SpawnedPals[Index].Get();
		if (nullptr != Pal && false == IsAnyPlayerWithin(Pal->GetActorLocation(), DespawnDistance))
		{
			SpawnedPals.RemoveAtSwap(Index);
			Pal->Destroy();
		}
	}
}
