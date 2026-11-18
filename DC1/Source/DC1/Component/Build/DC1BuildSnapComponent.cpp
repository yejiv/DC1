// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Build/DC1BuildSnapComponent.h"

#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"

#include "Component/Build/DC1BuildSocketComponent.h"
#include "Define/DC1Structs.h"

UDC1BuildSnapComponent::UDC1BuildSnapComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UDC1BuildSnapComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickEnabled(DrawDebugSocket);
}

void UDC1BuildSnapComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DrawDebugSockets();
}

void UDC1BuildSnapComponent::LayoutSockets()
{
	SetRelativeTransform(FTransform::Identity);

	const UBoxComponent* ParentBox = Cast<UBoxComponent>(GetAttachParent());
	if (false == ::IsValid(ParentBox))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildSnapComponent :: Parent Component is not Box Component || None Parent"));
		return;
	}

	const FVector BoxExtent = ParentBox->GetUnscaledBoxExtent();

	for (USceneComponent* Child : GetAttachChildren())
	{
		UDC1BuildSocketComponent* SocketChild = Cast<UDC1BuildSocketComponent>(Child);
		if (false == ::IsValid(SocketChild))
		{
			continue;
		}

		SocketChild->SetRelativeLocation(BoxExtent * SocketChild->GetPositionFromDirection());
	}
}

bool UDC1BuildSnapComponent::GetBuildSocketsToWorld(TArray<FDC1BuildSnapSocket>& OutSockets) const
{
	OutSockets.Reset();

	for (const USceneComponent* Child : GetAttachChildren())
	{
		const UDC1BuildSocketComponent* SocketChild = Cast<UDC1BuildSocketComponent>(Child);
		if (false == ::IsValid(SocketChild))
		{
			continue;
		}

		FDC1BuildSnapSocket SnapSocket;

		SnapSocket.SocketTransform = SocketChild->GetComponentTransform();
		SnapSocket.SnapType = SocketChild->GetSnapType().IsValid() ? SocketChild->GetSnapType() : FGameplayTag::EmptyTag;
		SnapSocket.AcceptTypes = SocketChild->GetAcceptTypes();
		SnapSocket.SourceSocket = SocketChild;

		OutSockets.Add(SnapSocket);
	}

	return false == OutSockets.IsEmpty();
}

void UDC1BuildSnapComponent::DrawDebugSockets()
{
	const UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return;
	}

	const UBoxComponent* ParentBox = Cast<UBoxComponent>(GetAttachParent());
	if (false == ::IsValid(ParentBox))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildSnapComponent :: Parent Component is not Box Component || None Parent"));
		return;
	}

	const FVector BoxExtent = ParentBox->GetScaledBoxExtent();

	DrawDebugBox(World, GetComponentLocation(), BoxExtent, GetComponentQuat(), FColor::Silver);

	TArray<FDC1BuildSnapSocket> Sockets;

	if (false == GetBuildSocketsToWorld(Sockets))
	{
		return;
	}

	for (const FDC1BuildSnapSocket& Socket : Sockets)
	{
		const FVector Location = Socket.SocketTransform.GetLocation();
		const FRotator Rotator = Socket.SocketTransform.GetRotation().Rotator();

		DrawDebugCoordinateSystem(World, Location, Rotator, 40.f, false, -1.f, 0, 1.f);
		DrawDebugSphere(World, Location, 20.f, 8, FColor::White);
	}
}

