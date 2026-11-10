// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Cues/DC1GameplayCueNotify_DamageNumber.h"

#include "Character/Player/DC1PlayerController.h"
#include "Component/Widget/DC1DamageNumberComponent.h"
#include "Tags/DC1GamePlayTags.h"

bool UDC1GameplayCueNotify_DamageNumber::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (false == ::IsValid(MyTarget))
	{
		return false;
	}

	const UGameInstance* GameInstance = MyTarget->GetGameInstance();
	if (nullptr == GameInstance)
	{
		return false;
	}

	const ADC1PlayerController* PC = Cast<ADC1PlayerController>(GameInstance->GetFirstLocalPlayerController());
	if (nullptr == PC)
	{
		return false;
	}

	UDC1DamageNumberComponent* DamageNumberComponent = PC->GetDamageNumberComponent();
	if (false == ::IsValid(DamageNumberComponent))
	{
		return false;
	}

	const EDC1DamageNumberType Type = (true == Parameters.OriginalTag.MatchesTagExact(DC1EffectTags::GameplayCue_DamageNumber_Shield)) ? EDC1DamageNumberType::Shield : EDC1DamageNumberType::Health;
	DamageNumberComponent->ShowDamageNumber(Parameters.Location, Parameters.RawMagnitude, Type);

	return true;
}
