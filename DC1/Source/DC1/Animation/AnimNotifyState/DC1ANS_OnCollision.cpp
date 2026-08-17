// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotifyState/DC1ANS_OnCollision.h"

#include "Character/DC1Character.h"

void UDC1ANS_OnCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (nullptr == MeshComp) return;

	if (ADC1Character* Owner = Cast<ADC1Character>(MeshComp->GetOwner()))
	{
		Owner->SetAttackCollision(true);
	}
}

void UDC1ANS_OnCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (nullptr == MeshComp) return;

	if (ADC1Character* Owner = Cast<ADC1Character>(MeshComp->GetOwner()))
	{
		Owner->SetAttackCollision(false);
	}
}
