// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notify/DC1AnimNotify_PalWorkHit.h"

#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UDC1AnimNotify_PalWorkHit::UDC1AnimNotify_PalWorkHit()
{
	// 몽타주 에디터 프리뷰 액터는 ASC가 없음
	bShouldFireInEditor = false;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(230, 160, 40);
#endif
}

void UDC1AnimNotify_PalWorkHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	SendWorkHit(MeshComp, Multiplier);
}

void UDC1AnimNotify_PalWorkHit::SendWorkHit(USkeletalMeshComponent* InMeshComp, float InMultiplier)
{
	if (false == ::IsValid(InMeshComp))
	{
		return;
	}

	AActor* Owner = InMeshComp->GetOwner();
	if (false == ::IsValid(Owner))
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventMagnitude = InMultiplier;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, DC1PalTags::Event_Pal_Work_Hit, Payload);
}

FString UDC1AnimNotify_PalWorkHit::GetNotifyName_Implementation() const
{
	return TEXT("Pal Work Hit");
}
