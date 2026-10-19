// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/Pal/DC1GA_Pal_Work.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

UDC1GA_Pal_Work::UDC1GA_Pal_Work(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDC1GA_Pal_Work::StartWork(ADC1InteractBuildActor* InWorkTarget)
{
	ADC1PalCharacter* Pal = GetPal();
	if (false == ::IsValid(Pal) || false == InWorkTarget->IsPalWorkerAssigned(Pal))
	{
		return false;
	}

	PalWorkersChangedHandle = InWorkTarget->OnPalWorkersChanged.AddUObject(this, &ThisClass::HandlePalWorkersChanged);
	InWorkTarget->AddWorker(Pal);

	return true;
}

void UDC1GA_Pal_Work::StopWork()
{
	ADC1InteractBuildActor* Target = WorkTarget.Get();
	if (true == ::IsValid(Target))
	{
		Target->OnPalWorkersChanged.Remove(PalWorkersChangedHandle);
		Target->RemoveWorker(GetPal());
	}

	PalWorkersChangedHandle.Reset();
}

void UDC1GA_Pal_Work::OnWorkStarted()
{
	UAnimMontage* Montage = FindWorkMontage();

	UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f);
	if (true == ::IsValid(PlayMontageTask))
	{
		PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
		PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
		PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);

		PlayMontageTask->ReadyForActivation();
	}
}

void UDC1GA_Pal_Work::HandlePalWorkersChanged()
{
	if (false == WorkTarget.IsValid() || false == WorkTarget->IsPalWorkerAssigned(GetPal()))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UDC1GA_Pal_Work::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDC1GA_Pal_Work::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

UAnimMontage* UDC1GA_Pal_Work::FindWorkMontage() const
{
	ADC1InteractBuildActor* Target = WorkTarget.Get();
	if (false == ::IsValid(Target))
	{
		return nullptr;
	}

	ADC1PalCharacter* Pal = GetPal();
	if (false == ::IsValid(Pal))
	{
		return nullptr;
	}

	const UDC1PalData* PalData = Pal->GetPalData();
	if (false == ::IsValid(PalData))
	{
		return nullptr;
	}

	FGameplayTag AnimTag = WorkToMontageTag.FindRef(Target->GetWorkTypeTag());
	if (false == AnimTag.IsValid())
	{
		return nullptr;
	}

	return PalData->FindMontageByTag(AnimTag).LoadSynchronous();
}

ADC1PalCharacter* UDC1GA_Pal_Work::GetPal() const
{
	return Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
}
