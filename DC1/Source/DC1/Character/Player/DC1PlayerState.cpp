// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1PlayerState.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1EquipComponent.h"
#include "Component/DC1TechTreeComponent.h"
#include "Component/Inventory/DC1PlayerInventoryComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"

#include "Save/DC1PlayerSaveGame.h"

ADC1PlayerState::ADC1PlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	// ASC 오너의 서브오브젝트여야 GAS가 자동 등록한다
	PlayerSet = CreateDefaultSubobject<UDC1PlayerSet>(TEXT("PlayerSet"));

	SetNetUpdateFrequency(100.f);

	TechTreeComponent = CreateDefaultSubobject<UDC1TechTreeComponent>(TEXT("TechTreeComponent"));

	InventoryComponent = CreateDefaultSubobject<UDC1PlayerInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);

	PalPartyComponent = CreateDefaultSubobject<UDC1PalPartyComponent>(TEXT("PalPartyComponent"));
	PalPartyComponent->SetIsReplicated(true);

	EquipComponent = CreateDefaultSubobject<UDC1EquipComponent>(TEXT("EquipComponent"));
	EquipComponent->SetIsReplicated(true);
}

UAbilitySystemComponent* ADC1PlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADC1PlayerState::WriteSaveData(UDC1PlayerSaveGame* Out) const
{
	if (false == IsValid(Out))
		return;

	if(true == IsValid(PalPartyComponent))
		PalPartyComponent->ExportSaveData(Out->PartyPals);
}

void ADC1PlayerState::ReadSaveData(const UDC1PlayerSaveGame* In)
{
}
