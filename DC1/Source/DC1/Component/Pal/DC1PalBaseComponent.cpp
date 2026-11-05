// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Pal/DC1PalBaseComponent.h"
#include "GameMode/DC1GameState.h"

#include "Character/Player/DC1PlayerState.h"
#include "Character/Pal/DC1PalCharacter.h"

#include "Component/Pal/DC1PalPartyComponent.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"

#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystemComponent.h"

#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

UDC1PalBaseComponent::UDC1PalBaseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);
}

void UDC1PalBaseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, BasePals);
}

void UDC1PalBaseComponent::InitBase()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	if (BasePals.Num() < BaseCapacity)
	{
		BasePals.SetNum(BaseCapacity);
	}

	GetWorld()->GetTimerManager().SetTimer(BaseHealTimer, this, &ThisClass::HealBasePals, BaseHealInterval, true);
}

void UDC1PalBaseComponent::ReleaseBase(APlayerState* InDestroyer)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(BaseHealTimer);

	for (int32 Index = BaseSpawnedPals.Num() - 1; Index >= 0; --Index)
	{
		ADC1PalCharacter* Spawned = BaseSpawnedPals[Index];
		BaseSpawnedPals.RemoveAt(Index);

		if (true == ::IsValid(Spawned))
		{
			StoreFoldedPal(Spawned->Fold());
		}
	}

	const ADC1GameState* GS = ADC1GameState::Get(this);
	const ADC1PlayerState* DestroyerState = Cast<ADC1PlayerState>(InDestroyer);
	UDC1PalPartyComponent* DestroyerParty = (true == ::IsValid(DestroyerState)) ? DestroyerState->GetPalPartyComponent() : nullptr;

	for (FDC1PalInstance& Pal : BasePals)
	{
		if (false == Pal.IsValidPal())
		{
			continue;
		}

		UDC1PalPartyComponent* OwnerParty = (true == ::IsValid(GS)) ? GS->FindPalPartyByPlayerID(Pal.OwnerPlayerID) : nullptr;
		if (true == ::IsValid(OwnerParty) && true == OwnerParty->ReceivePal(Pal))
		{
			Pal = FDC1PalInstance();
			continue;
		}

		if (true == ::IsValid(DestroyerParty) && true == DestroyerParty->ReceivePal(Pal))
		{
			Pal = FDC1PalInstance();
			continue;
		}

		UE_LOG(LogTemp, Warning, TEXT("[Pal] Base removed: owner and destroyer cannot take it. Pal %s was dropped."), *Pal.PalID.ToString(EGuidFormats::Digits));

		Pal = FDC1PalInstance();
	}
}

void UDC1PalBaseComponent::RefreshBasePals()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	ADC1PalBoxActor* PalBox = GetActiveBase();

	// 거점이 없어지거나 칸에서 빠진 애들 해제 및 접음, 체력은 옮겨간 칸에 반영
	for (int32 Index = BaseSpawnedPals.Num() - 1; Index >= 0; --Index)
	{
		ADC1PalCharacter* Spawned = BaseSpawnedPals[Index];
		if (false == ::IsValid(Spawned))
		{
			BaseSpawnedPals.RemoveAt(Index);
			continue;
		}

		const bool bStillInBase = (nullptr != PalBox) && BasePals.ContainsByPredicate([Spawned](const FDC1PalInstance& Pal)
			{
				return (Pal.PalID == Spawned->GetInstanceID());
			});

		if (false == bStillInBase)
		{
			BaseSpawnedPals.RemoveAt(Index);
			StoreFoldedPal(Spawned->Fold());
		}
	}

	if (nullptr == PalBox)
	{
		return;
	}

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (nullptr == NavSystem)
	{
		return;
	}

	const ADC1GameState* GS = ADC1GameState::Get(this);

	// 칸에 있는데 안 나온 팰을 꺼냄, 기절한 팰은 회복될 때까지 안 나옴
	for (const FDC1PalInstance& Pal : BasePals)
	{
		if (false == Pal.IsDeployable())
		{
			continue;
		}

		const bool bSpawned = BaseSpawnedPals.ContainsByPredicate([&Pal](const ADC1PalCharacter* Spawned)
			{
				return (Spawned->GetInstanceID() == Pal.PalID);
			});

		if (true == bSpawned)
		{
			continue;
		}

		FNavLocation SpawnLocation;
		if (false == NavSystem->GetRandomPointInNavigableRadius(PalBox->GetActorLocation(), PalBox->GetBaseRadius(), SpawnLocation))
		{
			UE_LOG(LogTemp, Warning, TEXT("[Pal] Base: no navigable spot near the pal box."));
			return;
		}

		const UDC1PalPartyComponent* OwnerParty = (true == ::IsValid(GS)) ? GS->FindPalPartyByPlayerID(Pal.OwnerPlayerID) : nullptr;
		ADC1PlayerState* OwnerState = (nullptr != OwnerParty) ? Cast<ADC1PlayerState>(OwnerParty->GetOwner()) : nullptr;

		if (ADC1PalCharacter* Spawned = ADC1PalCharacter::SpawnFromInstanceData(GetWorld(), Pal, FTransform(SpawnLocation.Location), OwnerState, PalBox))
		{
			BaseSpawnedPals.Add(Spawned);
		}
	}
}

void UDC1PalBaseComponent::FoldBasePal(const FGuid& InPalID)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	const int32 SpawnedIndex = BaseSpawnedPals.IndexOfByPredicate([&InPalID](const ADC1PalCharacter* Spawned)
		{
			return (true == ::IsValid(Spawned) && Spawned->GetInstanceID() == InPalID);
		});

	if (INDEX_NONE == SpawnedIndex)
	{
		return;
	}

	ADC1PalCharacter* Spawned = BaseSpawnedPals[SpawnedIndex];
	BaseSpawnedPals.RemoveAt(SpawnedIndex);

	StoreFoldedPal(Spawned->Fold());
}

void UDC1PalBaseComponent::StoreFoldedPal(const FDC1PalInstance& InFolded)
{
	const int32 Index = BasePals.IndexOfByPredicate([&InFolded](const FDC1PalInstance& Pal)
		{
			return (Pal.PalID == InFolded.PalID);
		});

	if (INDEX_NONE == Index)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Base: folded pal %s is not in base slots."), *InFolded.PalID.ToString(EGuidFormats::Digits));
		return;
	}

	BasePals[Index] = InFolded;
}

void UDC1PalBaseComponent::OnRep_BasePals()
{
	NotifyBaseChanged();
}

void UDC1PalBaseComponent::HealBasePals()
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry) || Registry->BoxHealRatePerSecond <= 0.f)
	{
		return;
	}

	bool bChanged = false;

	for (FDC1PalInstance& Pal : BasePals)
	{
		if (false == Pal.IsValidPal())
		{
			continue;
		}

		const float MaxHealth = UDC1PalPartyComponent::GetMaxHealthOf(Pal);
		if (Pal.CurrentHealth >= MaxHealth)
		{
			continue;
		}

		Pal.CurrentHealth = FMath::Min(MaxHealth, Pal.CurrentHealth + (MaxHealth * Registry->BoxHealRatePerSecond * BaseHealInterval));
		bChanged = true;
	}

	if (false == bChanged)
	{
		return;
	}

	// 나와 있는 거점 팰은 칸 값을 따름
	for (ADC1PalCharacter* Spawned : BaseSpawnedPals)
	{
		const FDC1PalInstance* Pal = BasePals.FindByPredicate([Spawned](const FDC1PalInstance& Other)
			{
				return (::IsValid(Spawned) && Other.PalID == Spawned->GetInstanceID());
			});

		if (nullptr == Pal)
		{
			continue;
		}

		if (UAbilitySystemComponent* ASC = Spawned->GetAbilitySystemComponent())
		{
			ASC->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), Pal->CurrentHealth);
		}
	}

	// 기절에서 회복된 거점 팰을 꺼냄
	RefreshBasePals();

	NotifyBaseChanged();
}

ADC1PalBoxActor* UDC1PalBaseComponent::GetActiveBase() const
{
	ADC1PalBoxActor* PalBox = GetOwner<ADC1PalBoxActor>();
	const ADC1GameState* GS = ADC1GameState::Get(this);
	if (false == ::IsValid(PalBox) || false == ::IsValid(GS) || PalBox != GS->GetBasePalBox())
	{
		return nullptr;
	}

	return PalBox;
}

void UDC1PalBaseComponent::NotifyBaseChanged() const
{
	ADC1GameState* GS = ADC1GameState::Get(this);
	if (true == ::IsValid(GS))
	{
		GS->NotifyBaseChanged();
	}
}
