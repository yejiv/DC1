// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Teleport/DC1TeleportActor.h"

#include "Subsystem/DC1TeleportSubsystem.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Teleport/DC1TeleportViewModel.h"
#include "Tags/DC1GamePlayTags.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Define/DC1Enums.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Teleport/DC1TeleportComponent.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerController.h"

#include "Kismet/GameplayStatics.h"

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
		TeleportSubsystem->OnPointsChanged.AddUObject(this, &ThisClass::HandlePointsChanged);
	}
}

void ADC1TeleportActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		TeleportSubsystem->UnregisterPoint(this);
		TeleportSubsystem->OnPointsChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

UPrimitiveComponent* ADC1TeleportActor::GetInteractCollider() const
{
	return InteractCollider;
}

void ADC1TeleportActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	const ADC1PlayerController* PC = GetWorld()->GetFirstPlayerController<ADC1PlayerController>();
	if (nullptr == PC)
		return;

	const APlayerState* PS = PC->GetPlayerState<APlayerState>();
	if (nullptr == PS)
		return;

	OutOptions = CanUse(PS) ? UnlockedOptions : LockedOptions;
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

		UGameplayStatics::PlaySound2D(this, UnlockSound);
	}
	// 순간이동
	else
	{
		// 화면이 열리면서 목록을 만들기 때문에 PushScreen 전에 출발 지점을 넘긴다
		const APlayerController* PC = InInteractOwner->GetController<APlayerController>();
		const ULocalPlayer* LocalPlayer = (nullptr != PC) ? PC->GetLocalPlayer() : nullptr;
		if (nullptr == LocalPlayer)
			return;

		if (UDC1ViewModelSubsystem* VMSubsystem = LocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
		{
			if (UDC1TeleportViewModel* TeleportViewModel = VMSubsystem->GetViewModel<UDC1TeleportViewModel>())
			{
				TeleportViewModel->SetListMode(EDC1TeleportListMode::Teleport, this);
			}
		}

		if (UDC1UIManagerSubsystem* Subsystem = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
		{
			Subsystem->PushScreen(DC1UITags::UI_Main_Teleport);
		}
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

void ADC1TeleportActor::HandlePointsChanged()
{
	OnInteractOptionsChanged.Broadcast();
}
