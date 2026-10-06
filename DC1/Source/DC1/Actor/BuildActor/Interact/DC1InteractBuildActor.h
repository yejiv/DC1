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
	FDC1OnInteractOptionsChanged	OnInteractOptionsChanged;

public:
	ADC1InteractBuildActor();

public:
	const bool						IsUnderConstruction() const { return EDC1BuildState::Construction == BuildState; }

	virtual void					Tick(float DeltaTime) override;

protected:
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// IDC1InteractableInterface override
	virtual UPrimitiveComponent*	GetInteractCollider() const override;
	virtual void					GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual FDC1OnInteractOptionsChanged& GetOnInteractOptionsChanged() override { return OnInteractOptionsChanged; }
	virtual void					Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override final;

public:
	virtual void					InitBuild(const FDC1BuildActorData& InBuildActorData) override;
	virtual void					BuildDestroy() override;

	void							AddWorker(const ADC1Character* InWorker);
	void							RemoveWorker(const ADC1Character* InWorker);

protected:
	virtual void					ApplyState() override;

	// Completed 상태에서의 상호작용
	virtual void					OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) {}
	

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

	// 건설 중 옵션 F 건설(HoldAbility), C 취소(HoldConfirm)
	UPROPERTY(EditDefaultsOnly, Category = "Build|Interact")
	TArray<FDC1InteractOption>				ConstructionOptions;

	// Completed 후 옵션 - ex) F : 상자 열기, V : 빠른 이동, C : 제작 취소
	UPROPERTY(EditDefaultsOnly, Category = "Build|Interact")
	TArray<FDC1InteractOption>				CompletedOptions;

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
