// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "AI/DC1TeamTypes.h"
#include "DC1PalCharacter.generated.h"

class UDC1AbilitySystemComponent;
class UDC1PalAttributeSet;
class UDC1PalData;

/**
 *
 */
UCLASS()
class DC1_API ADC1PalCharacter : public ADC1Character, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	ADC1PalCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	EDC1Team							GetTeam() const { return Team; }
	void								SetTeam(EDC1Team NewTeam);

	const UDC1PalData*					GetPalData() const { return PalData; }

	void								SetOrientToMovement(bool Enable);
	virtual void						GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const override;

	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;

protected:
	virtual void						PostInitializeComponents() override;

	virtual void						OnTeamChanged();

	UFUNCTION()
	void								OnRep_Team();

	void								ApplyPalData();

	void								HandleDamaged(const struct FOnAttributeChangeData& Data);

	// 래그돌, AI 정지, 퍼셉션 해제
	virtual void OnDeath() override;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1PalAttributeSet> PalAttributeSet = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal", meta = (Categories = "Pal.Species"))
	FGameplayTag SpeciesTag;

	UPROPERTY(Transient) //디스크에 저장X
	TObjectPtr<const UDC1PalData> PalData = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Team, Category = "Pal|Team")
	EDC1Team Team = { EDC1Team::Wild };
};
