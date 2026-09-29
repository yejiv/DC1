
#include "Subsystem/DC1BuildDestroySubsystem.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Define/DC1BuildSupportHelper.h"

void UDC1BuildDestroySubsystem::RequestDestroy(ADC1BuildActor* InTarget)
{
	if (false == ::IsValid(InTarget) || false == InTarget->HasAuthority() || true == ClaimedActors.Contains(InTarget))
	{
		return;
	}

	const int32 CascadeID = NextCascadeID++;
	Cascades.Add(CascadeID);

	StartDestroyWave({ InTarget }, CascadeID);
}

void UDC1BuildDestroySubsystem::StartDestroyWave(const TArray<ADC1BuildActor*>& InActors, int32 InCascadeID)
{
	FDC1DestroyCascade* Cascade = Cascades.Find(InCascadeID);
	if (nullptr == Cascade)
	{
		return;
	}

	Cascade->NextCandidates.Reset();
	Cascade->RemainingCount = 0;

	for (ADC1BuildActor* Actor : InActors)
	{
		if (false == ::IsValid(Actor) || true == ClaimedActors.Contains(Actor))
		{
			continue;
		}

		ClaimedActors.Add(Actor);
		Cascade->Claimed.Add(Actor);

		TArray<ADC1BuildActor*> Neighbors;
		DC1BuildSupportHelper::GatherNeighbors(Actor, Neighbors);

		for (ADC1BuildActor* Neighbor : Neighbors)
		{
			Cascade->NextCandidates.AddUnique(Neighbor);
		}

		Actor->OnDestroyFinished.AddUObject(this, &ThisClass::HandleDestroyFinished, InCascadeID);

		++(Cascade->RemainingCount);

		Actor->BeginDestroySequence();
	}

	if (Cascade->RemainingCount <= 0)
	{
		FinishCascade(InCascadeID);
	}
}

void UDC1BuildDestroySubsystem::HandleDestroyFinished(ADC1BuildActor* InActor, int32 InCascadeID)
{
	if (true == ::IsValid(InActor))
	{
		InActor->OnDestroyFinished.RemoveAll(this);
		InActor->BuildDestroy();
	}

	FDC1DestroyCascade* Cascade = Cascades.Find(InCascadeID);
	if (nullptr == Cascade)
	{
		return;
	}

	--(Cascade->RemainingCount);

	if (Cascade->RemainingCount > 0)
	{
		return;
	}

	TArray<ADC1BuildActor*> NextWave;
	TSet<ADC1BuildActor*>	Visited;
	TArray<ADC1BuildActor*>	Pending;

	for (TWeakObjectPtr<ADC1BuildActor>& Next : Cascade->NextCandidates)
	{
		ADC1BuildActor* NextActor = Next.Get();
		if (true == ::IsValid(NextActor))
		{
			Pending.Add(NextActor);
		}
	}

	while (false == Pending.IsEmpty())
	{
		ADC1BuildActor* Current = Pending.Pop();

		if (false == ::IsValid(Current) || true == ClaimedActors.Contains(Current) || true == Visited.Contains(Current))
		{
			continue;
		}

		Visited.Add(Current);

		if (false == DC1BuildSupportHelper::HasSupport(Current))
		{
			NextWave.Add(Current);
			continue;
		}

		TArray<ADC1BuildActor*> Neighbors;
		DC1BuildSupportHelper::GatherNeighbors(Current, Neighbors);

		Pending.Append(Neighbors);
	}

	if (true == NextWave.IsEmpty())
	{
		FinishCascade(InCascadeID);
	}
	else
	{
		StartDestroyWave(NextWave, InCascadeID);
	}
}

void UDC1BuildDestroySubsystem::FinishCascade(int32 InCascadeID)
{
	FDC1DestroyCascade* Cascade = Cascades.Find(InCascadeID);
	if (nullptr == Cascade)
	{
		return;
	}

	for (TWeakObjectPtr<ADC1BuildActor>& Claim : Cascade->Claimed)
	{
		ClaimedActors.Remove(Claim);
	}

	Cascades.Remove(InCascadeID);
}
