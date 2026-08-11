// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/Anim/DC1AnimData.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"

TSubclassOf<UAnimInstance> UDC1AnimData::FindAnimInstanceByTag(const FGameplayTag& AnimTag) const
{
	if (const TSubclassOf<UAnimInstance>* Found = AnimInstances.Find(AnimTag))
	{
		return *Found;
	}
	return nullptr;
}

UBlendSpace* UDC1AnimData::FindBlendSpaceByTag(const FGameplayTag& AnimTag) const
{
	if (const TObjectPtr<UBlendSpace>* Found = BlendSpaces.Find(AnimTag))
	{
		return *Found;
	}
	return nullptr;
}

UAnimMontage* UDC1AnimData::FindMontageByTag(const FGameplayTag& AnimTag) const
{
	if (const TObjectPtr<UAnimMontage>* Found = Montages.Find(AnimTag))
	{
		return *Found;
	}
	return nullptr;
}

TArray<FGameplayTag> UDC1AnimData::GetAnimInstanceKeys() const
{
	TArray<FGameplayTag> Tags;
	AnimInstances.GenerateKeyArray(Tags);
	return Tags;
}
