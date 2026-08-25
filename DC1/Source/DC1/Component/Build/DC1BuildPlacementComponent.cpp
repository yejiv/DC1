// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildPlacementComponent.h"

// Sets default values for this component's properties
UDC1BuildPlacementComponent::UDC1BuildPlacementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UDC1BuildPlacementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UDC1BuildPlacementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
}


// Called every frame
void UDC1BuildPlacementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UDC1BuildPlacementComponent::IsPlacing() const
{
	return false;
}

bool UDC1BuildPlacementComponent::BeginPlacement(const FGameplayTag& InBuildTag)
{
	return false;
}

void UDC1BuildPlacementComponent::EndPlacement()
{
}

