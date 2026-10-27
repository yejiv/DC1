// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "UObject/WeakInterfacePtr.h"
#include "DC1PalBoxActor.generated.h"

/**
 * 
 */

UCLASS()
class DC1_API ADC1PalBoxActor : public ADC1InteractBuildActor
{
	GENERATED_BODY()

public:
	float							GetBaseRadius() const { return BaseRadius; }

	// 거점 범위 판단
	bool							IsInBase(const FVector& InLocation) const;

	// 거점 안 작업 대상 (서버 전용)
	const TArray<TWeakInterfacePtr<IDC1WorkTargetInterface>>& GetWorkTargets() const { return WorkTargets; }
	void							RegisterWorkTarget(IDC1WorkTargetInterface* InWorkTarget);

protected:
	virtual void					OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;

private:
	void							InteractF(ADC1Character* InInteractOwner);

	// 거점이 이미 있거나 다른 거점 범위와 겹치면 못 짓는다
	virtual bool					CanPlaceAt(const APlayerState* InBuilder) const override;
	virtual void					OnPlaced(const APlayerState* InBuilder) override;

	// 반경 안의 기존 노드를 등록
	void							CollectWorkTargets();

protected:
	// 거점 범위 반경
	UPROPERTY(EditDefaultsOnly, Category = "PalBox", meta = (ClampMin = "0.0"))
	float							BaseRadius = { 3000.f };

private:
	TArray<TWeakInterfacePtr<IDC1WorkTargetInterface>>	WorkTargets;
};
