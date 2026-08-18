// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/DC1Enums.h"
#include "GameplayTagContainer.h"
#include "DC1TechTreeComponent.generated.h"

struct FDC1TechRow;
struct FOnAttributeChangeData;

class UDataTable;

// Tech Point, Level, Tech Slot State 변경
DECLARE_MULTICAST_DELEGATE(FDC1OnTechStateChange);

// Tech Unlock ( Build, Crafting )
// FGameplayTag = Unlocked CategoryTag
DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnTechUnlocked, const FGameplayTag&);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1TechTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	FDC1OnTechStateChange	OnTechStateChange;
	FDC1OnTechUnlocked		OnTechUnlocked;

public:	
	UDC1TechTreeComponent();

public:
	bool					IsTechUnlocked(const FName& TechID) const { return UnlockedTechIDs.Contains(TechID); }
	int32					GetTechPoints() const { return TechPoints; }
	int32					GetAncientTechPoints() const { return AncientTechPoints; }
	const TArray<FName>&	GetUnlockedTechID() const { return UnlockedTechIDs; }
	int32					GetPlayerLevel() const;
	EDC1TechSlotState		GetTechState(const FName& TechID) const;
	bool					HasUnlockedTag(const FGameplayTag& Tag) const { return UnlockedTechTags.HasTag(Tag); }

public:
	virtual void			BeginPlay() override;
	virtual void			GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


public:
	UFUNCTION(Server, Reliable)
	void			Server_UnlockTech(const FName& TechID);
	
	UFUNCTION(Server, Reliable)
	void			Server_UnlockAncientTech(const FName& TechID);

	void			AddTechPoints(int32 Points);
	void			AddAncientTechPoints(int32 Points);


private:
	UFUNCTION()
	void			OnRep_TechState();

	UFUNCTION()
	void			OnRep_UnlockedTags();

	void			CachedTechRows();
	
	void			BindLevelChange();
	void			HandleLevelChanged(const FOnAttributeChangeData& Data);
	

private:
	// 해금된 Tech IDs ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	TArray<FName>					UnlockedTechIDs;

	// 기술 포인트 ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	int32							TechPoints = 10;

	// 고대 기술 포인트 ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	int32							AncientTechPoints = 0;

	// 해금된 Tech Tags ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedTags)
	FGameplayTagContainer			UnlockedTechTags;

	// 시작부터 해금될 Tech Tags
	UPROPERTY(EditDefaultsOnly, Category = "Tech")
	FGameplayTagContainer			DefaultUnlockedTags;

	// OnRep으로 인한 UnlockedTechTags와의 비교 위한 Container
	FGameplayTagContainer			LastUnlockedTags;

	// Tech Data Table Caching
	UPROPERTY()
	TObjectPtr<const UDataTable>	CachedTechData;

	// 특정 TechID가 가진 Tech Data Caching ( Point, Tech Cost 비교 )
	TMap<FName, const FDC1TechRow*>	TechRowsByID;
};
