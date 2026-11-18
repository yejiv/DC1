// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/DC1PalSpawnSubsystem.h"

#include "Actor/Pal/DC1PalSpawnVolume.h"
#include "Engine/World.h"

void UDC1PalSpawnSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	if (nullptr == World || NM_Client == World->GetNetMode())
	{
		return;
	}

	UpdateAccumulator += DeltaTime;
	if (UpdateAccumulator < UpdateInterval)
	{
		return;
	}

	UpdateAccumulator = FMath::Fmod(UpdateAccumulator, UpdateInterval);
	UpdateSpawns();
}

TStatId UDC1PalSpawnSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDC1PalSpawnSubsystem, STATGROUP_Tickables);
}

bool UDC1PalSpawnSubsystem::DoesSupportWorldType(EWorldType::Type WorldType) const
{
	return EWorldType::Game == WorldType || EWorldType::PIE == WorldType;
}

void UDC1PalSpawnSubsystem::RegisterVolume(ADC1PalSpawnVolume* InVolume)
{
	if (IsValid(InVolume))
	{
		Volumes.AddUnique(InVolume);
	}
}

void UDC1PalSpawnSubsystem::UnregisterVolume(ADC1PalSpawnVolume* InVolume)
{
	Volumes.Remove(InVolume);
}

int32 UDC1PalSpawnSubsystem::GetTotalLivingPalCount() const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<ADC1PalSpawnVolume>& Volume : Volumes)
	{
		if (Volume.IsValid())
		{
			Count += Volume->GetLivingPalCount();
		}
	}
	return Count;
}

void UDC1PalSpawnSubsystem::UpdateSpawns()
{
	Volumes.RemoveAll([](const TWeakObjectPtr<ADC1PalSpawnVolume>& Volume) { return false == Volume.IsValid(); });
	for (const TWeakObjectPtr<ADC1PalSpawnVolume>& Volume : Volumes)
	{
		Volume->CullDistantPals();
	}

	if (0 == Volumes.Num() || GetTotalLivingPalCount() >= GlobalPalBudget)
	{
		return;
	}

	const double WorldTime = GetWorld()->GetTimeSeconds();
	for (int32 Offset = 0; Offset < Volumes.Num(); ++Offset)
	{
		const int32 Index = (NextVolumeIndex + Offset) % Volumes.Num();
		ADC1PalSpawnVolume* Volume = Volumes[Index].Get();
		if (nullptr != Volume && Volume->HasPlayerInActivationRange() &&
			Volume->GetLivingPalCount() < Volume->GetDesiredPalCount() && Volume->CanSpawnNow(WorldTime))
		{
			Volume->SpawnOne();
			NextVolumeIndex = (Index + 1) % Volumes.Num();
			return;
		}
	}
}
