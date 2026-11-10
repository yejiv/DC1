// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalRideViewModel.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Component/Pal/DC1PalRideComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"

void UDC1PalRideViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PalPartyComponent = InPlayerState->GetPalPartyComponent();

	if (false == PalPartyComponent.IsValid())
		return;

	PalPartyComponent->OnPalSlotChanged.AddUObject(this, &ThisClass::HandleDeployedPalChanged);

	HandleDeployedPalChanged();
}

void UDC1PalRideViewModel::ClearSource()
{
	UnbindRideSkillCooldowns();

	if (true == RideComponent.IsValid())
	{
		RideComponent->OnRideStateChanged.RemoveAll(this);
		RideComponent.Reset();
	}

	if (true == PalPartyComponent.IsValid())
	{
		PalPartyComponent->OnPalSlotChanged.RemoveAll(this);
		PalPartyComponent.Reset();
	}

	bRideAvailable = false;
	bRiding = false;
	RideSkillNames.Reset();
}

bool UDC1PalRideViewModel::IsRideSkillCoolingDown(int32 InIndex) const
{
	return (true == RideSkillCooldowns.IsValidIndex(InIndex)) && (true == RideSkillCooldowns[InIndex].bActive);
}

float UDC1PalRideViewModel::GetRideSkillCooldownRatio(int32 InIndex) const
{
	if (false == IsRideSkillCoolingDown(InIndex))
		return 1.f;

	const FDC1RideSkillCooldown& Cooldown = RideSkillCooldowns[InIndex];
	if (Cooldown.Duration <= 0.f)
		return 1.f;

	// 태그가 늦게 와서 다 찼는데도 태그가 남아 있으면 1 에서 멈춘다. 끝은 태그가 떨어질 때
	return FMath::Clamp((GetWorldTime() - Cooldown.StartTime) / Cooldown.Duration, 0.f, 1.f);
}

void UDC1PalRideViewModel::SetRideHoldPressed(bool InPressed)
{
	OnRideHoldChanged.Broadcast(InPressed);
}

void UDC1PalRideViewModel::NotifyRideHoldCompleted()
{
	OnRideHoldCompleted.Broadcast();
}

void UDC1PalRideViewModel::HandleDeployedPalChanged()
{
	if (false == PalPartyComponent.IsValid())
		return;

	const ADC1PalCharacter* DeployedPal = PalPartyComponent->GetDeployedPal();
	UDC1PalRideComponent* NewRideComponent = (nullptr != DeployedPal) ? DeployedPal->GetRideComponent() : nullptr;

	if (RideComponent.Get() != NewRideComponent)
	{
		if (true == RideComponent.IsValid())
		{
			RideComponent->OnRideStateChanged.RemoveAll(this);
		}

		RideComponent = NewRideComponent;

		if (true == RideComponent.IsValid())
		{
			RideComponent->OnRideStateChanged.AddUObject(this, &ThisClass::HandleRideStateChanged);
		}
	}

	const UDC1PalData* PalData = (nullptr != DeployedPal) ? DeployedPal->GetPalData() : nullptr;
	bRideAvailable = (nullptr != PalData) && (true == PalData->bRideable) && (true == RideComponent.IsValid());

	RefreshRideSkillNames(bRideAvailable ? PalData : nullptr);
	RefreshRideSkillCooldowns(bRideAvailable ? DeployedPal : nullptr, bRideAvailable ? PalData : nullptr);

	HandleRideStateChanged();
}

void UDC1PalRideViewModel::HandleRideStateChanged()
{
	bRiding = (true == bRideAvailable) && (EDC1RideState::None != RideComponent->GetRideState());

	OnRideInfoChanged.Broadcast();
}

void UDC1PalRideViewModel::RefreshRideSkillNames(const UDC1PalData* InPalData)
{
	RideSkillNames.Reset();

	if (nullptr == InPalData)
		return;

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
		return;

	// 순서가 입력 순서라 정의가 없어도 자리는 남긴다
	for (const FGameplayTag& SkillTag : InPalData->SkillTags)
	{
		const FDC1PalSkillDef* SkillDef = Registry->FindSkillDef(SkillTag);
		RideSkillNames.Add((nullptr != SkillDef) ? SkillDef->DisplayName : FText::GetEmpty());
	}
}

void UDC1PalRideViewModel::RefreshRideSkillCooldowns(const ADC1PalCharacter* InPal, const UDC1PalData* InPalData)
{
	UAbilitySystemComponent* NewASC = (nullptr != InPal) ? InPal->GetAbilitySystemComponent() : nullptr;

	// 파티 칸이 바뀔 때마다 불리므로, 같은 팰이면 세던 쿨타임을 지우지 않는다
	if (nullptr != NewASC && nullptr != InPalData && PalAbilitySystemComponent.Get() == NewASC && RideSkillCooldowns.Num() == InPalData->SkillTags.Num())
		return;

	UnbindRideSkillCooldowns();

	if (nullptr == NewASC || nullptr == InPalData)
		return;

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
		return;

	PalAbilitySystemComponent = NewASC;

	// 순서는 RideSkillNames 와 같다
	for (const FGameplayTag& SkillTag : InPalData->SkillTags)
	{
		const FDC1PalSkillDef* SkillDef = Registry->FindSkillDef(SkillTag);

		FDC1RideSkillCooldown& Cooldown = RideSkillCooldowns.AddDefaulted_GetRef();
		Cooldown.CooldownTag = DC1PalTags::FindSkillCooldownTag(SkillTag);
		Cooldown.Duration = (nullptr != SkillDef) ? SkillDef->Cooldown : 0.f;

		if (false == Cooldown.CooldownTag.IsValid())
			continue;

		// 타기 전에 이미 쿨타임이면 남은 시간을 모르니 지금부터 센다
		if (true == NewASC->HasMatchingGameplayTag(Cooldown.CooldownTag))
		{
			Cooldown.bActive = true;
			Cooldown.StartTime = GetWorldTime();
		}

		if (false == BoundCooldownTags.Contains(Cooldown.CooldownTag))
		{
			BoundCooldownTags.Add(Cooldown.CooldownTag);
			CooldownTagHandles.Add(NewASC->RegisterGameplayTagEvent(Cooldown.CooldownTag, EGameplayTagEventType::NewOrRemoved)
				.AddUObject(this, &ThisClass::HandleCooldownTagChanged));
		}
	}
}

void UDC1PalRideViewModel::UnbindRideSkillCooldowns()
{
	if (true == PalAbilitySystemComponent.IsValid())
	{
		for (int32 Index = 0; Index < BoundCooldownTags.Num(); ++Index)
		{
			PalAbilitySystemComponent->UnregisterGameplayTagEvent(CooldownTagHandles[Index], BoundCooldownTags[Index], EGameplayTagEventType::NewOrRemoved);
		}
	}

	PalAbilitySystemComponent.Reset();
	RideSkillCooldowns.Reset();
	BoundCooldownTags.Reset();
	CooldownTagHandles.Reset();
}

void UDC1PalRideViewModel::HandleCooldownTagChanged(const FGameplayTag InTag, int32 InNewCount)
{
	const bool bStarted = (InNewCount > 0);

	for (int32 Index = 0; Index < RideSkillCooldowns.Num(); ++Index)
	{
		FDC1RideSkillCooldown& Cooldown = RideSkillCooldowns[Index];
		if (Cooldown.CooldownTag != InTag)
			continue;

		Cooldown.bActive = bStarted;
		Cooldown.StartTime = GetWorldTime();

		OnRideSkillCooldownChanged.Broadcast(Index, bStarted);
	}
}

float UDC1PalRideViewModel::GetWorldTime() const
{
	const UWorld* World = PalAbilitySystemComponent.IsValid() ? PalAbilitySystemComponent->GetWorld() : nullptr;
	return (nullptr != World) ? World->GetTimeSeconds() : 0.f;
}
