// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Teleport/DC1TeleportComponent.h"

#include "Actor/Teleport/DC1TeleportActor.h"

#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1Player.h"
#include "Component/DC1CharacterMovementComponent.h"
#include "GameMode/DC1GameMode.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "AbilitySystemComponent.h"

#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"

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

void UDC1TeleportComponent::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerState* PS = GetOwner<APlayerState>())
	{
		PS->OnPawnSet.AddDynamic(this, &ThisClass::HandlePawnSet);
	}
}

void UDC1TeleportComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (APlayerState* PS = GetOwner<APlayerState>())
	{
		PS->OnPawnSet.RemoveDynamic(this, &ThisClass::HandlePawnSet);
	}

	Super::EndPlay(EndPlayReason);
}

void UDC1TeleportComponent::HandlePawnSet(APlayerState* InPlayerState, APawn* InNewPawn, APawn* InOldPawn)
{
	// 부활 대기 중 + 새 플레이어 Pawn ( UnPossess의 null / 탑승 팰 제외 )
	if (false == bPendingRespawnFade || nullptr == Cast<ADC1Player>(InNewPawn))
		return;

	bPendingRespawnFade = false;

	if (UDC1UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
	{
		UIManager->SetHUDVisibility(true);
		UIManager->PopAllScreen();
		UIManager->FadeOut(1.f);
	}
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
	UnlockPointById(TeleportActor->GetPointId());
}

void UDC1TeleportComponent::UnlockPointById(FName InPointId)
{
	if (false == GetOwner()->HasAuthority())
		return;

	if (true == InPointId.IsNone())
		return;

	// 해금된 오브젝트의 ID는 보관한다
	UnlockedPointIds.AddUnique(InPointId);

	// PlayerState는 갱신 주기가 낮아서 바로 보낸다
	GetOwner()->ForceNetUpdate();

	OnRep_UnlockedPointIds();
}

void UDC1TeleportComponent::RequestRespawn(AActor* InDest)
{
	// 리슨 서버 호스트는 RPC가 바로 실행되어 새 Pawn이 즉시 붙으므로 먼저 표시
	bPendingRespawnFade = true;

	Server_RequestRespawn(InDest);
}

void UDC1TeleportComponent::Server_RequestRespawn_Implementation(AActor* InDest)
{
	ADC1PlayerState* PS = GetOwner<ADC1PlayerState>();
	AController* Controller = (nullptr != PS) ? PS->GetOwningController() : nullptr;
	UAbilitySystemComponent* ASC = (nullptr != PS) ? PS->GetAbilitySystemComponent() : nullptr;

	// 사망 상태에서만 부활
	if (nullptr == Controller || nullptr == ASC || false == ASC->HasMatchingGameplayTag(DC1StateTags::State_Dead))
	{
		LOG(TEXT("[Teleport] Respawn Denied"));
		Client_OnTeleportFinished(false);
		return;
	}

	ADC1GameMode* GameMode = GetWorld()->GetAuthGameMode<ADC1GameMode>();
	if (nullptr == GameMode)
	{
		Client_OnTeleportFinished(false);
		return;
	}

	// 사용 가능한 지점이면 그 위치, 아니면 시작 지점에서 부활
	FTransform SpawnTransform;
	const bool bHasDest = ResolveDest(InDest, SpawnTransform);

	const bool bSuccess = GameMode->RespawnPlayer(Controller, true == bHasDest ? &SpawnTransform : nullptr);

	// 성공 시 FadeOut은 새 Pawn이 도착한 클라에서 ( HandlePawnSet )
	if (false == bSuccess)
	{
		LOG_WARNING(TEXT("[Teleport] Respawn Failed"));
		Client_OnTeleportFinished(false);
	}
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

	// 3. Dest Actor가 사용 가능한 액터인지 판단 (해금 여부)
	FTransform DestTransform;
	if (false == ResolveDest(InDest, DestTransform))
		return false;

	const FVector DestLocation = DestTransform.GetLocation();
	const FRotator DestRotation = DestTransform.GetRotation().Rotator();

	const APlayerState* PS = Cast<const APlayerState>(GetOwner());
	if (nullptr == PS)
		return false;

	ADC1Player* Player = Cast<ADC1Player>(PS->GetPawn());
	if (nullptr == Player)
		return false;

	bool bResult = Player->TeleportTo(DestLocation, DestRotation);
	if (false == bResult)
	{
		LOG(TEXT("[Teleport] Failed Teleport"));
		return false;
	}

	// 회전 동기화는 CMC가 전담 ( 서버 / 본인 클라 / 카메라 )
	if (UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(Player->GetCharacterMovement()))
	{
		CMC->ForceRotation(DestRotation, true);
	}

	Player->PlayWorldSound(TeleportSound, DestLocation);
	Multicast_PlayTeleportFX();

	return true;
}

void UDC1TeleportComponent::Multicast_PlayTeleportFX_Implementation()
{
	if (nullptr == TeleportFX || true == IsRunningDedicatedServer())
		return;

	const APlayerState* PS = Cast<const APlayerState>(GetOwner());
	const ACharacter* Character = (nullptr != PS) ? PS->GetPawn<ACharacter>() : nullptr;
	if (nullptr == Character)
		return;

	UNiagaraFunctionLibrary::SpawnSystemAttached(TeleportFX, Character->GetMesh(), NAME_None,
		FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
}

bool UDC1TeleportComponent::ResolveDest(AActor* InDest, FTransform& OutTransform) const
{
	const IDC1TeleportPointInterface* DestInterface = Cast<const IDC1TeleportPointInterface>(InDest);
	if (nullptr == DestInterface)
		return false;

	const APlayerState* PS = Cast<const APlayerState>(GetOwner());
	if (nullptr == PS)
		return false;

	if (false == DestInterface->CanUse(PS))
	{
		LOG(TEXT("[Teleport] Cannot Use Dest"));
		return false;
	}

	OutTransform = DestInterface->GetArrivalTransform();
	return true;
}

void UDC1TeleportComponent::Client_OnTeleportFinished_Implementation(bool bInSuccess)
{
	// 부활 실패면 새 Pawn 대기 해제
	bPendingRespawnFade = false;

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

void UDC1TeleportComponent::OnPostLoad()
{
	// 리슨 서버 호스트는 OnRep이 안 옴
	OnRep_UnlockedPointIds();
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
