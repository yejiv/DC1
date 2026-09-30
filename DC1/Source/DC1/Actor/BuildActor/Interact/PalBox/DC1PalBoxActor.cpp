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

void ADC1PalBoxActor::Interact(ADC1Character* InInteractOwner)
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	// 처음 연 팰박스가 내 거점이 된다. 이미 있으면 서버가 무시한다
	if (::IsValid(InInteractOwner))
	{
		if (const ADC1PlayerState* PlayerState = InInteractOwner->GetPlayerState<ADC1PlayerState>())
		{
			if (UDC1PalPartyComponent* Party = PlayerState->GetPalPartyComponent(); nullptr != Party && false == Party->HasBase())
			{
				Party->Server_SetBase(this);
			}
		}
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
