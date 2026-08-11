// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "GameplayTagContainer.h"
#include "DC1AnimNotify_PalAttackHit.generated.h"

UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "DC1 Pal Attack Hit"))
class DC1_API UDC1AnimNotify_PalAttackHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UDC1AnimNotify_PalAttackHit();

	virtual void	Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "DC1", meta = (Categories = "Event"))
	FGameplayTag	EventTag;
};
