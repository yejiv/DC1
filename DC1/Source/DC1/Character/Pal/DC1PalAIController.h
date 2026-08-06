// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "DC1PalAIController.generated.h"

class ADC1Character;
class UStateTreeAIComponent;
class UAISenseConfig_Sight;

/**
 *
 */
UCLASS()
class DC1_API ADC1PalAIController : public AAIController
{
	GENERATED_BODY()

public:
	ADC1PalAIController();

	// AIPerception은 리스너의 팀 에이전트로 컨트롤러를 먼저 잡는다.
	// 진영 원본은 Pawn 하나로 유지하기 위해 넘긴다.
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor> CurrentTarget = { nullptr };

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

protected:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig = { nullptr };
};
