// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystem/DC1ExperienceSubsystem.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1ExperienceSubsystem::Grant(AActor* InEarner, const FGameplayTag& InSourceTag, int32 InAmount)
{
	if (nullptr == InEarner || false == InEarner->HasAuthority())
		return;

	if (0 >= InAmount || false == InSourceTag.IsValid())
		return;

	AActor* Recipient = FindRecipient(InEarner);
	if (false == IsValid(Recipient))
		return;

	FGameplayEventData Payload;
	Payload.EventTag = InSourceTag;
	Payload.EventMagnitude = static_cast<float>(InAmount);
	Payload.Instigator = InEarner;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Recipient, InSourceTag, Payload);
}

AActor* UDC1ExperienceSubsystem::FindRecipient(AActor* InEarner) const
{
	if (ADC1PlayerState* PS = Cast<ADC1PlayerState>(InEarner))
	{
		return InEarner;
	}

	if (ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(InEarner))
	{
		// 거점 배치된 팰은 Pal Actor 그대로 리턴
		if (nullptr != Pal->GetBasePalBox())
		{
			return Pal;
		}

		// 플레이어가 소유한 팰은 플레이어의 PlayerState 반환
		return Pal->GetOwnerPlayerState();
	}

	if (const APawn* Pawn = Cast<APawn>(InEarner))
	{
		return Pawn->GetPlayerState<ADC1PlayerState>();
	}

	return nullptr;
}
