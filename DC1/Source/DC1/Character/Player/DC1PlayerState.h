// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DC1PlayerState.generated.h"

class UDC1AbilitySystemComponent;
class UDC1PlayerSet;
class UDC1TechTreeComponent;
class UDC1PlayerInventoryComponent;
class UDC1PalPartyComponent;

/**
 * 
 */
UCLASS()
class DC1_API ADC1PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ADC1PlayerState(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;
	UDC1TechTreeComponent*				GetTechTreeComponent() const { return TechTreeComponent; }
	UDC1PlayerInventoryComponent*		GetInventoryComponent() const { return InventoryComponent; }
	UDC1PalPartyComponent*				GetPalPartyComponent() const { return PalPartyComponent; }

private:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1PlayerSet> PlayerSet = { nullptr };
	
	UPROPERTY(VisibleAnywhere, Category = "Tech")
	TObjectPtr< UDC1TechTreeComponent>		TechTreeComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UDC1PlayerInventoryComponent> InventoryComponent = { nullptr };

	// 잡은 팰 목록. 아이템 슬롯과 구조가 달라 따로 둔다
	UPROPERTY(VisibleAnywhere, Category = "Pal")
	TObjectPtr<UDC1PalPartyComponent> PalPartyComponent = { nullptr };
};
