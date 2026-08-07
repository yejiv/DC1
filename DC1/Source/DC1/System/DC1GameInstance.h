// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DC1GameInstance.generated.h"

/**
 * 
 */
UCLASS()
class DC1_API UDC1GameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	UDC1GameInstance(const FObjectInitializer& ObjectInitializer);

public:
	virtual void Init() override;
	virtual void Shutdown() override;

private:
	void	OnPreloadComplete();

};
