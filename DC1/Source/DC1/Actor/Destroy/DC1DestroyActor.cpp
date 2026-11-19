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

	// 분리된 조각들이 같은 속도로 대형을 유지한 채 떨어지지 않도록 바깥으로 밀어준다
	if (BreakImpulseStrength > 0.f)
	{
		DestroyComp->AddRadialImpulse(DestroyComp->Bounds.Origin, DestroyComp->Bounds.SphereRadius, BreakImpulseStrength, ERadialImpulseFalloff::RIF_Constant, true);
	}
}

void ADC1DestroyActor::InitDestroy(const UGeometryCollection* InCollection)
{
	if (false == ::IsValid(DestroyComp) || false == ::IsValid(InCollection))
	{
		return;
	}

	DestroyComp->SetRestCollection(InCollection);
}
