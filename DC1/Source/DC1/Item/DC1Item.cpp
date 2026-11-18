// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/DC1Item.h"

#include "Components/StaticMeshComponent.h"

ADC1Item::ADC1Item()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>("ItemMesh");
	SetRootComponent(ItemMesh);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}