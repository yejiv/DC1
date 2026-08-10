// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DC1Character.h"

#include "Component/DC1AbilitySystemComponent.h"

ADC1Character::ADC1Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

UAbilitySystemComponent* ADC1Character::GetAbilitySystemComponent() const
{
	return nullptr;
}

// Called when the game starts or when spawned
void ADC1Character::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ADC1Character::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ADC1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}
