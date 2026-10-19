// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Teleport/DC1TeleportComponent.h"

#include "Actor/Teleport/DC1TeleportActor.h"

#include "Character/Player/DC1PlayerState.h"

#include "Net/UnrealNetwork.h"

#include "Define/DC1Macro.h"
#include "Subsystem/DC1TeleportSubsystem.h"

UDC1TeleportComponent::UDC1TeleportComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UDC1TeleportComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UDC1TeleportComponent, UnlockedPointIds, COND_OwnerOnly);
}

bool UDC1TeleportComponent::IsPointUnlocked(FName InPointId) const
{
	return UnlockedPointIds.Contains(InPointId);
}

void UDC1TeleportComponent::Server_UnlockPoint_Implementation(AActor* InPoint)
{
	// 해금 대상은 텔레포트 오브젝트만
	const ADC1TeleportActor* TeleportActor = Cast<ADC1TeleportActor>(InPoint);
	if (nullptr == TeleportActor)
		return;

	// (1) 거리 검증
	if (false == IsInnerInteractRange(InPoint))
	{
		LOG(TEXT("[Teleport] Server - Out of Range (Interact)"));
		return;
	}

	// ID는 클라가 보낸 값이 아니라 액터에서 직접 꺼낸다
	const FName PointId = TeleportActor->GetPointId();
	if (true == PointId.IsNone())
		return;

	// 해금된 오브젝트의 ID는 보관한다
	UnlockedPointIds.AddUnique(PointId);

	// PlayerState는 갱신 주기가 낮아서 바로 보낸다
	GetOwner()->ForceNetUpdate();

	OnRep_UnlockedPointIds();
}

void UDC1TeleportComponent::Server_RequestTeleport_Implementation(AActor* InSource, AActor* InDest)
{

}

void UDC1TeleportComponent::OnRep_UnlockedPointIds()
{
	if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		TeleportSubsystem->NotifyPointsChanged();
	}
}

bool UDC1TeleportComponent::IsInnerInteractRange(AActor* InPoint) const
{
	if (nullptr == InPoint)
		return false;

	const APlayerState* PS = GetOwner<APlayerState>();
	const AActor* Pawn = (nullptr != PS) ? PS->GetPawn() : nullptr;
	if (nullptr == Pawn)
		return false;

	const FVector ObjectLocation = InPoint->GetActorLocation();
	const FVector PawnLocation = Pawn->GetActorLocation();

	const float DistSQ = FVector::DistSquared(PawnLocation, ObjectLocation);

	return DistSQ < FMath::Square(InteractAllowDistance);
}
