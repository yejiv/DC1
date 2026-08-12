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

	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	void					UpdateTarget();

	void					ApplyPerceptionSettings();

	AActor*					GetCurrentTarget() const { return ::IsValid(CurrentTarget) ? CurrentTarget.Get() : nullptr; }
	const FVector&			GetLastKnownTargetLocation() const { return LastKnownTargetLocation; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	TObjectPtr<AActor>		CurrentTarget = { nullptr };

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	
	void SetCurrentTarget(AActor* NewTarget);

protected:
	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UStateTreeAIComponent> StateTreeComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig = { nullptr };

	// 현재 감지 중인 적들. 죽으면 UpdateTarget에서 걸러짐.
	TArray<TWeakObjectPtr<AActor>>	SensedTargets;

	FVector							LastKnownTargetLocation = { FVector::ZeroVector };
	float							LastSeenTime = { 0.f };

	// 놓친 뒤 추격 유지하는 시간
	float							TargetMemoryDuration = { 3.f };
};
