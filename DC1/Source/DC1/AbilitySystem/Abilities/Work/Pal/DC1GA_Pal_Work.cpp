// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/Pal/DC1GA_Pal_Work.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/State/DC1GameplayTag_State.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Item/Equip/Tool/DC1Tool.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDC1GA_Pal_Work::UDC1GA_Pal_Work(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UDC1GA_Pal_Work::StartWork(IDC1WorkTargetInterface* InWorkTarget)
{
	ADC1PalCharacter* Pal = GetPal();
	if (false == ::IsValid(Pal) || false == InWorkTarget->IsPalWorkerAssigned(Pal))
	{
		return false;
	}

	PalWorkersChangedHandle = InWorkTarget->GetOnPalWorkersChanged().AddUObject(this, &ThisClass::HandlePalWorkersChanged);
	InWorkTarget->AddWorker(Pal);

	SpawnWorkTool(InWorkTarget);

	return true;
}

void UDC1GA_Pal_Work::StopWork()
{
	IDC1WorkTargetInterface* Target = WorkTarget.Get();
	if (nullptr != Target)
	{
		Target->GetOnPalWorkersChanged().Remove(PalWorkersChangedHandle);
		Target->RemoveWorker(GetPal());
	}

	PalWorkersChangedHandle.Reset();

	ADC1Tool* Tool = WorkTool.Get();
	if (true == ::IsValid(Tool))
	{
		Tool->Destroy();
	}
	WorkTool.Reset();
}

void UDC1GA_Pal_Work::SpawnWorkTool(IDC1WorkTargetInterface* InWorkTarget)
{
	const TSubclassOf<ADC1Tool>* ToolClass = (nullptr != InWorkTarget) ? WorkTools.Find(InWorkTarget->GetWorkTypeTag()) : nullptr;
	if (nullptr == ToolClass || nullptr == *ToolClass)
	{
		return;
	}

	ADC1PalCharacter* Pal = GetPal();
	USkeletalMeshComponent* MeshComp = ::IsValid(Pal) ? Pal->GetMesh() : nullptr;
	UWorld* World = ::IsValid(Pal) ? Pal->GetWorld() : nullptr;
	if (nullptr == MeshComp || nullptr == World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Pal;
	SpawnParams.Instigator = Pal;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADC1Tool* Tool = World->SpawnActor<ADC1Tool>(*ToolClass, MeshComp->GetSocketTransform(ToolSocketName), SpawnParams);
	if (nullptr == Tool)
	{
		return;
	}

	Tool->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ToolSocketName);
	Tool->SetActorHiddenInGame(false);

	WorkTool = Tool;
}

void UDC1GA_Pal_Work::OnWorkStarted()
{
	// 타격 이벤트 대기 ( 반복 ) - 몽타주와 같은 수명, GA가 끝나면 태스크도 정리
	UAbilityTask_WaitGameplayEvent* WaitHitTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, DC1PalTags::Event_Pal_Work_Hit, nullptr, false, true);
	if (true == ::IsValid(WaitHitTask))
	{
		WaitHitTask->EventReceived.AddDynamic(this, &ThisClass::HandleWorkHit);
		WaitHitTask->ReadyForActivation();
	}

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

void UDC1GA_Pal_Work::HandleWorkHit(FGameplayEventData InPayload)
{
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	if (nullptr == Info || false == Info->IsNetAuthority())
	{
		return;
	}

	IDC1WorkTargetInterface* Target = WorkTarget.Get();
	if (nullptr == Target)
	{
		return;
	}

	Target->ApplyWorkHit(GetPal(), InPayload.EventMagnitude);
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
	IDC1WorkTargetInterface* Target = WorkTarget.Get();
	if (nullptr == Target)
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
