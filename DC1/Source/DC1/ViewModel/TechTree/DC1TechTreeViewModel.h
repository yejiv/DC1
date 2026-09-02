// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Component/DC1TechTreeComponent.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1TechTreeViewModel.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1TechTreeViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()
	
public:
	// Widget이 구독
	FDC1OnTechStateChange	OnTechStateChanged;
	FDC1OnTechUnlocked		OnTechUnlocked;

public:
	virtual void									BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void									ClearSource() override;

public:
	bool											HasModel() const { return ::IsValid(TechComp); }

	// 조회
	int32											GetTechPoints() const;
	int32											GetAncientTechPoints() const;
	int32											GetPlayerLevel() const;
	EDC1TechSlotState								GetTechState(const FGameplayTag& InTechTag) const;

	const TMap<FGameplayTag, const FDC1TechRow>&	GetTechRowsByTag() const;

	// 액션
	void											RequestUnlockTech(const FGameplayTag& InTechTag);
	void											RequestUnlockAncientTech(const FGameplayTag& InTechTag);

private:
	void											SetTechTreeComponent(UDC1TechTreeComponent* InTechComp);

	void											HandleTechStateChanged();
	void											HandleTechUnlocked(const FGameplayTag& InTechTag);

private:
	UPROPERTY()
	TObjectPtr<UDC1TechTreeComponent>	TechComp = { nullptr };
};
