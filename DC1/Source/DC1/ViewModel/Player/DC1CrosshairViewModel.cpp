// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModel/Player/DC1CrosshairViewModel.h"

#include "AbilitySystemComponent.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1CrosshairViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	ClearSource();

	if (nullptr == InPlayerState)
		return;

	AbilitySystemComponent = InPlayerState->GetAbilitySystemComponent();

	if (false == AbilitySystemComponent.IsValid())
		return;

	RegisterStateHandle(DC1StateTags::State_ZoomIn);
	RegisterStateHandle(DC1StateTags::State_Weapon_Bow);

	FiredEventHandle = AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(FiredEventTag).AddUObject(this, &UDC1CrosshairViewModel::HandleFiredEvent);
}

void UDC1CrosshairViewModel::ClearSource()
{
	if (false == AbilitySystemComponent.IsValid())
		return;

	for (auto& Pair : StateTagHandles)
	{
		AbilitySystemComponent->UnregisterGameplayTagEvent(Pair.Value, Pair.Key);
	}
	StateTagHandles.Reset();

	AbilitySystemComponent->GenericGameplayEventCallbacks.FindOrAdd(FiredEventTag).Remove(FiredEventHandle);
	FiredEventHandle.Reset();

	AbilitySystemComponent.Reset();
	bVisible = false;
}

bool UDC1CrosshairViewModel::IsVisible() const
{
	return bVisible;
}

void UDC1CrosshairViewModel::HandleStateTagChanged(const FGameplayTag InTag, int32 InNewCount)
{
	RefreshVisibility();
}

void UDC1CrosshairViewModel::HandleFiredEvent(const FGameplayEventData* InPayload)
{
	if (nullptr == InPayload)
		return;

	float Strength = InPayload->EventMagnitude;
	OnFired.Broadcast(Strength);
}

bool UDC1CrosshairViewModel::EvaluateVisibility() const
{
	if (false == AbilitySystemComponent.IsValid())
		return false;

	if (AbilitySystemComponent->HasMatchingGameplayTag(DC1StateTags::State_ZoomIn) &&
		AbilitySystemComponent->HasMatchingGameplayTag(DC1StateTags::State_Weapon_Bow))
	{
		return true;
	}

	return false;
}

void UDC1CrosshairViewModel::RefreshVisibility()
{
	bool PreVisible = bVisible;
	bVisible = EvaluateVisibility();

	if(PreVisible != bVisible)
		OnVisibilityChanged.Broadcast(bVisible);
}

void UDC1CrosshairViewModel::RegisterStateHandle(const FGameplayTag& InTag)
{
	if (false == AbilitySystemComponent.IsValid())
		return;

	if (nullptr != StateTagHandles.Find(InTag))
		return;

	StateTagHandles.Add(InTag,
		AbilitySystemComponent->RegisterAndCallGameplayTagEvent(InTag,
			FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(this, &UDC1CrosshairViewModel::HandleStateTagChanged)));
}
