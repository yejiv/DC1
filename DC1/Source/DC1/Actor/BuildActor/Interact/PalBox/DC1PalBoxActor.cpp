// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

#include "Subsystem/DC1TeleportSubsystem.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Teleport/DC1TeleportViewModel.h"
#include "Tags/DC1GamePlayTags.h"

#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalBaseComponent.h"
#include "Component/Base/DC1BaseRegistryComponent.h"
#include "GameMode/DC1GameState.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Components/ArrowComponent.h"
#include "Component/Teleport/DC1TeleportComponent.h"

ADC1PalBoxActor::ADC1PalBoxActor()
	: Super()
{
	ArrivalPoint = CreateDefaultSubobject<UArrowComponent>("ArrivalPoint");
	ArrivalPoint->SetupAttachment(RootComponent);

	PalBaseComponent = CreateDefaultSubobject<UDC1PalBaseComponent>("PalBaseComponent");
	RegistryComponent = CreateDefaultSubobject<UDC1BaseRegistryComponent>("RegistryComponent");
}

bool ADC1PalBoxActor::IsInBase(const FVector& InLocation) const
{
	return FVector::DistSquared2D(InLocation, GetActorLocation()) <= FMath::Square(BaseRadius);
}

FText ADC1PalBoxActor::GetPointName() const
{
	return FText::FromString("Base");
}

FTransform ADC1PalBoxActor::GetArrivalTransform() const
{
	if (nullptr == ArrivalPoint)
		return FTransform::Identity;

	return ArrivalPoint->GetComponentTransform();
}

bool ADC1PalBoxActor::CanUse(const APlayerState* InPlayerState) const
{
	// 전원 같은 팀 + 거점 1개 → 완성된 거점이면 누구나 사용 ( 파괴 연출 중 제외 )
	return EDC1BuildState::Completed == BuildState;
}

void ADC1PalBoxActor::BeginPlay()
{
	Super::BeginPlay();

	if (EDC1BuildState::Completed == BuildState)
	{
		// Subsystem Register
		if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
		{
			TeleportSubsystem->RegisterPoint(this);
		}
	}
}

void ADC1PalBoxActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		TeleportSubsystem->UnregisterPoint(this);
	}

	if (EEndPlayReason::Destroyed == EndPlayReason && true == HasAuthority())
	{
		if (true == ::IsValid(RegistryComponent))
		{
			RegistryComponent->DestroyBaseRequiredBuilds(GetDestroyInstigator());
		}

		if (true == ::IsValid(PalBaseComponent))
		{
			PalBaseComponent->ReleaseBase(GetDestroyInstigator());
		}
	}

	Super::EndPlay(EndPlayReason);
}

void ADC1PalBoxActor::OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	switch (InKey)
	{
	case EDC1InteractKey::F :
		InteractF(InInteractOwner);
		break;
	case EDC1InteractKey::V:
		InteractV(InInteractOwner);
		break;
	default:
		return;
	}
}

void ADC1PalBoxActor::InteractF(ADC1Character* InInteractOwner)
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	UIManager->PushScreen(DC1UITags::UI_Main_PalBox);
}

void ADC1PalBoxActor::InteractV(ADC1Character* InInteractOwner)
{
	if (nullptr == InInteractOwner)
		return;

	ADC1PlayerState* PS = Cast<ADC1PlayerState>(InInteractOwner->GetPlayerState());
	if (false == IsValid(PS))
		return;

	UDC1TeleportComponent* PlayerTeleportComponent = PS->GetTeleportComponent();
	if (nullptr == PlayerTeleportComponent)
		return;

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

bool ADC1PalBoxActor::CanPlaceAt(const APlayerState* InBuilder) const
{
	// 서버당 거점 1개
	const ADC1GameState* GS = ADC1GameState::Get(this);
	// 거점 개수가 충분한지 = HasBase ( 현재 1개가 Max라 따로 범위 검사 X )
	if (true == ::IsValid(GS) && true == GS->HasBase())
	{
		return false;
	}

	return true;
}

const TArray<TWeakInterfacePtr<IDC1WorkTargetInterface>>& ADC1PalBoxActor::GetWorkTargets() const
{
	return RegistryComponent->GetWorkTargets();
}

void ADC1PalBoxActor::RegisterWorkTarget(IDC1WorkTargetInterface* InWorkTarget)
{
	RegistryComponent->RegisterWorkTarget(InWorkTarget);
}

void ADC1PalBoxActor::OnPlaced(const APlayerState* InBuilder)
{
	Super::OnPlaced(InBuilder);

	ADC1GameState* GS = ADC1GameState::Get(this);
	if (false == ::IsValid(GS))
	{
		return;
	}

	GS->SetBase(this);

	if (this != GS->GetBasePalBox())
	{
		return;
	}

	// 거점이 GS에 Set 되었을경우에만
	PalBaseComponent->InitBase();
	RegistryComponent->CollectWorkTargets();

	GS->NotifyBaseChanged();
}

void ADC1PalBoxActor::BuildCompleted()
{
	Super::BuildCompleted();

	// Subsystem Register
	if (UDC1TeleportSubsystem* TeleportSubsystem = GetWorld()->GetSubsystem<UDC1TeleportSubsystem>())
	{
		TeleportSubsystem->RegisterPoint(this);
	}
}
