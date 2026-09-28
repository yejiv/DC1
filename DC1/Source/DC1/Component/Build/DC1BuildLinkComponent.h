// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Define/DC1Structs.h"
#include "DC1BuildLinkComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1BuildLinkComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDC1BuildLinkComponent();

public:
	const TArray<FDC1BuildStructureLink>& GetLinks() const { return Links; }

protected:
	// Called when the game starts
	virtual void	BeginPlay() override;
	virtual void	EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void			RefreshPreviewLinks();

private:
	void			LinkStructure();
	void			UnlinkStructure();

private:
	TArray<FDC1BuildStructureLink> Links;
};
