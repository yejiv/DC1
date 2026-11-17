// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "DC1ProductionBuildActor.generated.h"

DECLARE_MULTICAST_DELEGATE(FDC1OnProductionChanged);

class USphereComponent;
class ADC1PalCharacter;

UCLASS()
class DC1_API ADC1ProductionBuildActor : public ADC1InteractBuildActor
{
	GENERATED_BODY()
	
public:
	// 생산 상태 변경 ( NoItem, Compact, Detail )
	FDC1OnProductionChanged			OnProductionChanged;

public:
	ADC1ProductionBuildActor();

public:
	FName			GetProductionID() const { return ProductItemID; }
	float			GetProductionProgress() const { return ProductionProgress; }
	int32			GetStoredCount() const { return StoredCount; }
	int32			GetMaxStoredCount() const { return MaxStoredCount; }

	virtual FGameplayTag	GetWorkTypeTag() const override;

protected:
	virtual void	BeginPlay() override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual void	GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual bool	ShouldKeepOptionsWhileHeld() const override;
	// Save Interface override
	virtual void	OnPostLoad() override;

	void			CollectProduct(ADC1Character* InCollect);

protected:
	virtual void	ApplyState() override;
	virtual bool	CanAcceptWorker(const ADC1Character* InWorker) const override;
	virtual void	OnWorkTick(float InWork) override;

	virtual EDC1BuildInfoState			ComputeInfoState() const override;
	virtual TSubclassOf<UDC1BuildInfoWidget>	GetInfoWidgetClass(EDC1BuildInfoState InState) const override;

	virtual void	OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;

	virtual bool	CanPalWork() const override;
	virtual bool	HasPalWorkerSlot() const override;

	UFUNCTION()
	void			OnRep_ProductionProgress();

	UFUNCTION()
	void			OnRep_StoredCount(int32 OldCount);

	UFUNCTION()
	void			OnRep_ProductItemID();

	virtual void	OnProductCollected(int32 InCollectedCount) {}

	// 서버 전용
	void			SetStoredCount(int32 InNewCount);
	// 서버 RegisterRecipe / 클라 OnRep 수렴점
	void			ApplyProductItemID();

	static void		OverrideOptions(TArray<FDC1InteractOption>& OutOptions, const TArray<FDC1InteractOption>& InOverrides);

private:
	APawn*			GetLocalPawn() const;
	// 서버 SetStoredCount / 클라 OnRep 수렴점
	void			ApplyStoredCount(int32 InOldCount);

	void			RefreshLocalInRange();
	// 건설 완료 + 생산 아이템 지정 + 작업량 유효
	bool			CanProduce() const;

	// 범위 구체 판정 - 로컬 플레이어 폰 한정
	UFUNCTION()
	void			OnInfoRangeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void			OnInfoRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// DT_Item의 Item ID 값
	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_ProductItemID, SaveGame, Category = "Production")
	FName							ProductItemID = { NAME_None };

	// 아이템 1개당 생산 필요 작업량
	UPROPERTY(EditdefaultsOnly, SaveGame, Category = "Production", meta = (ClampMin = "0.01"))
	float							RequiredWorkPerItem = { 100.f };

	UPROPERTY(EditDefaultsOnly, Replicated, SaveGame, Category = "Production", meta = (ClampMin = "1", ClampMax = "9999"))
	int32							MaxStoredCount = { 9999 };

	UPROPERTY(EditDefaultsOnly, Category = "Production|Interact")
	TArray<FDC1InteractOption>		CollectOptions;

	UPROPERTY(EditDefaultsOnly, Category = "Production|UI")
	TObjectPtr<USphereComponent>	InfoRangeSphere = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Production|UI")
	TSubclassOf<UDC1BuildInfoWidget>	ProductionInfoWidgetClass = { nullptr };

	// 생산품이 늘어날 때 재생 ( 요리 / 분쇄 / 화로 등 시설별 지정 )
	UPROPERTY(EditDefaultsOnly, Category = "Production|Sound")
	TObjectPtr<USoundBase>			ProduceSound = { nullptr };
	// 일할 수 있는 최대 팰 수
	UPROPERTY(EditDefaultsOnly, Category = "Production|Pal", meta = (ClampMin = "0"))
	int32							MaxPalWorkers = { 1 };

	// 팰만 작업 가능 옵션
	UPROPERTY(EditDefaultsOnly, Category = "Production|Pal")
	bool							IsPalOnlyWork = { false };
	// 생산 진행률
	UPROPERTY(ReplicatedUsing = OnRep_ProductionProgress)
	float							ProductionProgress = { 0.f };
	// 보유 중 개수
	UPROPERTY(ReplicatedUsing = OnRep_StoredCount, SaveGame)
	int32							StoredCount = { 0 };

	// 누적 작업량 - 서버
	UPROPERTY(SaveGame)
	float							CurrentProductionWork = { 0.f };
	// 1회 제작 시 실제 생산 개수
	UPROPERTY(SaveGame)
	int32							ProductCountPerWork = { 1 };

private:
	bool							IsLocalInRange = { false };
};
