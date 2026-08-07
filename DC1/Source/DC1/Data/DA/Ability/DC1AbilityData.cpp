// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/Ability/DC1AbilityData.h"

#include "Component/DC1AbilitySystemComponent.h"

void UDC1AbilityData::GiveToAbilitySystem(UDC1AbilitySystemComponent* ASC, FDC1AbilitySetGrantedHandles& OutHandles, UObject* SourceObject) const
{
	if (nullptr == ASC) return;

	// Base Ability
	for (const TObjectPtr<UDC1AbilityData>& Base : BaseAbilityData)
	{
		if (IsValid(Base))
			Base->GiveToAbilitySystem(ASC, OutHandles, SourceObject);
	}

	// AttributeSet

	// Ability
	for (const FDC1AbilityEntry& Entry : Abilities)
	{
		if (nullptr == Entry.AbilityClass) continue;

		FGameplayAbilitySpec AbilitySpec(Entry.AbilityClass, Entry.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(Entry.ActivationTag);

		const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(AbilitySpec);
		OutHandles.AddAbilitySpecHandle(SpecHandle);

		if (true == Entry.IsPassive)
			ASC->TryActivateAbility(SpecHandle);
	}
}
