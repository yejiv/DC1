// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Interact/DC1InteractComponent.h"
#include "Define/DC1Enums.h"
#include "Actor/BuildActor/DC1BuildActor.h"
#include "Character/Player/DC1Player.h"

#include "Components/SphereComponent.h"
#include "Interface/DC1InteractableInterface.h"

// Sets default values for this component's properties
UDC1InteractComponent::UDC1InteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetIsReplicatedByDefault(true);
}

IDC1InteractableInterface* UDC1InteractComponent::GetCurrentInteractable() const
{
	if (nullptr == CurrentTarget.Get())
	{
		return nullptr;
	}

	return Cast<IDC1InteractableInterface>(CurrentTarget.Get());
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

void UDC1InteractComponent::SetTargetLocked(bool InLocked)
{
	IsTargetLocked = InLocked;
}

void UDC1InteractComponent::BeginPlay()
{
	Super::BeginPlay();

	
	SetInteractEnabled(true);
}


void UDC1InteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (true == IsTargetLocked)
	{
		return;
	}

	SetCurrentTarget(FindBestTarget());
}

void UDC1InteractComponent::RequestStartWork(ADC1BuildActor* InBuildTarget)
{
	Server_StartWork(InBuildTarget);
}

void UDC1InteractComponent::RequestStopWork()
{
	Server_StopWork();
}

void UDC1InteractComponent::Server_StartWork_Implementation(ADC1BuildActor* InBuildTarget)
{
	Server_StopWork_Implementation();

	if (false == ::IsValid(InBuildTarget))
	{
		return;
	}
	
	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return;
	}

	ADC1Player* Player = PC->GetPawn<ADC1Player>();
	if (false == ::IsValid(Player))
	{
		return;
	}

	const USphereComponent* InteractDetectCollider = Player->GetInteractDetectCollider();
	if (false == ::IsValid(InteractDetectCollider))
	{
		return;
	}

	const IDC1InteractableInterface* Interactble = Cast<IDC1InteractableInterface>(InBuildTarget);
	if (nullptr == Interactble)
	{
		return;
	}

	const UPrimitiveComponent* InteractCollider = Interactble->GetInteractCollider();

	if (false == ::IsValid(InteractDetectCollider) || false == ::IsValid(InteractCollider))
	{
		return;
	}

	bool IsOverlapping = InteractDetectCollider->IsOverlappingComponent(InteractCollider);

	if (true == IsOverlapping)
	{
		InBuildTarget->AddWorker(Player);
		ServerWorkTarget = InBuildTarget;
	}
}

void UDC1InteractComponent::Server_StopWork_Implementation()
{
	if (nullptr != ServerWorkTarget)
	{
		ADC1BuildActor* BuildTarget = ServerWorkTarget.Get();
		if (true == ::IsValid(BuildTarget))
		{
			APlayerController* PC = GetOwner<APlayerController>();
			if (false == ::IsValid(PC))
			{
				return;
			}

			const ADC1Player* Player = PC->GetPawn<ADC1Player>();
			if (false == ::IsValid(Player))
			{
				return;
			}

			BuildTarget->RemoveWorker(Player);
		}
	}

	ServerWorkTarget = nullptr;
}

AActor* UDC1InteractComponent::FindBestTarget() const
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	const ADC1Player* Player = PC->GetPawn<ADC1Player>();
	if (false == ::IsValid(Player))
	{
		return nullptr;
	}

	const USphereComponent* InteractDetectCollider = Player->GetInteractDetectCollider();
	if (false == ::IsValid(InteractDetectCollider))
	{
		return nullptr;
	}
	
	OverlapBuffer.Reset();

	InteractDetectCollider->GetOverlappingActors(OverlapBuffer);
	if (true == OverlapBuffer.IsEmpty())
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	float BestDot = (FLT_MAX * -1.f);
	AActor* BestTarget = nullptr;

	for (AActor* OverlapActor : OverlapBuffer)
	{
		const IDC1InteractableInterface* Interactable = Cast<IDC1InteractableInterface>(OverlapActor);
		if (nullptr == Interactable)
		{
			continue;
		}

		const UPrimitiveComponent* InteractCollider = Interactable->GetInteractCollider();
		if (false == ::IsValid(InteractCollider))
		{
			continue;
		}

		FVector OverlapLocation = InteractCollider->GetComponentLocation();

		float Dot = FVector::DotProduct(ViewRotation.Vector(), (OverlapLocation - ViewLocation).GetSafeNormal());

		if (OverlapActor == CurrentTarget.Get())
		{
			Dot += CurrentTargetDotBonus;
		}

		if (Dot > BestDot)
		{
			BestTarget = OverlapActor;
			BestDot = Dot;
		}
	}

	return BestTarget;
}

void UDC1InteractComponent::SetCurrentTarget(AActor* InNewTarget)
{
	if (InNewTarget == CurrentTarget.Get())
	{
		return;
	}

	if (nullptr != CurrentTarget.Get())
	{
		IDC1InteractableInterface* Interactable = Cast<IDC1InteractableInterface>(CurrentTarget.Get());
		if(nullptr != Interactable)
		{
			Interactable->OnInteractableUnfocused();

			UE_LOG(LogTemp, Warning, TEXT("Unfind Actor"));
		}
	}
	
	if (nullptr != InNewTarget)
	{
		IDC1InteractableInterface* Interactable = Cast<IDC1InteractableInterface>(InNewTarget);
		if (nullptr != Interactable)
		{
			Interactable->OnInteractableFocused();

			UE_LOG(LogTemp, Warning, TEXT("Find Actor"));
		}
	}

	CurrentTarget = InNewTarget;
}