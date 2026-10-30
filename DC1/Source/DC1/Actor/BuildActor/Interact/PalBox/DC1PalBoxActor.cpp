// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

#include "Subsystem/DC1TeleportSubsystem.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Teleport/DC1TeleportViewModel.h"
#include "Tags/DC1GamePlayTags.h"

#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Components/ArrowComponent.h"
#include "Component/Teleport/DC1TeleportComponent.h"

#include "EngineUtils.h"

ADC1PalBoxActor::ADC1PalBoxActor()
	: Super()
{
	ArrivalPoint = CreateDefaultSubobject<UArrowComponent>("ArrivalPoint");
	ArrivalPoint->SetupAttachment(RootComponent);
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
	// 플레이어당 거점 1개
	if (const ADC1PlayerState* PlayerState = Cast<ADC1PlayerState>(InBuilder))
	{
		const UDC1PalPartyComponent* Party = PlayerState->GetPalPartyComponent();
		if (nullptr != Party && true == Party->HasBase())
		{
			return false;
		}
	}

	// 다른 거점과 범위가 겹치면 안 된다. 미리보기는 거점이 아니다 (리슨 서버에선 호스트 미리보기도 서버 월드에 있다)
	for (TActorIterator<ADC1PalBoxActor> It(GetWorld()); It; ++It)
	{
		const ADC1PalBoxActor* Other = *It;
		if (this == Other || true == Other->IsPreview())
		{
			continue;
		}

		if (FVector::DistSquared2D(GetActorLocation(), Other->GetActorLocation()) < FMath::Square(BaseRadius + Other->BaseRadius))
		{
			return false;
		}
	}

	return true;
}

void ADC1PalBoxActor::RegisterWorkTarget(IDC1WorkTargetInterface* InWorkTarget)
{
	if (nullptr == InWorkTarget)
	{
		return;
	}

	WorkTargets.AddUnique(TWeakInterfacePtr<IDC1WorkTargetInterface>(InWorkTarget));
}

void ADC1PalBoxActor::OnPlaced(const APlayerState* InBuilder)
{
	Super::OnPlaced(InBuilder);

	if (const ADC1PlayerState* PlayerState = Cast<ADC1PlayerState>(InBuilder))
	{
		if (UDC1PalPartyComponent* Party = PlayerState->GetPalPartyComponent())
		{
			Party->SetBase(this);
		}
	}

	CollectWorkTargets();
}

void ADC1PalBoxActor::CollectWorkTargets()
{
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;

		const ADC1BuildActor* Build = Cast<ADC1BuildActor>(Actor);
		if (nullptr != Build && true == Build->IsPreview())
		{
			continue;
		}

		IDC1WorkTargetInterface* WorkTarget = Cast<IDC1WorkTargetInterface>(Actor);
		if (nullptr == WorkTarget || false == IsInBase(Actor->GetActorLocation()))
		{
			continue;
		}

		RegisterWorkTarget(WorkTarget);
	}
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
