// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "DC1ProductionBuildActor.generated.h"

DECLARE_MULTICAST_DELEGATE(FDC1OnProductionChanged);

UCLASS()
class DC1_API ADC1ProductionBuildActor : public ADC1InteractBuildActor
{
	GENERATED_BODY()
	
public:
	FDC1OnProductionChanged			OnProductionChanged;

public:
	ADC1ProductionBuildActor();

public:
	FName			GetProductionID() const { return ProductItemID; }
	float			GetProductionProgress() const { return ProductionProgress; }
	int32			GetStoredCount() const { return StoredCount; }
	int32			GetMaxStoredCount() const { return MaxStoredCount; }

protected:
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;

protected:
	virtual bool	CanAcceptWorker() const override;
	virtual void	OnWorkTick(float InWork) override;

	UFUNCTION()
	void			OnRep_ProductionProgress();

	UFUNCTION()
	void			OnRep_StoredCount(int32 OldCount);

private:
	// 서버 전용
	void			SetStoredCount(int32 InNewCount);

	// 서버 SetStoredCount / 클라 OnRep 수렴점
	void			ApplyStoredCount(int32 InOldCount);

protected:
	// DT_Item의 Item ID 값
	UPROPERTY(EditDefaultsOnly, Category = "Production")
	FName							ProductItemID = { NAME_None };

	// 아이템 1개당 생산 필요 작업량
	UPROPERTY(EditdefaultsOnly, Category = "Prudoction", meta = (ClampMin = "0.01"))
	float							RequiredWorkPerItem = { 100.f };

	UPROPERTY(EditDefaultsOnly, Category = "Production", meta = (ClampMin = "1"))
	int32							MaxStoredCount = { 1 };

	UPROPERTY(EditDefaultsOnly, Category = "Production|Interact")
	TArray<FDC1InteractOption>		CollectOptions;

private:
	UPROPERTY(ReplicatedUsing = OnRep_ProductionProgress)
	float							ProductionProgress = { 0.f };

	UPROPERTY(ReplicatedUsing = OnRep_StoredCount)
	int32							StoredCount = { 0 };

private:
	// 누적 작업량 - 서버
	float							CurrentProductionWork = { 0.f };
};
