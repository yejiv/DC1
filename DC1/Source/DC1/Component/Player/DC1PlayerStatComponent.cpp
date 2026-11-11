#include "Component/Player/DC1PlayerStatComponent.h"

#include "Net/UnrealNetwork.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"

#pragma region Stat Investment

int32 FDC1StatInvestment::GetCount(EDC1PlayerStat InStat) const
{
	switch (InStat)
	{
	case EDC1PlayerStat::Health:	return Health;
	case EDC1PlayerStat::Stamina:	return Stamina;
	case EDC1PlayerStat::Attack:	return Attack;
	case EDC1PlayerStat::WorkSpeed:	return WorkSpeed;
	case EDC1PlayerStat::Weight:	return Weight;
	default:						return 0;
	}
}

int32 FDC1StatInvestment::GetTotal() const
{
	return Health + Stamina + Attack + WorkSpeed + Weight;
}

void FDC1StatInvestment::AddCount(EDC1PlayerStat InStat, int32 InDelta)
{
	if (int32* Count = FindCount(InStat))
	{
		*Count = FMath::Max(0, *Count + InDelta);
	}
}

int32* FDC1StatInvestment::FindCount(EDC1PlayerStat InStat)
{
	switch (InStat)
	{
	case EDC1PlayerStat::Health:	return &Health;
	case EDC1PlayerStat::Stamina:	return &Stamina;
	case EDC1PlayerStat::Attack:	return &Attack;
	case EDC1PlayerStat::WorkSpeed:	return &WorkSpeed;
	case EDC1PlayerStat::Weight:	return &Weight;
	default:						return nullptr;
	}
}

#pragma endregion

#pragma region Player Stat Component

UDC1PlayerStatComponent::UDC1PlayerStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	StatDefinitions =
	{
		{ EDC1PlayerStat::Health,		{ UDC1PlayerSet::GetMaxHealthAttribute(),	UDC1PlayerSet::GetHealthAttribute(),	100.f } },
		{ EDC1PlayerStat::Stamina,		{ UDC1PlayerSet::GetMaxStaminaAttribute(),	UDC1PlayerSet::GetStaminaAttribute(),	10.f } },
		{ EDC1PlayerStat::Attack,		{ UDC1PlayerSet::GetAttackPowerAttribute(),	FGameplayAttribute(),					2.f } },
		{ EDC1PlayerStat::Defense,		{ UDC1PlayerSet::GetDefensePowerAttribute(),	FGameplayAttribute(),					0.f } },
		{ EDC1PlayerStat::WorkSpeed,	{ UDC1PlayerSet::GetWorkSpeedAttribute(),	FGameplayAttribute(),					50.f } },
		{ EDC1PlayerStat::Weight,		{ UDC1PlayerSet::GetMaxWeightAttribute(),	FGameplayAttribute(),					50.f } },
	};
}

TArray<EDC1PlayerStat> UDC1PlayerStatComponent::GetStats() const
{
	TArray<EDC1PlayerStat> Stats;
	StatDefinitions.GetKeys(Stats);
	return Stats;
}

int32 UDC1PlayerStatComponent::GetAvailableStatPoints() const
{
	const UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (nullptr == ASC)
	{
		return 0;
	}

	// 레벨 1부터 시작하므로 레벨업 횟수 = Level - 1
	const int32 Level = FMath::TruncToInt32(ASC->GetNumericAttribute(UDC1PlayerSet::GetLevelAttribute()));
	return FMath::Max(0, (Level - 1) - InvestedStats.GetTotal());
}

float UDC1PlayerStatComponent::GetEnhanceAmount(EDC1PlayerStat InStat) const
{
	const FDC1PlayerStatDefinition* Definition = GetStatDefinition(InStat);
	return nullptr != Definition ? Definition->EnhanceAmount : 0.f;
}

bool UDC1PlayerStatComponent::IsEnhanceableStat(EDC1PlayerStat InStat) const
{
	const FDC1PlayerStatDefinition* Definition = GetStatDefinition(InStat);
	return nullptr != Definition && Definition->DisplayAttribute.IsValid() && 0.f < Definition->EnhanceAmount;
}

bool UDC1PlayerStatComponent::CanEnhanceStats(const FDC1StatInvestment& InAllocation) const
{
	const int32 AvailablePoints = GetAvailableStatPoints();
	int32 TotalCount = 0;
	bool bValid = true;

	InAllocation.ForEachCount([&](EDC1PlayerStat InStat, int32 InCount)
	{
		if (0 == InCount)
		{
			return;
		}

		// 개별 상한을 먼저 검사해야 합계가 오버플로하지 않는다
		if (InCount < 0 || InCount > AvailablePoints || false == IsEnhanceableStat(InStat))
		{
			bValid = false;
			return;
		}

		TotalCount += InCount;
	});

	return bValid && 0 < TotalCount && TotalCount <= AvailablePoints;
}

void UDC1PlayerStatComponent::RequestEnhanceStats(const FDC1StatInvestment& InAllocation)
{
	if (false == CanEnhanceStats(InAllocation))
	{
		return;
	}

	Server_EnhanceStats(InAllocation);
}

void UDC1PlayerStatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetLevelAttribute()).AddUObject(this, &ThisClass::HandleLevelChanged);
	}
}

void UDC1PlayerStatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetLevelAttribute()).RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UDC1PlayerStatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UDC1PlayerStatComponent, InvestedStats, COND_OwnerOnly);
}

bool UDC1PlayerStatComponent::Server_EnhanceStats_Validate(const FDC1StatInvestment& InAllocation)
{
	bool bValid = true;
	InAllocation.ForEachCount([&bValid](EDC1PlayerStat InStat, int32 InCount)
	{
		bValid &= (0 <= InCount);
	});

	return bValid;
}

void UDC1PlayerStatComponent::Server_EnhanceStats_Implementation(const FDC1StatInvestment& InAllocation)
{
	// 포인트 부족 등은 레벨 복제 타이밍으로도 생길 수 있으므로 킥하지 않고 무시
	if (false == CanEnhanceStats(InAllocation))
	{
		return;
	}

	InAllocation.ForEachCount([this](EDC1PlayerStat InStat, int32 InCount)
	{
		if (0 == InCount)
		{
			return;
		}

		InvestedStats.AddCount(InStat, InCount);
		ApplyStatInvestment(InStat, InCount);
	});

	// 리슨 서버 호스트는 OnRep이 오지 않음
	OnRep_InvestedStats();
}

void UDC1PlayerStatComponent::OnRep_InvestedStats()
{
	OnStatPointsChanged.Broadcast();
}

void UDC1PlayerStatComponent::ApplyStatInvestment(EDC1PlayerStat InStat, int32 InDeltaCount)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const FDC1PlayerStatDefinition* Definition = GetStatDefinition(InStat);
	if (nullptr == ASC || nullptr == Definition || false == Definition->DisplayAttribute.IsValid())
	{
		return;
	}

	const float Delta = Definition->EnhanceAmount * InDeltaCount;
	if (0.f == Delta)
	{
		return;
	}

	// 덧셈으로 반영해야 다른 시스템이 올린 Base 몫을 지우지 않는다
	ASC->ApplyModToAttribute(Definition->DisplayAttribute, EGameplayModOp::Additive, Delta);

	// 최대치가 오르면 현재값도 같이 올리고, 내려가면 최대치 이하로 자른다
	if (Definition->CurrentAttribute.IsValid())
	{
		const float CurrentValue = ASC->GetNumericAttributeBase(Definition->CurrentAttribute);
		const float MaxValue = ASC->GetNumericAttribute(Definition->DisplayAttribute);
		ASC->SetNumericAttributeBase(Definition->CurrentAttribute, FMath::Min(CurrentValue + FMath::Max(0.f, Delta), MaxValue));
	}
}

void UDC1PlayerStatComponent::HandleLevelChanged(const FOnAttributeChangeData& InData)
{
	OnStatPointsChanged.Broadcast();
}

void UDC1PlayerStatComponent::OnPreSave()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (nullptr == ASC)
		return;

	SavedLevel		= ASC->GetNumericAttributeBase(UDC1PlayerSet::GetLevelAttribute());
	SavedExperience	= ASC->GetNumericAttributeBase(UDC1PlayerSet::GetExperienceAttribute());
	SavedHealth		= ASC->GetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute());
	SavedSatiety	= ASC->GetNumericAttributeBase(UDC1PlayerSet::GetSatietyAttribute());
}

void UDC1PlayerStatComponent::OnPostLoad()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (nullptr == ASC)
		return;

	// 1. 레벨 / 경험치 ( 0 = 이 필드가 없던 옛 세이브 → 건드리지 않음 )
	if (0.f < SavedLevel)
	{
		ASC->SetNumericAttributeBase(UDC1PlayerSet::GetLevelAttribute(), SavedLevel);
		ASC->SetNumericAttributeBase(UDC1PlayerSet::GetExperienceAttribute(), SavedExperience);
	}

	// 2. 투자 횟수만큼 최대치 다시 올리기 ( 평소 강화와 같은 함수 )
	InvestedStats.ForEachCount([this](EDC1PlayerStat InStat, int32 InCount)
	{
		if (0 < InCount)
		{
			ApplyStatInvestment(InStat, InCount);
		}
	});

	// 3. 현재값은 최대치가 정해진 뒤에 ( ApplyStatInvestment가 현재값도 올리므로 마지막에 덮어씀 )
	if (0.f < SavedHealth)
	{
		const float MaxHealth = ASC->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
		ASC->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), FMath::Min(SavedHealth, MaxHealth));
	}
	if (0.f < SavedSatiety)
	{
		const float MaxSatiety = ASC->GetNumericAttribute(UDC1PlayerSet::GetMaxSatietyAttribute());
		ASC->SetNumericAttributeBase(UDC1PlayerSet::GetSatietyAttribute(), FMath::Min(SavedSatiety, MaxSatiety));
	}

	// 리슨 서버 호스트는 OnRep이 안 옴
	OnRep_InvestedStats();
}

UAbilitySystemComponent* UDC1PlayerStatComponent::GetAbilitySystemComponent() const
{
	return UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner());
}

#pragma endregion
