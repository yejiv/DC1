// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/Notify/DC1AnimNotify_PalWorkHit.h"

#include "Actor/Resource/DC1ResourceActor.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Animation/AnimNotifies/AnimNotify_PlaySound.h"
#include "AnimNotify_PlayNiagaraEffect.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"

UDC1AnimNotify_PalWorkHit::UDC1AnimNotify_PalWorkHit()
{
#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(230, 160, 40);
#endif
}

void UDC1AnimNotify_PalWorkHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	const UWorld* World = (true == ::IsValid(MeshComp)) ? MeshComp->GetWorld() : nullptr;
	if (nullptr != World && EWorldType::EditorPreview == World->WorldType)
	{
		PlayFeedbackNotifies(MeshComp, Animation, EventReference);
		return;
	}

	SendWorkHit(MeshComp, Multiplier);
	PlayWorkFeedback(MeshComp, Animation, EventReference);
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

void UDC1AnimNotify_PalWorkHit::PlayWorkFeedback(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, const FAnimNotifyEventReference& InEventReference) const
{
	if (false == ::IsValid(InMeshComp) || NM_DedicatedServer == InMeshComp->GetNetMode())
	{
		return;
	}

	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(InMeshComp->GetOwner());
	if (false == ::IsValid(Pal))
	{
		return;
	}

	const AActor* TargetActor = Cast<AActor>(Pal->GetCurrentWorkTarget());
	if (false == ::IsValid(TargetActor) || nullptr != Cast<ADC1ResourceActor>(TargetActor))
	{
		return;
	}

	PlayFeedbackNotifies(InMeshComp, InAnimation, InEventReference);
}

void UDC1AnimNotify_PalWorkHit::PlayFeedbackNotifies(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, const FAnimNotifyEventReference& InEventReference) const
{
	if (true == ::IsValid(SoundNotify))
	{
		SoundNotify->Notify(InMeshComp, InAnimation, InEventReference);
	}

	if (true == ::IsValid(FXNotify))
	{
		FXNotify->Notify(InMeshComp, InAnimation, InEventReference);
	}
}

FString UDC1AnimNotify_PalWorkHit::GetNotifyName_Implementation() const
{
	return TEXT("Pal Work Hit");
}
