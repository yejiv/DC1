// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "EngineUtils.h"

bool ADC1PalBoxActor::IsInBase(const FVector& InLocation) const
{
	return FVector::DistSquared2D(InLocation, GetActorLocation()) <= FMath::Square(BaseRadius);
}

void ADC1PalBoxActor::OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	switch (InKey)
	{
	case EDC1InteractKey::F :
		InteractF(InInteractOwner);
		break;
	case EDC1InteractKey::V:
		UE_LOG(LogTemp, Warning, TEXT("ADC1PalBoxActor :: 빠른 이동"));
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
