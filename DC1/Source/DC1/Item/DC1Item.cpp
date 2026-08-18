// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/DC1Item.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"

ADC1Item::ADC1Item()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	InteractCollider = CreateDefaultSubobject<USphereComponent>("InteractCollider");
	SetRootComponent(InteractCollider);
	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	ItemMesh->SetupAttachment(InteractCollider);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}