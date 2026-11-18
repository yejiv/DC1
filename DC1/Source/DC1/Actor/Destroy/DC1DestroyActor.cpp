// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Destroy/DC1DestroyActor.h"

#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"

ADC1DestroyActor::ADC1DestroyActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = false;

	DestroyComp = CreateDefaultSubobject<UGeometryCollectionComponent>("DestroyComp");

	RootComponent = DestroyComp;
}

void ADC1DestroyActor::BeginPlay()
{
	Super::BeginPlay();

	SetLifeSpan(DestroyLifeTime);

	if (false == ::IsValid(DestroyComp))
	{
		return;
	}

	DestroyComp->CrumbleActiveClusters();
}

void ADC1DestroyActor::InitDestroy(const UGeometryCollection* InCollection)
{
	if (false == ::IsValid(DestroyComp) || false == ::IsValid(InCollection))
	{
		return;
	}

	DestroyComp->SetRestCollection(InCollection);
}
