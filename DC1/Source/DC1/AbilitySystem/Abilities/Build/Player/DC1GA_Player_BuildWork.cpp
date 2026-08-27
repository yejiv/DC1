// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Build/Player/DC1GA_Player_BuildWork.h"

#include "Tags/State/DC1GameplayTag_State.h"
#include "Character/Player/DC1PlayerController.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"
#include "Tags/DC1GamePlayTags.h"

UDC1GA_Player_BuildWork::UDC1GA_Player_BuildWork(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dead);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Build_Placement);
}

void UDC1GA_Player_BuildWork::OnInputReleased(float TimeHeld)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

ADC1BuildActor* UDC1GA_Player_BuildWork::FindBuildTarget() const
{
	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (nullptr == InteractComp)
	{
		return nullptr;
	}

	AActor* BuildTraget = InteractComp->GetCurrentTarget();
	if (false == ::IsValid(BuildTraget))
	{
		return nullptr;
	}

	return Cast<ADC1BuildActor>(BuildTraget);
}

void UDC1GA_Player_BuildWork::StartWork(ADC1BuildActor* InBuildTarget)
{
	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (nullptr == InteractComp)
	{
		return;
	}

	InteractComp->RequestStartWork(InBuildTarget);
	InteractComp->SetTargetLocked(true);
}

void UDC1GA_Player_BuildWork::StopWork()
{
	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (nullptr == InteractComp)
	{
		return;
	}

	InteractComp->RequestStopWork();
	InteractComp->SetTargetLocked(false);
}

void UDC1GA_Player_BuildWork::OnWorkStarted()
{
	// TODO Play Montage
	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if(true == ::IsValid(ReleaseTask))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	//if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	//{
	//	if (UAnimMontage* Montage = AnimData->FindMontageByTag(WorkMontageTag))
	//	{
	//		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, "Start"))
	//		{
	//			PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
	//			PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
	//			PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
	//			PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
	//			PlayMontageTask->ReadyForActivation();
	//		}
	//	}
	//	else
	//	{
	//		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	//		return;
	//	}
	//}
}

UDC1InteractComponent* UDC1GA_Player_BuildWork::GetInteractComponent() const
{
	const FGameplayAbilityActorInfo* ActorInfo = GetCurrentActorInfo();
	if (nullptr == ActorInfo)
	{
		return nullptr;
	}
	
	ADC1PlayerController* PC = Cast<ADC1PlayerController>(ActorInfo->PlayerController);
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	return PC->GetInteractComponent();
}
