// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalRideSprint.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Component/Pal/DC1PalMovementComponent.h"
#include "Component/Pal/DC1PalRideComponent.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "AbilitySystemComponent.h"

UDC1GameplayAbility_PalRideSprint::UDC1GameplayAbility_PalRideSprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 어빌리티 수명 == 질주하는 동안
	ActivationOwnedTags.AddTag(DC1StateTags::State_Sprinting);

	ActivationBlockedTags.AddTag(DC1StateTags::State_Sprinting);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dead);
}

bool UDC1GameplayAbility_PalRideSprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ADC1PalCharacter* Pal = GetPal();
	if (nullptr == Pal)
	{
		return false;
	}

	// 다 올라탄 뒤에만
	const UDC1PalRideComponent* RideComponent = Pal->GetRideComponent();
	if (nullptr == RideComponent || EDC1RideState::Riding != RideComponent->GetRideState())
	{
		return false;
	}

	if (const UAbilitySystemComponent* ASC = Pal->GetAbilitySystemComponent())
	{
		if (ASC->GetNumericAttribute(UDC1CharacterAttributeSet::GetStaminaAttribute()) <= 0.f)
		{
			return false;
		}
	}

	return true;
}

void UDC1GameplayAbility_PalRideSprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	SetSprint(true);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (nullptr == ASC)
	{
		return;
	}

	StaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &ThisClass::HandleStaminaChanged);

	if (nullptr != CostEffectClass && true == ASC->IsOwnerActorAuthoritative())
	{
		const FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
		if (true == Spec.IsValid())
		{
			CostEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
	}
}

void UDC1GameplayAbility_PalRideSprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	SetSprint(false);

	// Super 가 ActorInfo 를 정리하기 전에
	if (nullptr != ActorInfo)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			if (true == StaminaChangedHandle.IsValid())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
				StaminaChangedHandle.Reset();
			}

			if (true == CostEffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(CostEffectHandle);
				CostEffectHandle = FActiveGameplayEffectHandle();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GameplayAbility_PalRideSprint::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.f)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDC1GameplayAbility_PalRideSprint::SetSprint(bool InSprint) const
{
	const ADC1PalCharacter* Pal = GetPal();
	if (nullptr == Pal)
	{
		return;
	}

	if (UDC1PalMovementComponent* PalMovement = Cast<UDC1PalMovementComponent>(Pal->GetCharacterMovement()))
	{
		PalMovement->SetRideSprint(InSprint);
	}
}

ADC1PalCharacter* UDC1GameplayAbility_PalRideSprint::GetPal() const
{
	return Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
}
