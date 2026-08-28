// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/Build/Player/DC1GA_Player_BuildWork.h"

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
	FinishWork();
}

void UDC1GA_Player_BuildWork::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_BuildWork::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
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
	if (true == ::IsValid(InBuildTarget))
	{
		BuildCompletedDelegateHandle = InBuildTarget->OnBuildCompleted.AddUObject(this, &ThisClass::HandleBuildCompleted);
		WorkTarget = InBuildTarget;
	}

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
	if (nullptr != WorkTarget.Get())
	{
		WorkTarget.Get()->OnBuildCompleted.Remove(BuildCompletedDelegateHandle);
	}

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (nullptr == InteractComp)
	{
		return;
	}

	// 건설 완료, 키입력 Completed가 아닌 다른 경로로 EndAbility가 호출된 경우를 위해 다시 호출
	InteractComp->RequestStopWork();

	InteractComp->SetTargetLocked(false);
}

void UDC1GA_Player_BuildWork::OnWorkStarted()
{
	IsSectionEnd = false;
	IsPlayingMontage = false;

	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if(true == ::IsValid(ReleaseTask))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player);
	if (true == ::IsValid(AnimData))
	{
		UAnimMontage* Montage = AnimData->FindMontageByTag(WorkMontageTag);
		if (true == ::IsValid(Montage))
		{
			UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, StartSectionName);
			if (true == ::IsValid(PlayMontageTask))
			{
				PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
				PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
				PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);

				PlayMontageTask->ReadyForActivation();

				IsPlayingMontage = true;
			}
		}
	}
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

void UDC1GA_Player_BuildWork::FinishWork()
{
	if (true == IsSectionEnd)
	{
		return;
	}

	IsSectionEnd = true;

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (nullptr != InteractComp)
	{
		InteractComp->RequestStopWork();
	}

	if (false == IsPlayingMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		MontageJumpToSection(EndSectionName);
	}
}

void UDC1GA_Player_BuildWork::HandleBuildCompleted()
{
	FinishWork();
}