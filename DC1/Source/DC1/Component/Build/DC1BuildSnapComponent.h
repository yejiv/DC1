// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DC1BuildSnapComponent.generated.h"

struct FDC1BuildSnapSocket;

UCLASS()
class DC1_API UDC1BuildSnapComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildSnapComponent();

protected:
	virtual void	BeginPlay() override;

public:	
	virtual void	TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void			LayoutSockets();
	bool			GetBuildSocketsToWorld(TArray<FDC1BuildSnapSocket>& OutSockets) const;

private:
	void			DrawDebugSockets();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Debug")
	bool			DrawDebugSocket = { false };
};
