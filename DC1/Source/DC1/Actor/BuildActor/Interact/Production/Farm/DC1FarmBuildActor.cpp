// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Interact/Production/Farm/DC1FarmBuildActor.h"

#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include "Item/Pickup/DC1PickupItem.h"

#include "Tags/Build/DC1GameplayTag_Build.h"
#include "Tags/Player/DC1GameplayTag_Player.h"

namespace
{
	constexpr float GrowthTickInterval = 0.1f;
}

FGameplayTag ADC1FarmBuildActor::GetWorkTypeTag() const
{
	if (false == IsCompleted())
	{
		return Super::GetWorkTypeTag();
	}

	const FDC1FarmPhaseInfo* Phase = GetCurrentPhase();
	if (nullptr == Phase || true == Phase->IsGrowthPhase)
	{
		return FGameplayTag::EmptyTag;
	}

	return Phase->WorkTypeTag;
}

void ADC1FarmBuildActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, PhaseIndex);
}

void ADC1FarmBuildActor::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	Super::GetInteractOptions(OutOptions);

	if (false == IsCompleted())
	{
		return;
	}

	const FDC1FarmPhaseInfo* Phase = GetCurrentPhase();
	if (nullptr == Phase)
	{
		return;
	}

	if (true == Phase->IsGrowthPhase)
	{
		OutOptions.RemoveAll([](const FDC1InteractOption& Option)
			{
				return Option.InteractionTag == DC1InteractTags::Interact_Work;
			});
		return;
	}

	FDC1InteractOption* WorkOption = OutOptions.FindByPredicate([](const FDC1InteractOption& Option)
		{
			return Option.InteractionTag == DC1InteractTags::Interact_Work;
		});
	if (nullptr != WorkOption)
	{
		WorkOption->DisplayText = Phase->OptionText;
	}
}

void ADC1FarmBuildActor::OnPostLoad()
{
	Super::OnPostLoad();

	// BP에서 Phases 개수가 줄었을 때 대비
	if (false == Phases.IsValidIndex(PhaseIndex))
	{
		PhaseIndex = 0;
		CurrentProductionWork = 0.f;
	}

	// 1. 단계별 분모로 진행도 재계산
	const FDC1FarmPhaseInfo* Phase = GetCurrentPhase();
	const float Required = (nullptr != Phase && true == Phase->IsGrowthPhase) ? Phase->GrowthDuration : RequiredWorkPerItem;
	ProductionProgress = (Required > 0.f) ? CurrentProductionWork / Required : 0.f;

	// 2. 성장 중이면 타이머 재시작
	if (true == IsGrowing())
	{
		StartGrowth();
	}
}

bool ADC1FarmBuildActor::CanAcceptWorker(const ADC1Character* InWorker) const
{
	if (true == IsGrowing())
	{
		return false;
	}

	return Super::CanAcceptWorker(InWorker);
}

void ADC1FarmBuildActor::OnWorkTick(float InWork)
{
	if (false == IsCompleted())
	{
		Super::OnWorkTick(InWork);
		return;
	}

	if (RequiredWorkPerItem <= 0.f || true == Phases.IsEmpty() || true == IsGrowing())
	{
		return;
	}

	AddPhaseWork(InWork, RequiredWorkPerItem);
}

bool ADC1FarmBuildActor::CanPalWork() const
{
	if (true == IsGrowing())
	{
		return false;
	}

	return Super::CanPalWork();
}

void ADC1FarmBuildActor::OnRep_PhaseIndex()
{
	ApplyFarmPhase();
}

const FDC1FarmPhaseInfo* ADC1FarmBuildActor::GetCurrentPhase() const
{
	return Phases.IsValidIndex(PhaseIndex) ? &Phases[PhaseIndex] : nullptr;
}

bool ADC1FarmBuildActor::IsGrowing() const
{
	const FDC1FarmPhaseInfo* Phase = GetCurrentPhase();

	return (true == IsCompleted()) && (nullptr != Phase) && (true == Phase->IsGrowthPhase);
}

void ADC1FarmBuildActor::AddPhaseWork(float InWork, float InRequiredWork)
{
	CurrentProductionWork += InWork;

	if (CurrentProductionWork < InRequiredWork)
	{
		ProductionProgress = CurrentProductionWork / InRequiredWork;
		OnProductionChanged.Broadcast();
		return;
	}

	// 작업 경험치 - 작업 단계 완료 시 작업 중인 플레이어 ( 성장 단계 제외, AdvancePhase 의 ClearWorkers 전, TODO 임시값 )
	if (false == IsGrowing())
	{
		GrantExpToPlayerWorkers(DC1PlayerTags::Event_Exp_Player_Work, 10);
	}

	AdvancePhase();
}

void ADC1FarmBuildActor::AdvancePhase()
{
	GetWorldTimerManager().ClearTimer(GrowthTimerHandle);

	if (PhaseIndex >= Phases.Num() - 1)
	{
		SpawnHarvest();
	}

	PhaseIndex = (PhaseIndex + 1) % Phases.Num();

	CurrentProductionWork = 0.f;
	ProductionProgress = 0.f;

	ClearWorkers();
	ClearPalWorkers();

	if (true == IsGrowing())
	{
		StartGrowth();
	}

	ApplyFarmPhase();
}

void ADC1FarmBuildActor::StartGrowth()
{
	GetWorldTimerManager().SetTimer(GrowthTimerHandle, this, &ThisClass::TickGrowth, GrowthTickInterval, true);
}

void ADC1FarmBuildActor::TickGrowth()
{
	const FDC1FarmPhaseInfo* Phase = GetCurrentPhase();
	if (nullptr == Phase)
	{
		return;
	}

	AddPhaseWork(GrowthTickInterval, Phase->GrowthDuration);
}

void ADC1FarmBuildActor::SpawnHarvest()
{
	if (true == ProductItemID.IsNone())
	{
		return;
	}

	ADC1PickupItem::SpawnDrop(GetWorld(), ProductItemID, HarvestCount, GetCheckBoxCenter());
}

void ADC1FarmBuildActor::ApplyFarmPhase()
{
	OnInteractOptionsChanged.Broadcast();
	OnProductionChanged.Broadcast();
}
