// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DC1AbilitySystemComponent.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Ability/DC1AbilityData.h"

#pragma region AbilityHandle
void FDC1AbilitySetGrantedHandles::AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle)
{
	AbilitySpecHandles.Add(Handle);
}
void FDC1AbilitySetGrantedHandles::TakeFromAbilitySystem(UDC1AbilitySystemComponent* ASC)
{
	if (nullptr == ASC || false == ASC->IsOwnerActorAuthoritative()) return;

	for (const FGameplayAbilitySpecHandle& handle : AbilitySpecHandles)
	{
		if (handle.IsValid())
		{
			ASC->ClearAbility(handle);
		}
	}

	AbilitySpecHandles.Reset();
}
#pragma endregion

void UDC1AbilitySystemComponent::InitAbilitySystem(AActor* Owner, AActor* Avatar, const FGameplayTag& DataTag)
{
	if (nullptr == Owner)
	{
		return;
	}

	InitAbilityActorInfo(Owner, Avatar);
	DataAssetTag = DataTag;
	if (Owner->HasAuthority())
	{
		if (const UDC1AbilityData* AbilityData = UDC1AssetManager::GetAssetByTag<UDC1AbilityData>(DataTag))
		{
			AbilityData->GiveToAbilitySystem(this, GrantedHandles, GetOwner());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[DC1] InitAbilitySystem: No AbilityData for tag [%s] on [%s]. No abilities granted."),
				*DataTag.ToString(), *GetNameSafe(Owner));
		}
	}
}

void UDC1AbilitySystemComponent::UnInitializeAbilitySystem()
{
	if (IsOwnerActorAuthoritative())
	{
		GrantedHandles.TakeFromAbilitySystem(this);
		RemoveActiveEffects(FGameplayEffectQuery());
	}
	AbilityTagCache.Reset();
	ClearActorInfo();
}

void UDC1AbilitySystemComponent::RefreshAvatarActor(AActor* NewAvatarActor)
{
	InitAbilityActorInfo(GetOwner(), NewAvatarActor);
}

void UDC1AbilitySystemComponent::ClearAvatarActor()
{
	SetAvatarActor(nullptr);
}

bool UDC1AbilitySystemComponent::ActivateAbilityByTag(const FGameplayTag& AbilityTag)
{
	TArray<FGameplayAbilitySpecHandle> Handles;
	AbilityTagCache.MultiFind(AbilityTag, Handles);

	for (const FGameplayAbilitySpecHandle& handle : Handles)
	{
		if (TryActivateAbility(handle))
			return true;
	}
	return false;
}

void UDC1AbilitySystemComponent::CancelAbilityByTag(const FGameplayTag& AbilityTag)
{
	TArray<FGameplayAbilitySpecHandle> Handles;
	AbilityTagCache.MultiFind(AbilityTag, Handles);

	for (const FGameplayAbilitySpecHandle& handle : Handles)
	{
		CancelAbilityHandle(handle);
	}
}

bool UDC1AbilitySystemComponent::HasAbilityByTag(const FGameplayTag& AbilityTag)
{
	return AbilityTagCache.Contains(AbilityTag);
}

void UDC1AbilitySystemComponent::SetExclusiveStateTag(const FGameplayTag& AxisRootTag, const FGameplayTag& NewStateTag)
{
	if (!IsOwnerActorAuthoritative()) { return; }

	RemoveAxisStateTag(AxisRootTag);

	if (NewStateTag.IsValid())
		AddStateTag(NewStateTag);
}

void UDC1AbilitySystemComponent::AddStateTag(const FGameplayTag& StateTag)
{
	if (const UDC1AbilityData* AbilityData = UDC1AssetManager::GetAssetByTag<UDC1AbilityData>(DataAssetTag))
	{
		FGameplayEffectSpecHandle handle = MakeOutgoingSpec(AbilityData->FindEffectByTag(StateTag), 1.f, MakeEffectContext());
		if (handle.IsValid())
		{
			ApplyGameplayEffectSpecToSelf(*handle.Data.Get());
		}
	}
}

void UDC1AbilitySystemComponent::RemoveAxisStateTag(const FGameplayTag& AxisRootTag)
{
	RemoveActiveEffectsWithGrantedTags(AxisRootTag.GetSingleTagContainer());
}

void UDC1AbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	for (const FGameplayTag& Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		AbilityTagCache.Add(Tag, AbilitySpec.Handle);
	}
}

void UDC1AbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnRemoveAbility(AbilitySpec);

	for (const FGameplayTag& Tag : AbilitySpec.GetDynamicSpecSourceTags())
	{
		AbilityTagCache.RemoveSingle(Tag, AbilitySpec.Handle);
	}
}

void UDC1AbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();
	RebuildAbilityTagCache();
}

void UDC1AbilitySystemComponent::RebuildAbilityTagCache()
{
	AbilityTagCache.Reset();

	ABILITYLIST_SCOPE_LOCK();

	for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
	{
		for (const FGameplayTag& Tag : Spec.GetDynamicSpecSourceTags())
		{
			AbilityTagCache.Add(Tag, Spec.Handle);
		}
	}
}
