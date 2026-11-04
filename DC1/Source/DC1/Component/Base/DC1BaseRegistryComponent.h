// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/DC1WorkTargetInterface.h"
#include "UObject/WeakInterfacePtr.h"
#include "DC1BaseRegistryComponent.generated.h"

class ADC1BuildActor;
class APlayerState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1BaseRegistryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1BaseRegistryComponent();

public:
	const TArray<TWeakInterfacePtr<IDC1WorkTargetInterface>>&	GetWorkTargets() const { return WorkTargets; }
	void							RegisterWorkTarget(IDC1WorkTargetInterface* InWorkTarget);
	void							CollectWorkTargets();

	void							RegisterBaseRequiredBuild(ADC1BuildActor* InBuild);
	void							DestroyBaseRequiredBuilds(APlayerState* InDestroyer);

private:
	TArray<TWeakInterfacePtr<IDC1WorkTargetInterface>>	WorkTargets;

	TArray<TWeakObjectPtr<ADC1BuildActor>>				BaseRequiredBuilds;
};
