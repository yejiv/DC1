
#include "Subsystem/DC1BuildDestroySubsystem.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Define/DC1BuildSupportHelper.h"
#include "Item/Pickup/DC1PickupItem.h"

#include "GameFramework/PlayerState.h"

void UDC1BuildDestroySubsystem::RequestDestroy(ADC1BuildActor* InTarget, APlayerState* InInstigator, EDC1BuildRefundMode InRootRefund)
{
	if (false == ::IsValid(InTarget) || false == InTarget->HasAuthority() || true == ClaimedActors.Contains(InTarget))
	{
		return;
	}

	// 첫 파괴는 바로 재료 반환
	RefundBuild(InTarget, InRootRefund, InInstigator);

	const int32 CascadeID = NextCascadeID++;
	FDC1DestroyCascade& NewCascade = Cascades.Add(CascadeID);
	NewCascade.Instigator = InInstigator;
	NewCascade.RootActor = InTarget;

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

		Actor->SetDestroyInstigator(Cascade->Instigator.Get());

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
	FDC1DestroyCascade* Cascade = Cascades.Find(InCascadeID);

	if (true == ::IsValid(InActor))
	{
		if (nullptr != Cascade && Cascade->RootActor.Get() != InActor)
		{
			// 연쇄 파괴인 경우 그 자리에 아이템 반환
			RefundBuild(InActor, EDC1BuildRefundMode::AtLocation, nullptr);
		}

		InActor->OnDestroyFinished.RemoveAll(this);
		InActor->BuildDestroy();
	}

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

void UDC1BuildDestroySubsystem::RefundBuild(const ADC1BuildActor* InActor, EDC1BuildRefundMode InMode, APlayerState* InInstigator) const
{
	UWorld* World = GetWorld();
	if (EDC1BuildRefundMode::None == InMode || false == ::IsValid(InActor) || nullptr == World)
	{
		return;
	}

	const FVector BuildLocation = InActor->GetActorLocation() + FVector::UpVector * 50.f;

	UDC1PlayerInventoryComponent* Inventory = nullptr;
	FVector OverflowLocation = BuildLocation;

	if (EDC1BuildRefundMode::ToInstigator == InMode)
	{
		const ADC1PlayerState* PlayerState = Cast<ADC1PlayerState>(InInstigator);
		Inventory = (nullptr != PlayerState) ? PlayerState->GetInventoryComponent() : nullptr;

		const APawn* Pawn = (nullptr != PlayerState) ? PlayerState->GetPawn() : nullptr;
		if (true == ::IsValid(Pawn))
		{
			OverflowLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 100.f + FVector::UpVector * 50.f;
		}
	}

	for (const FDC1ItemCost& Cost : InActor->GetBuildActorData().BuildCosts)
	{
		if (Cost.Count <= 0)
		{
			continue;
		}

		const int32 RemainCount = ::IsValid(Inventory) ? Inventory->AddItemPartial(Cost.ItemID, Cost.Count) : Cost.Count;
		if (RemainCount <= 0)
		{
			continue;
		}

		ADC1PickupItem::SpawnDrop(World, Cost.ItemID, RemainCount, OverflowLocation);
	}
}
