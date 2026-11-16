// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Equip/Glider/DC1Glider.h"

#include "Net/UnrealNetwork.h"

ADC1Glider::ADC1Glider()
	: Super()
{
}

void ADC1Glider::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADC1Glider, GlideSpeed);
}
