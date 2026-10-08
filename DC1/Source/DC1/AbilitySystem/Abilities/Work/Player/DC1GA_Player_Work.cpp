// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/Player/DC1GA_Player_Work.h"

#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"
#include "Tags/DC1GamePlayTags.h"

UDC1GA_Player_Work::UDC1GA_Player_Work(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dead);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Build_Placement);
}

void UDC1GA_Player_Work::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 착용하지 않은 도구는 UnequipTool이 태그 비교로 무시한다 - 착용 전에 끝나는 경로( 대상 없음, Commit 실패 )도 그대로 통과
	if (true == EquipToolTag.IsValid())
	{
		if (UDC1EquipComponent* EquipComp = GetEquipComponent())
		{
			EquipComp->UnequipTool(EquipToolTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GA_Player_Work::OnInputReleased(float TimeHeld)
{
	FinishWork();
}

void UDC1GA_Player_Work::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_Work::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

ADC1InteractBuildActor* UDC1GA_Player_Work::FindWorkTarget() const
{
	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (false == ::IsValid(InteractComp))
	{
		return nullptr;
	}

	AActor* CurrentTarget = InteractComp->GetCurrentTarget();
	if (false == ::IsValid(CurrentTarget))
	{
		return nullptr;
	}

	return Cast<ADC1InteractBuildActor>(CurrentTarget);
}

void UDC1GA_Player_Work::StartWork(ADC1InteractBuildActor* InWorkTarget)
{
	if (true == ::IsValid(InWorkTarget))
	{
		BuildCompletedDelegateHandle = InWorkTarget->OnBuildCompleted.AddUObject(this, &ThisClass::HandleBuildCompleted);
		WorkTarget = InWorkTarget;
	}

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (false == ::IsValid(InteractComp))
	{
		return;
	}

	InteractComp->RequestStartWork(InWorkTarget);
	InteractComp->SetTargetLocked(true);
}

void UDC1GA_Player_Work::StopWork()
{
	ADC1InteractBuildActor* Target = WorkTarget.Get();
	if (true == ::IsValid(Target))
	{
		Target->OnBuildCompleted.Remove(BuildCompletedDelegateHandle);
	}

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (false == ::IsValid(InteractComp))
	{
		return;
	}

	// 건설 완료, 키입력 Completed가 아닌 다른 경로로 EndAbility가 호출된 경우를 위해 다시 호출
	InteractComp->RequestStopWork();

	InteractComp->SetTargetLocked(false);
}

void UDC1GA_Player_Work::OnWorkStarted()
{
	IsSectionEnd = false;
	IsPlayingMontage = false;

	// 도구를 쓰지 않는 Work도 있으므로 태그가 비어 있으면 무기를 그대로 둔다
	// 착용/해제는 EquipComponent가 서버 권한을 확인한다
	if (true == EquipToolTag.IsValid())
	{
		if (UDC1EquipComponent* EquipComp = GetEquipComponent())
		{
			EquipComp->EquipTool(EquipToolTag);
		}
	}

	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (true == ::IsValid(ReleaseTask))
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

UDC1InteractComponent* UDC1GA_Player_Work::GetInteractComponent() const
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

UDC1EquipComponent* UDC1GA_Player_Work::GetEquipComponent() const
{
	// ASC가 PlayerState에 있으므로 OwningActor가 곧 PlayerState다
	const ADC1PlayerState* PS = Cast<ADC1PlayerState>(GetOwningActorFromActorInfo());
	if (false == ::IsValid(PS))
	{
		return nullptr;
	}

	return PS->GetEquipComponent();
}

void UDC1GA_Player_Work::FinishWork()
{
	if (true == IsSectionEnd)
	{
		return;
	}

	IsSectionEnd = true;

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (true == ::IsValid(InteractComp))
	{
		InteractComp->RequestStopWork();
	}

	if (false == IsPlayingMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		UAnimMontage* Montage = GetCurrentMontage();
		if (true == ::IsValid(Montage) && Montage->IsValidSectionName(EndSectionName))
		{
			MontageJumpToSection(EndSectionName);
		}
		else
		{
			UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
			if (true == ::IsValid(ASC))
			{
				FName CurrentSectionName = ASC->GetCurrentMontageSectionName();
				MontageSetNextSectionName(CurrentSectionName, NAME_None);
			}
		}
	}
}

void UDC1GA_Player_Work::HandleBuildCompleted()
{
	FinishWork();
}