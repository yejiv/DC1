
#include "Subsystem/DC1TeleportSubsystem.h"

#include "Actor/Teleport/DC1TeleportActor.h"
#include "Interface/DC1TeleportPointInterface.h"

void UDC1TeleportSubsystem::RegisterPoint(AActor* InPoint)
{
	if (true == Points.Contains(InPoint))
		return;

	Points.Add(InPoint);
}

void UDC1TeleportSubsystem::UnregisterPoint(AActor* InPoint)
{
	if (false == Points.Contains(InPoint))
		return;

	Points.Remove(InPoint);
}

void UDC1TeleportSubsystem::GetUsablePoints(const APlayerState* InPlayerState, TArray<AActor*>& OutPoints) const
{
	OutPoints.Reserve(Points.Num());

	for (const auto& Actor : Points)
	{
		IDC1TeleportPointInterface* TeleportActor = Cast<IDC1TeleportPointInterface>(Actor.Get());
		if (nullptr == TeleportActor)
			continue;

		if (true == TeleportActor->CanUse(InPlayerState))
			OutPoints.Add(Actor.Get());
	}
}

int32 UDC1TeleportSubsystem::GetTeleportActorNum()
{
	int32 TeleportActorCount = {};

	for (const auto& Actor : Points)
	{
		const IDC1TeleportPointInterface* Interface = Cast<const IDC1TeleportPointInterface>(Actor.Get());
		if (nullptr == Interface)
			continue;

		if (EDC1TeleportPointType::Teleport == Interface->GetPointType())
			++TeleportActorCount;
	}

	return TeleportActorCount;
}

ADC1TeleportActor* UDC1TeleportSubsystem::FindTeleportActorById(FName InPointId) const
{
	for (const auto& Actor : Points)
	{
		ADC1TeleportActor* TeleportActor = Cast<ADC1TeleportActor>(Actor.Get());
		if (nullptr == TeleportActor)
			continue;

		if (true == TeleportActor->GetPointId().IsEqual(InPointId))
		{
			return TeleportActor;
		}
	}

	return nullptr;
}

void UDC1TeleportSubsystem::NotifyPointsChanged()
{
	OnPointsChanged.Broadcast();
}
