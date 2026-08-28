// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Equip/Tool/DC1Tool.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

ADC1Tool::ADC1Tool()
	: Super()
{
	ToolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ToolMesh");
	ToolMesh->SetupAttachment(InteractCollider);
	ToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}