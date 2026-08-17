// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "DC1AbilitySystemComponent.generated.h"

USTRUCT()
struct FDC1AbilitySetGrantedHandles 
{
	GENERATED_BODY()

public:
	void AddAbilitySpecHandle(const FGameplayAbilitySpecHandle& Handle);
	void TakeFromAbilitySystem(UDC1AbilitySystemComponent* ASC);

public:
	TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;
	TArray<FActiveGameplayEffectHandle> EffectHandles;
};

UCLASS()
class DC1_API UDC1AbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
	// ----------- 초기화 & 해제 ------------
public:
	void InitAbilitySystem(AActor* Owner, AActor* Avatar, const FGameplayTag& DataTag);
	void UnInitializeAbilitySystem(); // Pool 반납, 해제
	void RefreshAvatarActor(AActor* NewAvatarActor); // Respawn
	void ClearAvatarActor();	// Unpossessed
	// ----------- Ability 활성화 -------------
public:
	bool ActivateAbilityByTag(const FGameplayTag& AbilityTag);
	void CancelAbilityByTag(const FGameplayTag& AbilityTag);

	bool HasAbilityByTag(const FGameplayTag& AbilityTag);
	bool HasAbility(const TSubclassOf<UGameplayAbility>& AbilityClass, const FGameplayTag& ActivationTag) const;

	// ------------- State Tag ----------------
public:
	void SetExclusiveStateTag(const FGameplayTag& AxisRootTag, const FGameplayTag& NewStateTag);
	FGameplayTag GetExclusiveStateTag(const FGameplayTag& AxisRootTag);
private:
	void AddStateTag(const FGameplayTag& StateTag);
	void RemoveAxisStateTag(const FGameplayTag& AxisRootTag);

	// ---------- Engine Override ------------
public:
	virtual void OnGiveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec) override;
	virtual void OnRep_ActivateAbilities() override;

	// --------------- Inner -------------------
private:
	void RebuildAbilityTagCache();

private:
	// 활성용 Handle
	TMultiMap<FGameplayTag, FGameplayAbilitySpecHandle> AbilityTagCache;

	// 정리용 Handle
	UPROPERTY()
	FDC1AbilitySetGrantedHandles GrantedHandles;

	FGameplayTag DataAssetTag;
};
