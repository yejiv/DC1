// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

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

	UIManager->PushScreen(DC1UITags::UI_Main_PalBox);
}
