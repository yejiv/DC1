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

	void					StopBehavior();

	// 때린 상대를 기억한다. 보이지 않아도 기억 시간 동안 쫓아간다
	void					ProvokeBy(AActor* InInstigator);
	bool					IsProvoked() const { return ::IsValid(ProvokedTarget); }

	// 배회 기준점. 빙의 시점의 위치
	const FVector&			GetHomeLocation() const { return HomeLocation; }
	void					SetHomeLocation(const FVector& InLocation) { HomeLocation = InLocation; }

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

	UPROPERTY()
	TObjectPtr<AActor>				ProvokedTarget = { nullptr };

	FVector							HomeLocation = { FVector::ZeroVector };
	float							LastSeenTime = { 0.f };

	// 놓친 뒤 추격 유지하는 시간
	float							TargetMemoryDuration = { 3.f };
};
