// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

#include "Character/DC1Character.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

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
