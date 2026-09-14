// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DC1InteractComponent.generated.h"

class IDC1InteractableInterface;
class USphereComponent;
class APlayerController;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1InteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDC1InteractComponent();

public:
	AActor*						GetCurrentTarget() const { return CurrentTarget.Get(); }
	IDC1InteractableInterface*	GetCurrentInteractable() const;
	void						SetInteractEnabled(bool InEnabled);
	void						SetTargetLocked(bool InLocked);

protected:
	virtual void				BeginPlay() override;

public:
	virtual void				TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void						RequestStartWork(ADC1InteractBuildActor* InBuildTarget);
	void						RequestStopWork();
	

private:
	UFUNCTION(Server, Reliable)
	void						Server_StartWork(ADC1InteractBuildActor* InBuildTarget);
	
	UFUNCTION(Server, Reliable)
	void						Server_StopWork();
	
	AActor*						FindBestTarget() const;
	void						SetCurrentTarget(AActor* InNewTarget);

private:

	// 경계에서 떨림 방지용 Bonus
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float	CurrentTargetDotBonus = { 0.05f };

private:
	TWeakObjectPtr<AActor>			CurrentTarget = { nullptr };

	//mutable	TArray<AActor*>			OverlapBuffer;

	bool							IsTargetLocked = { false };

	TWeakObjectPtr<ADC1InteractBuildActor>	ServerWorkTarget = { nullptr };
};
