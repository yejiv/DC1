// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1PlayerState.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1EquipComponent.h"
#include "Component/DC1TechTreeComponent.h"
#include "Component/Appearance/DC1AppearanceComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/Inventory/Valuable/DC1ValuableInventoryComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Component/PalSphere/DC1PalSphereComponent.h"
#include "Component/Teleport/DC1TeleportComponent.h"
#include "Component/Player/DC1PlayerStatComponent.h"
#include "Component/Notification/DC1NotificationComponent.h"

#include "Save/DC1PlayerSaveGame.h"
#include "Tags/State/DC1GameplayTag_State.h"

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

	ValuableInventoryComponent = CreateDefaultSubobject<UDC1ValuableInventoryComponent>(TEXT("ValuableInventoryComponent"));
	ValuableInventoryComponent->SetIsReplicated(true);

	PalPartyComponent = CreateDefaultSubobject<UDC1PalPartyComponent>(TEXT("PalPartyComponent"));
	PalPartyComponent->SetIsReplicated(true);

	PalSphereComponent = CreateDefaultSubobject<UDC1PalSphereComponent>(TEXT("PalSphereComponent"));

	EquipComponent = CreateDefaultSubobject<UDC1EquipComponent>(TEXT("EquipComponent"));
	EquipComponent->SetIsReplicated(true);

	AppearanceComponent = CreateDefaultSubobject<UDC1AppearanceComponent>(TEXT("AppearanceComponent"));

	TeleportComponent = CreateDefaultSubobject<UDC1TeleportComponent>(TEXT("TeleportComponent"));

	StatComponent = CreateDefaultSubobject<UDC1PlayerStatComponent>(TEXT("StatComponent"));

	NotificationComponent = CreateDefaultSubobject<UDC1NotificationComponent>(TEXT("NotificationComponent"));
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
	if (false == IsValid(In))
		return;

	if (false == IsValid(PalPartyComponent))
		return;

	PalPartyComponent->ImportSaveData(In->PartyPals);
}

void ADC1PlayerState::ResetForRespawn(float InHealthRatio)
{
	if (false == HasAuthority() || nullptr == AbilitySystemComponent)
		return;

	// Die()에서 TagOnly로 붙였으니 같은 옵션으로 제거
	AbilitySystemComponent->SetLooseGameplayTagCount(DC1StateTags::State_Dead, 0, EGameplayTagReplicationState::TagOnly);

	const float MaxHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
	AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), MaxHealth * InHealthRatio);

	const float MaxStamina = AbilitySystemComponent->GetNumericAttribute(UDC1CharacterAttributeSet::GetMaxStaminaAttribute());
	AbilitySystemComponent->SetNumericAttributeBase(UDC1CharacterAttributeSet::GetStaminaAttribute(), MaxStamina);
}
