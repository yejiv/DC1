// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notify/DC1AnimNotify_PalAttackHit.h"

#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UDC1AnimNotify_PalAttackHit::UDC1AnimNotify_PalAttackHit()
{
	EventTag = DC1PalTags::Event_Pal_Attack_Hit;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(220, 60, 60);
#endif
}

void UDC1AnimNotify_PalAttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (false == ::IsValid(MeshComp))
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (false == ::IsValid(Owner))
	{
		return;
	}

	if (false == EventTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] AttackHit notify on [%s] has no EventTag."), *GetNameSafe(Animation));
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, EventTag, FGameplayEventData());
}

FString UDC1AnimNotify_PalAttackHit::GetNotifyName_Implementation() const
{
	return TEXT("Pal Attack Hit");
}
