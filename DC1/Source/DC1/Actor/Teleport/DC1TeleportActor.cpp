// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Teleport/DC1TeleportActor.h"

#include "Subsystem/DC1TeleportSubsystem.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Define/DC1Enums.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Teleport/DC1TeleportComponent.h"

#include "Character/Player/DC1Player.h"

ADC1TeleportActor::ADC1TeleportActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	SetRootComponent(Mesh);

	InteractCollider = CreateDefaultSubobject<UBoxComponent>("InteractCollider");
	InteractCollider->SetupAttachment(Mesh);
	InteractCollider->SetCollisionProfileName(DC1CollisionProfile::Interaction);
	InteractCollider->SetGenerateOverlapEvents(true);

	ArrivalPoint = CreateDefaultSubobject<UArrowComponent>("ArrivalPoint");
	ArrivalPoint->SetupAttachment(Mesh);
}

void ADC1TeleportActor::BeginPlay()
{
	Super::BeginPlay();

	// Subsystem Register
	if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		TeleportSubsystem->RegisterPoint(this);
	}
}

void ADC1TeleportActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		TeleportSubsystem->UnregisterPoint(this);
	}

	Super::EndPlay(EndPlayReason);
}

UPrimitiveComponent* ADC1TeleportActor::GetInteractCollider() const
{
	return InteractCollider;
}

void ADC1TeleportActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	// 로컬 플레이어 CanUse에 따라 분기 처리 필요 (해금 전 LockedOptions / 후 UnlockedOptions)
}

void ADC1TeleportActor::Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	if (nullptr == InInteractOwner)
		return;

	ADC1PlayerState* PS = Cast<ADC1PlayerState>(InInteractOwner->GetPlayerState());
	if (false == IsValid(PS))
		return;

	UDC1TeleportComponent* PlayerTeleportComponent = PS->GetTeleportComponent();
	if (nullptr == PlayerTeleportComponent)
		return;

	// 해금
	if (false == CanUse(PS))
	{
		// Interact 함수는 클라이언트에서만 호출되므로 ServerRPC를 통해 서버 검증 진행
		PlayerTeleportComponent->Server_UnlockPoint(this);
	}
	// 순간이동
	else
	{

	}
}

FText ADC1TeleportActor::GetPointName() const
{
	return PointName;
}

FTransform ADC1TeleportActor::GetArrivalTransform() const
{
	if (nullptr == ArrivalPoint)
		return FTransform::Identity;

	return ArrivalPoint->GetComponentTransform();
}

bool ADC1TeleportActor::CanUse(const APlayerState* InPlayerState) const
{
	const ADC1PlayerState* PS = Cast<const ADC1PlayerState>(InPlayerState);
	if (nullptr == PS)
		return false;

	UDC1TeleportComponent* TeleportComponent = PS->GetTeleportComponent();
	if (nullptr == TeleportComponent)
		return false;

	return TeleportComponent->IsPointUnlocked(PointId);
}
