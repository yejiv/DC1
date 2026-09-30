// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/BuildActor/Destroy/DC1BuildDestroyActor.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"

ADC1BuildDestroyActor::ADC1BuildDestroyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;

	DestroyComp = CreateDefaultSubobject<UGeometryCollectionComponent>("DestroyComp");

	RootComponent = DestroyComp;
}

void ADC1BuildDestroyActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(DestroyLifeTime);

	if (false == ::IsValid(DestroyComp))
	{
		return;
	}

	DestroyComp->CrumbleActiveClusters();
}

void ADC1BuildDestroyActor::InitDestroy(const UGeometryCollection* InCollection)
{
	if (false == ::IsValid(DestroyComp) || false == ::IsValid(InCollection))
	{
		return;
	}

	DestroyComp->SetRestCollection(InCollection);
}
