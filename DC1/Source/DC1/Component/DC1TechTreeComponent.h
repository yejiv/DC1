// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enum/DC1Enums.h"
#include "GameplayTagContainer.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1TechTreeComponent.generated.h"

struct FOnAttributeChangeData;

class UDataTable;

// Tech Point, Level, Tech Slot State 변경
DECLARE_MULTICAST_DELEGATE(FDC1OnTechStateChange);

// Tech Unlock ( Build, Crafting )
// FGameplayTag = Unlocked TechTag
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
	bool					IsTechUnlocked(const FGameplayTag& InTechTag) const { return UnlockedTechTags.HasTagExact(InTechTag); }
	int32					GetTechPoints() const { return TechPoints; }
	int32					GetAncientTechPoints() const { return AncientTechPoints; }
	int32					GetPlayerLevel() const;
	EDC1TechSlotState		GetTechState(const FGameplayTag& InTechTag) const;
	bool					HasUnlockedTag(const FGameplayTag& Tag) const { return UnlockedTechTags.HasTag(Tag); }

	const TMap<FGameplayTag, const FDC1TechRow>& GetTechRowsByTag() const { return TechRowsByTag; }

public:
	virtual void			BeginPlay() override;
	virtual void			GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	void			RequestUnlockTech(const FGameplayTag& InTechTag);
	void			RequestUnlockAncientTech(const FGameplayTag& InTechTag);

private:
	UFUNCTION(Server, Reliable)
	void			Server_UnlockTech(const FGameplayTag& InTechTag);
	
	UFUNCTION(Server, Reliable)
	void			Server_UnlockAncientTech(const FGameplayTag& InTechTag);

	void			AddTechPoints(int32 InPoints);
	void			AddAncientTechPoints(int32 InPoints);


private:
	const FDC1TechRow* FindTechRow(const FGameplayTag& InTechTag) const;
	
	UFUNCTION()
	void			OnRep_TechState();

	UFUNCTION()
	void			OnRep_UnlockedTags();

	void			CachedTechRows();
	
	void			BindLevelChange();
	void			HandleLevelChanged(const FOnAttributeChangeData& InData);
	void			UnlockTech(const FGameplayTag& InTechTag);

private:
	// 기술 포인트 ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	int32							TechPoints = { 10 };

	// 고대 기술 포인트 ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_TechState)
	int32							AncientTechPoints = { 0 };

	// 해금된 Tech Tags ( OnRep )
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedTags)
	FGameplayTagContainer			UnlockedTechTags;

	// 시작부터 해금될 Tech Tags
	UPROPERTY(EditDefaultsOnly, Category = "Tech")
	FGameplayTagContainer			DefaultUnlockedTags;

private:
	// OnRep으로 인한 UnlockedTechTags와의 비교 위한 Container
	FGameplayTagContainer			LastUnlockedTags;

	// 특정 Tech Tag가 가진 Tech Data Caching ( Point, Tech Cost 비교 )
	TMap<FGameplayTag, const FDC1TechRow>	TechRowsByTag;
};
