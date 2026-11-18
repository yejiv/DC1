// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/Ability/DC1AbilityData.h"

#include "Component/DC1AbilitySystemComponent.h"
#include "GameplayEffect.h"

void UDC1AbilityData::GiveToAbilitySystem(UDC1AbilitySystemComponent* ASC, FDC1AbilitySetGrantedHandles& OutHandles, UObject* SourceObject) const
{
	if (nullptr == ASC) return;

	// Base Ability
	for (const TObjectPtr<UDC1AbilityData>& Base : BaseAbilityData)
	{
		if (IsValid(Base))
			Base->GiveToAbilitySystem(ASC, OutHandles, SourceObject);
	}

	// Ability
	for (const FDC1AbilityEntry& Entry : Abilities)
	{
		if (nullptr == Entry.AbilityClass) continue;

		// BaseAbilityData 합성이나 재부여로 같은 항목이 두 번 들어오는 것을 막는다
		if (true == ASC->HasAbility(Entry.AbilityClass, Entry.ActivationTag)) continue;

		FGameplayAbilitySpec AbilitySpec(Entry.AbilityClass, Entry.AbilityLevel);
		AbilitySpec.SourceObject = SourceObject;
		AbilitySpec.GetDynamicSpecSourceTags().AddTag(Entry.ActivationTag);

		const FGameplayAbilitySpecHandle SpecHandle = ASC->GiveAbility(AbilitySpec);
		OutHandles.AddAbilitySpecHandle(SpecHandle);

		if (true == Entry.IsPassive)
			ASC->TryActivateAbility(SpecHandle);
	}

	// Startup Effect
	for (const TSubclassOf<UGameplayEffect>& EffectClass : StartupEffects)
	{
		if (nullptr == EffectClass) continue;

		// BaseAbilityData 합성으로 같은 GE가 두 번 들어오면 회복량이 두 배가 된다
		FGameplayEffectQuery Query;
		Query.EffectDefinition = EffectClass;
		if (ASC->GetActiveEffects(Query).Num() > 0) continue;

		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(EffectClass, 1.f, ASC->MakeEffectContext());
		if (false == Spec.IsValid()) continue;

		OutHandles.AddEffectHandle(ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get()));
	}
}

FGameplayAbilitySpecHandle UDC1AbilityData::GiveVariant(UDC1AbilitySystemComponent* ASC, const FGameplayTag& InActivationTag, const FGameplayTag& InVariantTag, UObject* SourceObject) const
{
	const FDC1AbilityEntry* Entry = FindEntryByTag(InActivationTag);
	if (nullptr == ASC || nullptr == Entry || nullptr == Entry->AbilityClass || false == InVariantTag.IsValid()) return FGameplayAbilitySpecHandle();

	if (true == ASC->HasAbility(Entry->AbilityClass, InVariantTag)) return FGameplayAbilitySpecHandle();

	FGameplayAbilitySpec AbilitySpec(Entry->AbilityClass, Entry->AbilityLevel);
	AbilitySpec.SourceObject = SourceObject;
	AbilitySpec.GetDynamicSpecSourceTags().AddTag(InVariantTag);

	return ASC->GiveAbility(AbilitySpec);
}

const FDC1AbilityEntry* UDC1AbilityData::FindEntryByTag(const FGameplayTag& InActivationTag) const
{
	for (const FDC1AbilityEntry& Entry : Abilities)
	{
		if (Entry.ActivationTag == InActivationTag)
			return &Entry;
	}

	for (const TObjectPtr<UDC1AbilityData>& Base : BaseAbilityData)
	{
		if (IsValid(Base))
		{
			if (const FDC1AbilityEntry* Found = Base->FindEntryByTag(InActivationTag))
				return Found;
		}
	}

	return nullptr;
}

TSubclassOf<UGameplayEffect> UDC1AbilityData::FindEffectByTag(const FGameplayTag& EffectTag) const
{
	if (const TSubclassOf<UGameplayEffect>* Found = Effects.Find(EffectTag))
		return *Found;

	for (const TObjectPtr<UDC1AbilityData>& Base : BaseAbilityData)
	{
		if (IsValid(Base))
		{
			if (TSubclassOf<UGameplayEffect> Found = Base->FindEffectByTag(EffectTag))
				return Found;
		}
	}

	return nullptr;
}
