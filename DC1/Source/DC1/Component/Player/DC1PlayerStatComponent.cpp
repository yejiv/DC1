#include "Component/Player/DC1PlayerStatComponent.h"

#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"

#include "Tags/DC1GamePlayTags.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Player/DC1PlayerExpData.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1TechTreeComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"

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

		// 경험치 이벤트 관련 델리게이트 추가
		ExpFilter.AddTag(DC1PlayerTags::Event_Exp_Player);
		ExpFilter.AddTag(DC1PlayerTags::Event_Exp_Shared);
		ExpEventHandle = ASC->AddGameplayEventTagContainerDelegate(ExpFilter, 
			FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::HandleExpEvent));
	}
}

void UDC1PlayerStatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetLevelAttribute()).RemoveAll(this);
		ASC->RemoveGameplayEventTagContainerDelegate(ExpFilter, ExpEventHandle);
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

void UDC1PlayerStatComponent::HandleExpEvent(FGameplayTag InEventTag, const FGameplayEventData* InPayload)
{
	if (nullptr == InPayload)
		return;

	GainExperience(FMath::RoundToInt(InPayload->EventMagnitude));
}

void UDC1PlayerStatComponent::GainExperience(int32 InAmount)
{
	// 서버에서만 처리
	if (false == GetOwner()->HasAuthority() || InAmount <= 0)
		return;
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	const UDC1PlayerExpData* ExpData = UDC1AssetManager::GetAssetByTag<UDC1PlayerExpData>(DC1AssetTags::Asset_Player_ExpData);
	if (nullptr == ASC || nullptr == ExpData)
		return;

	// 현재 레벨이 최대 레벨보다 높은 경우 반환
	int32 Level = FMath::TruncToInt32(ASC->GetNumericAttributeBase(UDC1PlayerSet::GetLevelAttribute()));
	if (Level >= ExpData->MaxLevel)
		return;

	const int32 Gained = FMath::RoundToInt(InAmount * ExpData->ExpRate);
	if (Gained <= 0)
		return;

	float Experience = ASC->GetNumericAttributeBase(UDC1PlayerSet::GetExperienceAttribute()) + static_cast<float>(Gained);

	int32 LevelsGained = { 0 }; // 레벨업 횟수
	while (Level < ExpData->MaxLevel)
	{
		const int32 Need = ExpData->GetExpToNextLevel(Level);

		// 경험치 필요량 0 or 필요량보다 부족한 경우 레벨 상승 X
		if (Need <= 0 || Experience < static_cast<float>(Need))
			break;

		Experience -= static_cast<float>(Need);
		++Level;
		++LevelsGained;
	}

	// 최대 레벨인 경우 경험치 0 고정
	if (Level >= ExpData->MaxLevel)
	{
		Experience = 0.f;
	}

	if (0 < LevelsGained)
	{
		ASC->SetNumericAttributeBase(UDC1PlayerSet::GetLevelAttribute(), static_cast<float>(Level));
		if (0 < ExpData->TechPointsPerLevel)
		{
			const ADC1PlayerState* PS = GetOwner<ADC1PlayerState>();
			UDC1TechTreeComponent* TechTree = (nullptr != PS) ? PS->GetTechTreeComponent() : nullptr;
			if (nullptr != TechTree)
			{
				TechTree->AddTechPoints(LevelsGained * ExpData->TechPointsPerLevel);
			}
		}

		Client_PlayLevelUpSound();
		Multicast_PlayLevelUpFX();
	}
	ASC->SetNumericAttributeBase(UDC1PlayerSet::GetExperienceAttribute(), Experience);
}

void UDC1PlayerStatComponent::Client_PlayLevelUpSound_Implementation()
{
	UGameplayStatics::PlaySound2D(this, LevelUpSound);
}

void UDC1PlayerStatComponent::Multicast_PlayLevelUpFX_Implementation()
{
	if (nullptr == LevelUpFX || true == IsRunningDedicatedServer())
		return;

	const ADC1PlayerState* PS = GetOwner<ADC1PlayerState>();
	const ACharacter* Character = (nullptr != PS) ? PS->GetPawn<ACharacter>() : nullptr;
	if (nullptr == Character)
		return;

	UNiagaraFunctionLibrary::SpawnSystemAttached(LevelUpFX, Character->GetMesh(), NAME_None,
		FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::KeepRelativeOffset, true);
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
