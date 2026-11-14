#include "AbilitySystem/Effects/DC1GameplayEffect_SatietyRestore.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"

UDC1GameplayEffect_SatietyRestore::UDC1GameplayEffect_SatietyRestore(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DC1EffectTags::SetByCaller_Satiety;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UDC1PlayerSet::GetSatietyAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(Modifier);
}
