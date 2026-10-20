// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Teleport/DC1TeleportComponent.h"

#include "Actor/Teleport/DC1TeleportActor.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1CharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "Define/DC1Macro.h"
#include "Subsystem/DC1TeleportSubsystem.h"
#include "Subsystem/DC1UIManagerSubsystem.h"

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
	const bool bSuccess = TryTeleport(InSource, InDest);

	// 성공이든 실패든 항상 보낸다 ( 클라는 이미 FadeIn 상태 )
	Client_OnTeleportFinished(bSuccess);
}

bool UDC1TeleportComponent::TryTeleport(AActor* InSource, AActor* InDest)
{
	if (nullptr == InSource || nullptr == InDest)
		return false;

	// 1. Source, Dest 다른 객체인지 판단
	if (InSource == InDest)
	{
		LOG(TEXT("[Teleport] Source == Dest"));
		return false;
	}

	// 2. Player가 Source 인식 범위 내인지 판단
	if (false == IsInnerInteractRange(InSource))
	{
		LOG(TEXT("[Teleport] Out of Range (Source)"));
		return false;
	}

	const ADC1TeleportActor* DestActor = Cast<const ADC1TeleportActor>(InDest);
	if (nullptr == DestActor)
		return false;

	const APlayerState* PS = Cast<const APlayerState>(GetOwner());
	if (nullptr == PS)
		return false;

	// 3. Dest Actor가 사용 가능한 액터인지 판단 (해금 여부)
	if (false == DestActor->CanUse(PS))
	{
		LOG(TEXT("[Teleport] Cannot Use Dest"));
		return false;
	}

	FTransform DestTransform = DestActor->GetArrivalTransform();

	const FVector DestLocation = DestTransform.GetLocation();
	const FRotator DestRotation = DestTransform.GetRotation().Rotator();

	APawn* Pawn = PS->GetPawn();
	if (nullptr == Pawn)
		return false;

	bool bResult = Pawn->TeleportTo(DestLocation, DestRotation);
	if (false == bResult)
	{
		LOG(TEXT("[Teleport] Failed Teleport"));
		return false;
	}

	// 회전 동기화는 CMC가 전담 ( 서버 / 본인 클라 / 카메라 )
	if (const ACharacter* Character = Cast<ACharacter>(Pawn))
	{
		if (UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(Character->GetCharacterMovement()))
		{
			CMC->ForceRotation(DestRotation, true);
		}
	}

	return true;
}

void UDC1TeleportComponent::Client_OnTeleportFinished_Implementation(bool bInSuccess)
{
	if (UDC1UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
	{
		UIManager->FadeOut(1.f);
	}
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
