// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1InteractComponent.h"
#include "Enum/DC1Enums.h"
#include "Actor/BuildActor/DC1BuildActor.h"

// Sets default values for this component's properties
UDC1InteractComponent::UDC1InteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;
}


void UDC1InteractComponent::SetInteractEnabled(bool InEnabled)
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
	{
		return;
	}

	SetComponentTickEnabled(InEnabled);

	SetCurrentTarget(nullptr);
}

// Called when the game starts
void UDC1InteractComponent::BeginPlay()
{
	Super::BeginPlay();

	const APlayerController* PC = GetOwner<APlayerController>();
	if (true == ::IsValid(PC) || true == PC->IsLocalController())
	{
		SetInteractEnabled(true);
	}
}


// Called every frame
void UDC1InteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetCurrentTarget(TraceTarget());
}

const ADC1BuildActor* UDC1InteractComponent::TraceTarget() const
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	const APawn* Pawn = PC->GetPawn();
	if (false == ::IsValid(Pawn))
	{
		return nullptr;
	}

	FVector Start;
	FVector End;
	FRotator ViewRotation;

	PC->GetPlayerViewPoint(Start, ViewRotation);

	End = Start + (ViewRotation.Vector() * MaxTraceDistance);

	FHitResult Hit;
	FCollisionQueryParams HitParams;
	HitParams.AddIgnoredActor(Pawn);

	const bool IsHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_BuildInteraction), HitParams);

	if (true == IsHit)
	{
		const ADC1BuildActor* HitActor = Cast<ADC1BuildActor>(Hit.GetActor());
		if (true == ::IsValid(HitActor) && HitActor->IsInInteractRange(Pawn))
		{
			return HitActor;
		}
	}

	return nullptr;
}

void UDC1InteractComponent::SetCurrentTarget(const ADC1BuildActor* InNewTarget)
{
	if (InNewTarget == CurrentTarget.Get())
	{
		return;
	}

	CurrentTarget = InNewTarget;

	//Test - TODO Change UI
	UE_LOG(LogTemp, Warning, TEXT("Find Actor"));
}

