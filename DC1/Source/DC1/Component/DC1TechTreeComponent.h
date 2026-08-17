// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/DC1Enums.h"
#include "DC1TechTreeComponent.generated.h"

struct FDC1TechRow;
class UDataTable;

// FName TechID
DECLARE_MULTICAST_DELEGATE(FDC1OnTechStateChange);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1TechTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FDC1OnTechStateChange OnTechStateChange;

public:	
	UDC1TechTreeComponent();

public:
	bool					IsTechUnlocked(const FName& TechID) const { return UnlockedTechIDs.Contains(TechID); }
	int32					GetTechPoints() const { return TechPoints; }
	const TArray<FName>&	GetUnlockedTechID() const { return UnlockedTechIDs; }
	EDC1TechSlotState		GetTechState(const FName& TechID) const;
public:
	virtual void			BeginPlay() override;
	virtual void			GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
	UFUNCTION(Server, Reliable)
	void			Server_UnlockTech(const FName& TechID);
	
	void			AddTechPoints(int32 Points);


private:
	UFUNCTION()
	void			OnRep_TechState();

	void			CachedTechRows();

private:
	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	TArray<FName>					UnlockedTechIDs;

	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	int32							TechPoints = 10;

	UPROPERTY()
	TObjectPtr<const UDataTable>	CachedTechData;

	TMap<FName, const FDC1TechRow*>	TechRowsByID;
};
