// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/Player/DC1GA_Player_Work.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
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

bool UDC1GA_Player_Work::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	// 서버( 원격 클라 )는 아직 대상을 모름 - TargetData 수신 후 OnWorkStarted에서 같은 확인
	if (nullptr == ActorInfo || false == ActorInfo->IsLocallyControlled())
	{
		return true;
	}

	const ADC1Character* Worker = Cast<ADC1Character>(ActorInfo->AvatarActor.Get());
	const ADC1InteractBuildActor* Target = (true == ::IsValid(Worker)) ? Worker->GetCurrentWorkTarget() : nullptr;

	return (true == ::IsValid(Target)) && (true == WorkSets.Contains(Target->GetWorkTypeTag()));
}

void UDC1GA_Player_Work::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 착용하지 않은 도구는 UnequipTool이 태그 비교로 무시한다 - 착용 전에 끝나는 경로( 대상 없음, Commit 실패 )도 그대로 통과
	if (true == CurrentToolTag.IsValid())
	{
		if (UDC1EquipComponent* EquipComp = GetEquipComponent())
		{
			EquipComp->UnequipTool(CurrentToolTag);
		}
	}

	CurrentToolTag = FGameplayTag::EmptyTag;

	// 서버 - 대상을 받기 전에 끝난 경우
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (true == ::IsValid(ASC))
	{
		ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey()).Remove(TargetDataDelegateHandle);
	}

	TargetDataDelegateHandle.Reset();

	// 로컬 - 완성 구독 / 대상 잠금 해제
	ADC1InteractBuildActor* Target = WorkTarget.Get();
	if (true == ::IsValid(Target))
	{
		Target->OnBuildCompleted.Remove(BuildCompletedDelegateHandle);
	}

	BuildCompletedDelegateHandle.Reset();

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (true == ::IsValid(InteractComp) && nullptr != ActorInfo && true == ActorInfo->IsLocallyControlled())
	{
		InteractComp->SetTargetLocked(false);
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

void UDC1GA_Player_Work::OnLocalWorkTargetFound(ADC1InteractBuildActor* InWorkTarget)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();

	// 원격 클라 - 서버 GA는 대상을 모르므로 TargetData로 전송 ( Standalone은 같은 인스턴스라 불필요 )
	if (true == ::IsValid(ASC) && nullptr != Info && false == Info->IsNetAuthority())
	{
		FScopedPredictionWindow ScopedPrediction(ASC);

		FGameplayAbilityTargetData_ActorArray* ActorData = new FGameplayAbilityTargetData_ActorArray();
		ActorData->TargetActorArray.Add(InWorkTarget);

		FGameplayAbilityTargetDataHandle DataHandle(ActorData);
		ASC->CallServerSetReplicatedTargetData(CurrentSpecHandle, CurrentActivationInfo.GetActivationPredictionKey(), DataHandle, FGameplayTag(), ASC->ScopedPredictionKey);
	}

	BuildCompletedDelegateHandle = InWorkTarget->OnBuildCompleted.AddUObject(this, &ThisClass::HandleBuildCompleted);

	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (true == ::IsValid(InteractComp))
	{
		InteractComp->SetTargetLocked(true);
	}

	BeginWork(InWorkTarget);
}

void UDC1GA_Player_Work::WaitWorkTarget()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (false == ::IsValid(ASC))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	const FPredictionKey PredictionKey = CurrentActivationInfo.GetActivationPredictionKey();

	TargetDataDelegateHandle = ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, PredictionKey).AddUObject(this, &ThisClass::OnWorkTargetDataReceived);

	// 활성화보다 먼저 도착했으면 바로 호출
	ASC->CallReplicatedTargetDataDelegatesIfSet(CurrentSpecHandle, PredictionKey);
}

void UDC1GA_Player_Work::OnWorkTargetDataReceived(const FGameplayAbilityTargetDataHandle& InData, FGameplayTag InActivationTag)
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	if (true == ::IsValid(ASC))
	{
		const FPredictionKey PredictionKey = CurrentActivationInfo.GetActivationPredictionKey();

		ASC->AbilityTargetDataSetDelegate(CurrentSpecHandle, PredictionKey).Remove(TargetDataDelegateHandle);
		ASC->ConsumeClientReplicatedTargetData(CurrentSpecHandle, PredictionKey);
	}

	TargetDataDelegateHandle.Reset();

	ADC1InteractBuildActor* Target = nullptr;
	if (true == InData.IsValid(0))
	{
		const TArray<TWeakObjectPtr<AActor>> Actors = InData.Get(0)->GetActors();
		if (Actors.Num() > 0)
		{
			Target = Cast<ADC1InteractBuildActor>(Actors[0].Get());
		}
	}

	if (false == ::IsValid(Target))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 검증은 StartWork ( 거리 )
	BeginWork(Target);
}

bool UDC1GA_Player_Work::StartWork(ADC1InteractBuildActor* InWorkTarget)
{
	ADC1Player* Player = Cast<ADC1Player>(GetAvatarActorFromActorInfo());
	UDC1InteractComponent* InteractComp = GetInteractComponent();
	if (false == ::IsValid(Player) || false == ::IsValid(InteractComp))
	{
		return false;
	}

	// 클라가 보낸 대상 - 서버 기준 거리 검증
	if (false == InteractComp->IsInInteractRange(Player, InWorkTarget))
	{
		return false;
	}

	InWorkTarget->AddWorker(Player);

	return true;
}

void UDC1GA_Player_Work::StopWork()
{
	ADC1InteractBuildActor* Target = WorkTarget.Get();
	if (true == ::IsValid(Target))
	{
		Target->RemoveWorker(Cast<ADC1Player>(GetAvatarActorFromActorInfo()));
	}
}

void UDC1GA_Player_Work::OnWorkStarted()
{
	IsSectionEnd = false;
	IsPlayingMontage = false;

	ADC1InteractBuildActor* Target = WorkTarget.Get();
	const FDC1PlayerWorkSet* WorkSet = (true == ::IsValid(Target)) ? WorkSets.Find(Target->GetWorkTypeTag()) : nullptr;
	if (nullptr == WorkSet)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1GA_Player_Work :: No WorkSet for [%s] - Check WorkSets"), (true == ::IsValid(Target)) ? *Target->GetWorkTypeTag().ToString() : TEXT("None"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	// 도구를 쓰지 않는 Work도 있으므로 태그가 비어 있으면 무기를 그대로 둔다
	// 착용/해제는 EquipComponent가 서버 권한을 확인한다
	CurrentToolTag = WorkSet->ToolTag;
	if (true == CurrentToolTag.IsValid())
	{
		if (UDC1EquipComponent* EquipComp = GetEquipComponent())
		{
			EquipComp->EquipTool(CurrentToolTag);
		}
	}

	UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true);
	if (true == ::IsValid(ReleaseTask))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	CurrentEndSectionName = WorkSet->EndSectionName;

	const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player);
	if (true == ::IsValid(AnimData))
	{
		UAnimMontage* Montage = AnimData->FindMontageByTag(WorkSet->MontageTag);
		if (true == ::IsValid(Montage))
		{
			// 몽타주에 없는 섹션이면 처음부터 ( 없는 섹션 점프는 엔진이 조용히 무시 )
			const FName StartSection = (true == Montage->IsValidSectionName(WorkSet->StartSectionName)) ? WorkSet->StartSectionName : NAME_None;

			UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, StartSection);
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

	// 서버 - 키를 뗀 순간 진행 정지 ( 몽타주 End 섹션을 기다리지 않음 ). 클라의 Release는 WaitInputRelease가 서버로 전달
	const FGameplayAbilityActorInfo* Info = GetCurrentActorInfo();
	if (nullptr != Info && true == Info->IsNetAuthority())
	{
		StopWork();
	}

	if (false == IsPlayingMontage)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}
	else
	{
		UAnimMontage* Montage = GetCurrentMontage();
		if (true == ::IsValid(Montage) && Montage->IsValidSectionName(CurrentEndSectionName))
		{
			MontageJumpToSection(CurrentEndSectionName);
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
