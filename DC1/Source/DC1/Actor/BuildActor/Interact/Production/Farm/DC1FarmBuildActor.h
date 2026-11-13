// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"
#include "DC1FarmBuildActor.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;

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
	// ISM 작물 메쉬, null == 안보임
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UStaticMesh>	CropMesh = { nullptr };
};

UCLASS()
class DC1_API ADC1FarmBuildActor : public ADC1ProductionBuildActor
{
	GENERATED_BODY()

public:
	ADC1FarmBuildActor();

public:
	const TArray<FDC1FarmPhaseInfo>&	GetPhases() const { return Phases; }
	int32			GetPhaseIndex() const { return PhaseIndex; }

	virtual FGameplayTag	GetWorkTypeTag() const override;

protected:
	virtual void	BeginPlay() override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual bool	ShouldKeepOptionsWhileHeld() const override { return false; }

public:
	virtual void	OnPostLoad() override;

protected:
	virtual void	ApplyState() override;
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

	void			BuildCropGrid();
	void			RefreshCropMesh();

private:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInstancedStaticMeshComponent>	CropISM = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Farm|Crop", meta = (ClampMin = "1"))
	int32						CropRows = { 10 };

	UPROPERTY(EditDefaultsOnly, Category = "Farm|Crop", meta = (ClampMin = "1"))
	int32						CropCols = { 10 };

	UPROPERTY(EditDefaultsOnly, Category = "Farm|Crop", meta = (ClampMin = "0.0"))
	float						CropSpacing = { 20.f };

	UPROPERTY(EditDefaultsOnly, Category = "Farm")
	TArray<FDC1FarmPhaseInfo>	Phases;

	UPROPERTY(EditDefaultsOnly, Category = "Farm", meta = (ClampMin = "1"))
	int32						HarvestCount = { 1 };

	UPROPERTY(ReplicatedUsing = OnRep_PhaseIndex, SaveGame)
	int32						PhaseIndex = { 0 };

	FTimerHandle				GrowthTimerHandle;
};
