// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/DC1Character.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Tags/State/DC1GameplayTag_State.h"

ADC1Character::ADC1Character(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADC1Character::BindHealthWatcher()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (nullptr == ASC)
	{
		return;
	}

	FOnGameplayAttributeValueChange& Delegate =
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute());

	// ASC가 바뀌어 다시 불려도 중복 등록되지 않게
	Delegate.RemoveAll(this);
	Delegate.AddUObject(this, &ThisClass::HandleHealthChanged);
}

void ADC1Character::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.f)
	{
		return;
	}

	if (true == bDead)
	{
		return;
	}

	bDead = true;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Dead, 1);
	}

	OnDeath();
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
