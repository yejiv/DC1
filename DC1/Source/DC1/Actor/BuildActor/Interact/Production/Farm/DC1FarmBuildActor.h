// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"
#include "DC1FarmBuildActor.generated.h"

USTRUCT(BlueprintType)
struct FDC1FarmPhaseInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	bool					IsGrowthPhase = { false };

	UPROPERTY(EditDefaultsOnly, meta = (EditCondition = "IsGrowthPhase", ClampMin = "0.1"))
	float					GrowthDuration = { 30.f };

	UPROPERTY(EditDefaultsOnly, meta = (Categories = "Work", EditCondition = "!IsGrowthPhase"))
	FGameplayTag			WorkTypeTag;

	UPROPERTY(EditDefaultsOnly)
	FText					OptionText;
};

UCLASS()
class DC1_API ADC1FarmBuildActor : public ADC1ProductionBuildActor
{
	GENERATED_BODY()

public:
	const TArray<FDC1FarmPhaseInfo>&	GetPhases() const { return Phases; }
	int32			GetPhaseIndex() const { return PhaseIndex; }

	virtual FGameplayTag	GetWorkTypeTag() const override;

protected:
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual bool	ShouldKeepOptionsWhileHeld() const override { return false; }

protected:
	virtual bool	CanAcceptWorker(const ADC1Character* InWorker) const override;
	virtual void	OnWorkTick(float InWork) override;
	virtual bool	CanPalWork() const override;

private:
	UFUNCTION()
	void			OnRep_PhaseIndex();

	const FDC1FarmPhaseInfo*	GetCurrentPhase() const;
	bool			IsGrowing() const;

	void			AddPhaseWork(float InWork, float InRequiredWork);
	void			AdvancePhase();
	void			StartGrowth();
	void			TickGrowth();
	void			SpawnHarvest();
	void			ApplyFarmPhase();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Farm")
	TArray<FDC1FarmPhaseInfo>	Phases;

	UPROPERTY(EditDefaultsOnly, Category = "Farm", meta = (ClampMin = "1"))
	int32						HarvestCount = { 1 };

	UPROPERTY(ReplicatedUsing = OnRep_PhaseIndex)
	int32						PhaseIndex = { 0 };

	FTimerHandle				GrowthTimerHandle;
};
