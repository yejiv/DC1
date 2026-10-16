#include "ViewModel/Player/DC1PlayerStatsViewModel.h"

#include "AbilitySystemComponent.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1PlayerStatsViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	ClearSource();

	if (false == ::IsValid(InPlayerState))
	{
		return;
	}

	AbilitySystemComponent = InPlayerState->GetAbilitySystemComponent();
	StatComponent = InPlayerState->GetStatComponent();

	if (StatComponent.IsValid())
	{
		StatComponent->OnStatPointsChanged.AddUObject(this, &ThisClass::HandleStatPointsChanged);

		if (AbilitySystemComponent.IsValid())
		{
			for (const EDC1PlayerStat Stat : StatComponent->GetStats())
			{
				const FGameplayAttribute Attribute = GetDisplayAttribute(Stat);
				if (false == Attribute.IsValid())
				{
					continue;
				}

				AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).AddUObject(this, &ThisClass::HandleStatAttributeChanged, Stat);
				SubscribedAttributes.Add(Attribute);
			}
		}
	}

	BroadcastAll();
}

void UDC1PlayerStatsViewModel::ClearSource()
{
	if (AbilitySystemComponent.IsValid())
	{
		for (const FGameplayAttribute& Attribute : SubscribedAttributes)
		{
			AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Attribute).RemoveAll(this);
		}
	}

	if (StatComponent.IsValid())
	{
		StatComponent->OnStatPointsChanged.RemoveAll(this);
	}

	AbilitySystemComponent.Reset();
	StatComponent.Reset();
	SubscribedAttributes.Reset();
	PendingStats = FDC1StatInvestment();
}

float UDC1PlayerStatsViewModel::GetStatValue(EDC1PlayerStat InStat) const
{
	const FGameplayAttribute Attribute = GetDisplayAttribute(InStat);
	if (false == AbilitySystemComponent.IsValid() || false == Attribute.IsValid())
	{
		return 0.f;
	}

	return AbilitySystemComponent->GetNumericAttribute(Attribute);
}

float UDC1PlayerStatsViewModel::GetStatBaseValue(EDC1PlayerStat InStat) const
{
	const FGameplayAttribute Attribute = GetDisplayAttribute(InStat);
	if (false == AbilitySystemComponent.IsValid() || false == Attribute.IsValid())
	{
		return 0.f;
	}

	return AbilitySystemComponent->GetNumericAttributeBase(Attribute);
}

bool UDC1PlayerStatsViewModel::IsStatBoosted(EDC1PlayerStat InStat) const
{
	return GetStatValue(InStat) > GetStatBaseValue(InStat) + KINDA_SMALL_NUMBER;
}

bool UDC1PlayerStatsViewModel::IsEnhanceableStat(EDC1PlayerStat InStat) const
{
	return StatComponent.IsValid() && StatComponent->IsEnhanceableStat(InStat);
}

int32 UDC1PlayerStatsViewModel::GetInvestedCount(EDC1PlayerStat InStat) const
{
	return StatComponent.IsValid() ? StatComponent->GetInvestedCount(InStat) : 0;
}

float UDC1PlayerStatsViewModel::GetEnhanceAmount(EDC1PlayerStat InStat) const
{
	return StatComponent.IsValid() ? StatComponent->GetEnhanceAmount(InStat) : 0.f;
}

int32 UDC1PlayerStatsViewModel::GetRemainingStatPoints() const
{
	return FMath::Max(0, GetAvailableStatPoints() - PendingStats.GetTotal());
}

bool UDC1PlayerStatsViewModel::CanAddPending(EDC1PlayerStat InStat) const
{
	return IsEnhanceableStat(InStat) && 0 < GetRemainingStatPoints();
}

void UDC1PlayerStatsViewModel::AddPending(EDC1PlayerStat InStat)
{
	if (false == CanAddPending(InStat))
	{
		return;
	}

	PendingStats.AddCount(InStat, 1);
	BroadcastAll();
}

void UDC1PlayerStatsViewModel::RemovePending(EDC1PlayerStat InStat)
{
	if (false == CanRemovePending(InStat))
	{
		return;
	}

	PendingStats.AddCount(InStat, -1);
	BroadcastAll();
}

void UDC1PlayerStatsViewModel::ConfirmPending()
{
	if (false == HasPending() || false == StatComponent.IsValid())
	{
		return;
	}

	StatComponent->RequestEnhanceStats(PendingStats);
	PendingStats = FDC1StatInvestment();
	BroadcastAll();
}

void UDC1PlayerStatsViewModel::CancelPending()
{
	if (false == HasPending())
	{
		return;
	}

	PendingStats = FDC1StatInvestment();
	BroadcastAll();
}

void UDC1PlayerStatsViewModel::HandleStatAttributeChanged(const FOnAttributeChangeData& InData, EDC1PlayerStat InStat)
{
	OnStatChanged.Broadcast(InStat);
}

void UDC1PlayerStatsViewModel::HandleStatPointsChanged()
{
	// 배분 중에 가용 포인트가 줄면(초기화 등) 배분이 더 이상 유효하지 않음
	if (PendingStats.GetTotal() > GetAvailableStatPoints())
	{
		PendingStats = FDC1StatInvestment();
	}

	BroadcastAll();
}

void UDC1PlayerStatsViewModel::BroadcastAll()
{
	OnRemainingStatPointsChanged.Broadcast(GetRemainingStatPoints());

	if (false == StatComponent.IsValid())
	{
		return;
	}

	for (const EDC1PlayerStat Stat : StatComponent->GetStats())
	{
		OnStatChanged.Broadcast(Stat);
	}
}

int32 UDC1PlayerStatsViewModel::GetAvailableStatPoints() const
{
	return StatComponent.IsValid() ? StatComponent->GetAvailableStatPoints() : 0;
}

FGameplayAttribute UDC1PlayerStatsViewModel::GetDisplayAttribute(EDC1PlayerStat InStat) const
{
	const FDC1PlayerStatDefinition* Definition = StatComponent.IsValid() ? StatComponent->GetStatDefinition(InStat) : nullptr;
	return nullptr != Definition ? Definition->DisplayAttribute : FGameplayAttribute();
}
