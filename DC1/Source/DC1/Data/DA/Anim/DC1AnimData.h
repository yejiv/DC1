// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DC1AnimData.generated.h"

class UAnimInstance;
class UBlendSpace;
class UAnimMontage;

UCLASS()
class DC1_API UDC1AnimData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	TSubclassOf<UAnimInstance>	FindAnimInstanceByTag(const FGameplayTag& AnimTag) const;
	UBlendSpace*				FindBlendSpaceByTag(const FGameplayTag& AnimTag) const;
	UAnimMontage*				FindMontageByTag(const FGameplayTag& AnimTag) const;

	TArray<FGameplayTag>		GetAnimInstanceKeys() const;

public:
	// ABP 는 클래스(BlueprintGeneratedClass), BS / AM 은 에셋 인스턴스라 타입이 다르다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AnimInstance")
	TMap<FGameplayTag, TSubclassOf<UAnimInstance>> AnimInstances;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BlendSpace")
	TMap<FGameplayTag, TObjectPtr<UBlendSpace>> BlendSpaces;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimMontage")
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Montages;

};
