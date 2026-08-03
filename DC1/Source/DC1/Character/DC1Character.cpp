// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DC1Character.h"

#include "Component/DC1AbilitySystemComponent.h"

// Sets default values
ADC1Character::ADC1Character()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Add Component
	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>("AbilitySystemComponent");
}

// Called when the game starts or when spawned
void ADC1Character::BeginPlay()
{
	Super::BeginPlay();
}

UAbilitySystemComponent* ADC1Character::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
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

void ADC1Character::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	Super::AddMovementInput(WorldDirection, ScaleValue * SpeedRate, bForce);
}

