// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Base/DC1BaseRegistryComponent.h"

#include "Actor/BuildActor/DC1BuildActor.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "Subsystem/DC1BuildDestroySubsystem.h"

#include "EngineUtils.h"

UDC1BaseRegistryComponent::UDC1BaseRegistryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
