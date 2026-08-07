// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "AI/DC1TeamTypes.h"
#include "DC1PalCharacter.generated.h"

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
	ADC1PalCharacter();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// IGenericTeamAgentInterface
	virtual ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	EDC1Team GetTeam() const { return Team; }

	// 클라이언트에서 호출하면 무시
	void SetTeam(EDC1Team NewTeam);

	const UDC1PalData* GetPalData() const { return PalData; }

protected:
	virtual void PostInitializeComponents() override;

	virtual void OnTeamChanged();

	UFUNCTION()
	void OnRep_Team();

	void ApplyPalData();

protected:
	UPROPERTY()
	TObjectPtr<UDC1PalAttributeSet> PalAttributeSet = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pal", meta = (Categories = "Pal.Species"))
	FGameplayTag SpeciesTag;

	UPROPERTY(Transient)
	TObjectPtr<const UDC1PalData> PalData = { nullptr };

	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing = OnRep_Team, Category = "Pal|Team")
	EDC1Team Team = { EDC1Team::Wild };
};
