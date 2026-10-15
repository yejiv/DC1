// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "DC1ProductionBuildActor.generated.h"

DECLARE_MULTICAST_DELEGATE(FDC1OnProductionChanged);

class USphereComponent;

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
	virtual void	BeginPlay() override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual bool	ShouldKeepOptionsWhileHeld() const override;

	void			CollectProduct(ADC1Character* InCollect);

protected:
	virtual void	ApplyState() override;
	virtual bool	CanAcceptWorker() const override;
	virtual void	OnWorkTick(float InWork) override;

	virtual EDC1BuildInfoState			ComputeInfoState() const override;
	virtual TSubclassOf<UDC1BuildInfoWidget>	GetInfoWidgetClass(EDC1BuildInfoState InState) const override;

	virtual void	OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;

	UFUNCTION()
	void			OnRep_ProductionProgress();

	UFUNCTION()
	void			OnRep_StoredCount(int32 OldCount);

private:
	APawn*			GetLocalPawn() const;

	// 서버 전용
	void			SetStoredCount(int32 InNewCount);

	// 서버 SetStoredCount / 클라 OnRep 수렴점
	void			ApplyStoredCount(int32 InOldCount);

	void			RefreshLocalInRange();

	// 범위 구체 판정 - 로컬 플레이어 폰 한정
	UFUNCTION()
	void			OnInfoRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void			OnInfoRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// DT_Item의 Item ID 값
	UPROPERTY(EditDefaultsOnly, Category = "Production")
	FName							ProductItemID = { NAME_None };

	// 아이템 1개당 생산 필요 작업량
	UPROPERTY(EditdefaultsOnly, Category = "Production", meta = (ClampMin = "0.01"))
	float							RequiredWorkPerItem = { 100.f };

	UPROPERTY(EditDefaultsOnly, Category = "Production", meta = (ClampMin = "1"))
	int32							MaxStoredCount = { 1 };

	UPROPERTY(EditDefaultsOnly, Category = "Production|Interact")
	TArray<FDC1InteractOption>		CollectOptions;

	UPROPERTY(EditDefaultsOnly, Category = "Production|UI")
	TObjectPtr<USphereComponent>	InfoRangeSphere = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Production|UI")
	TSubclassOf<UDC1BuildInfoWidget>		ProductionInfoWidgetClass = { nullptr };

private:
	UPROPERTY(ReplicatedUsing = OnRep_ProductionProgress)
	float							ProductionProgress = { 0.f };

	UPROPERTY(ReplicatedUsing = OnRep_StoredCount)
	int32							StoredCount = { 0 };

private:
	// 누적 작업량 - 서버
	float							CurrentProductionWork = { 0.f };

	bool							IsLocalInRange = { false };
};
