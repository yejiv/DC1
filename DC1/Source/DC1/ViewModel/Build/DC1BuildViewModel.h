// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Component/DC1TechTreeComponent.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1BuildViewModel.generated.h"

class UDC1BuildPlacementComponent;

UCLASS()
class DC1_API UDC1BuildViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	// Widget이 구독
	FDC1OnTechStateChange	OnBuildStateChanged;
	FDC1OnTechUnlocked		OnBuildUnlocked;

public:
	virtual void				BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void				ClearSource() override;

public:
	// 표시에 필요한 건 TechComp 뿐이다. PlacementComp는 액션 시점에 따로 확인한다
	bool						HasModel() const { return ::IsValid(TechComp); }

	const TArray<FDC1BuildRow>& GetBuildRows() const { return SortedBuildRows; }
	const FDC1TechRow*			FindTechRow(const FGameplayTag& InBuildTag) const;
	bool						IsUnlocked(const FGameplayTag& InTechTag) const;

	bool						RequestStartPlacement(const FGameplayTag& InBuildTag);

private:
	void						SetModels(UDC1TechTreeComponent* InTechComp, UDC1BuildPlacementComponent* InPlacementComp);

	void						CacheBuildRows();
	void						HandleTechStateChanged();
	void						HandleTechUnlocked(const FGameplayTag& InTechTag);

private:
	UPROPERTY()
	TObjectPtr<UDC1TechTreeComponent>		TechComp = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1BuildPlacementComponent>	PlacementComp = { nullptr };

	TArray<FDC1BuildRow>					SortedBuildRows;
};
