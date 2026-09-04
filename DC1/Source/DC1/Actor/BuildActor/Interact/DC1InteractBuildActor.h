// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/DC1BuildActor.h"
#include "Interface/DC1InteractableInterface.h"
#include "DC1InteractBuildActor.generated.h"

class ADC1Character;

DECLARE_MULTICAST_DELEGATE(FDC1OnBuildCompleted);

UCLASS()
class DC1_API ADC1InteractBuildActor : public ADC1BuildActor, public IDC1InteractableInterface
{
	GENERATED_BODY()

public:
	FDC1OnBuildCompleted			OnBuildCompleted;

public:
	ADC1InteractBuildActor();

public:
	const bool						IsUnderConstruction() const { return EDC1BuildState::Construction == GetBuildState(); }

	virtual void					Tick(float DeltaTime) override;

protected:
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual UPrimitiveComponent*	GetInteractCollider() const override;
	virtual FGameplayTag			GetInteractAbilityTag() const override;

	virtual void					OnInteractableFocused() override;
	virtual void					OnInteractableUnfocused() override;


public:
	virtual void					InitBuild(const FDC1BuildActorData& InBuildActorData) override;
	virtual void					BuildDestroy() override;

	void							AddWorker(const ADC1Character* InWorker);
	void							RemoveWorker(const ADC1Character* InWorker);

protected:
	virtual void					ApplyState() override;

	UFUNCTION()
	void							OnRep_BuildProgress();

private:
	void							BuildCompleted();

	void							CancelWorkersAbility();
	float							ComputeTotalWorkSpeed();

	void							SetMaterialBuildProgress();
	void							SetMaterialFlash();

protected:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UBoxComponent>				InteractCollider = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Interact")
	FGameplayTag							InteractAbilityTag = { FGameplayTag::EmptyTag };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Interact")
	FGameplayTag							OnFocusedUITag = { FGameplayTag::EmptyTag };

private:
	// Completed 시점 Flash 지속시간
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	float									CompleteFlashDuration = { 0.4f };

	// 머테리얼 슬롯별 진행도 구간 ( ex) Body = 0 ~ 0.5, Head = 0.5 ~ 1 )
	UPROPERTY(EditDefaultsOnly, Category = "Build|Material")
	TArray<FVector2D>						MaterialProgressRanges;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Material|TranslucentColor")
	FLinearColor							ConstructionColor = { FLinearColor::Blue };

	UPROPERTY(ReplicatedUsing = OnRep_BuildProgress)
	float									BuildProgress = { 0.f };

private:
	TSet<TWeakObjectPtr<const ADC1Character>>	Workers;
	float									CurrentWork = { 0.f };
	float									FlashAmount = { 0.f };
};
