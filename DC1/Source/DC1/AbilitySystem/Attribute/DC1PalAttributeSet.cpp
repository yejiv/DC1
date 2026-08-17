// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

UDC1PalAttributeSet::UDC1PalAttributeSet()
{
	// PalData가 적용되기 전까지의 안전값. ADC1PalCharacter::ApplyPalData에서 덮어씀
	InitHealth(100.f);
	InitMaxHealth(100.f);
	InitAttackPower(10.f);
}
