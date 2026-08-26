// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DC1InteractComponent.generated.h"


class ADC1BuildActor;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1InteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDC1InteractComponent();

public:
	const ADC1BuildActor*	GetCurrentTarget() const { return CurrentTarget.Get(); }
	void					SetInteractEnabled(bool InEnabled);

protected:
	virtual void			BeginPlay() override;

public:	
	virtual void			TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	const APawn*			GetOwnerPawn() const;
	const ADC1BuildActor*	TraceTarget() const;
	void					SetCurrentTarget(const ADC1BuildActor* InNewTarget);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float	MaxTraceDistance = { 500.f };

	TWeakObjectPtr<const ADC1BuildActor>	CurrentTarget;
};
