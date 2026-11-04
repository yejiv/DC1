// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Base/DC1BaseRegistryComponent.h"

#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Actor/BuildActor/Interact/Chest/DC1ChestActor.h"
#include "Component/Inventory/Chest/DC1ChestInventoryComponent.h"

#include "Subsystem/DC1BuildDestroySubsystem.h"

#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"

UDC1BaseRegistryComponent::UDC1BaseRegistryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1BaseRegistryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BaseChests);
}

void UDC1BaseRegistryComponent::GetChestInventories(TArray<UDC1InventoryComponentBase*>& OutInventories) const
{
	for (const TWeakObjectPtr<ADC1ChestActor>& WeakChest : BaseChests)
	{
		ADC1ChestActor* Chest = WeakChest.Get();
		if (false == ::IsValid(Chest))
		{
			continue;
		}

		UDC1InventoryComponentBase* ChestInventoryComp = Chest->GetChestInventoryComponent();
		if (true == ::IsValid(ChestInventoryComp))
		{
			OutInventories.Add(ChestInventoryComp);
		}
	}
}

void UDC1BaseRegistryComponent::RegisterWorkTarget(IDC1WorkTargetInterface* InWorkTarget)
{
	if (nullptr == InWorkTarget)
	{
		return;
	}

	WorkTargets.AddUnique(TWeakInterfacePtr<IDC1WorkTargetInterface>(InWorkTarget));
}

void UDC1BaseRegistryComponent::CollectWorkTargets()
{
	const ADC1PalBoxActor* PalBox = GetOwner<ADC1PalBoxActor>();
	if (false == ::IsValid(PalBox))
	{
		return;
	}

	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;

		const ADC1BuildActor* Build = Cast<ADC1BuildActor>(Actor);
		if (nullptr != Build && true == Build->IsPreview())
		{
			continue;
		}

		IDC1WorkTargetInterface* WorkTarget = Cast<IDC1WorkTargetInterface>(Actor);
		if (nullptr == WorkTarget || false == PalBox->IsInBase(Actor->GetActorLocation()))
		{
			continue;
		}

		RegisterWorkTarget(WorkTarget);
	}
}

void UDC1BaseRegistryComponent::RegisterBaseRequiredBuild(ADC1BuildActor* InBuild)
{
	if (ROLE_Authority != GetOwnerRole() || false == ::IsValid(InBuild))
	{
		return;
	}

	BaseRequiredBuilds.RemoveAll([](const TWeakObjectPtr<ADC1BuildActor>& Build)
		{
			return (false == Build.IsValid());
		});

	BaseRequiredBuilds.AddUnique(InBuild);
}

void UDC1BaseRegistryComponent::DestroyBaseRequiredBuilds(APlayerState* InDestroyer)
{
	UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return;
	}

	UDC1BuildDestroySubsystem* BuildDestroySubsystem = World->GetSubsystem<UDC1BuildDestroySubsystem>();
	if (false == ::IsValid(BuildDestroySubsystem))
	{
		return;
	}

	for (const TWeakObjectPtr<ADC1BuildActor>& Build : BaseRequiredBuilds)
	{
		ADC1BuildActor* BuildActor = Build.Get();
		if (false == ::IsValid(BuildActor))
		{
			continue;
		}

		BuildDestroySubsystem->RequestDestroy(BuildActor, InDestroyer);
	}
}

void UDC1BaseRegistryComponent::RegisterChest(ADC1ChestActor* InChest)
{
	if (ROLE_Authority != GetOwnerRole() || false == ::IsValid(InChest))
	{
		return;
	}

	// 건설 완성 전
	if (false == InChest->IsCompleted())
	{
		return;
	}

	UDC1ChestInventoryComponent* Temp = InChest->GetChestInventoryComponent();
	if (false == ::IsValid(Temp) || false == Temp->CanStoreItems())
	{
		return;
	}

	BaseChests.RemoveAll([](const TWeakObjectPtr<ADC1ChestActor>& Chest)
		{
			return (false == Chest.IsValid());
		});

	BaseChests.AddUnique(InChest);
}

void UDC1BaseRegistryComponent::CollectChest()
{
	const ADC1PalBoxActor* PalBox = GetOwner<ADC1PalBoxActor>();
	if (false == ::IsValid(PalBox))
	{
		return;
	}

	for (TActorIterator<ADC1ChestActor> It(GetWorld()); It; ++It)
	{
		ADC1ChestActor* Chest = *It;
		if (false == ::IsValid(Chest))
		{
			continue;
		}

		if (false == PalBox->IsInBase(Chest->GetActorLocation()))
		{
			continue;
		}

		RegisterChest(Chest);
	}
}
