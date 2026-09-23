// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalHit.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Controller/DC1PalAIController.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AIController.h"
#include "Animation/AnimMontage.h"

UDC1GameplayAbility_PalHit::UDC1GameplayAbility_PalHit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_HitReacting);

	// 연타로 다시 맞으면 진행 중인 리액션을 끊고 처음부터 다시 재생한다.
	// 자기 태그를 ActivationBlockedTags 에 두면 몽타주가 도는 동안 들어온 피격이 통째로 무시된다
	bRetriggerInstancedAbility = true;
}

void UDC1GameplayAbility_PalHit::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	const ADC1PalCharacter* Pal = (nullptr != ActorInfo) ? Cast<ADC1PalCharacter>(ActorInfo->AvatarActor.Get()) : nullptr;
	const UDC1PalData* PalData = (nullptr != Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
	{
		// 진행 중인 스킬을 끊기.
		// 취소는 어빌리티 단위라 스킬 태그가 아니라 발동 태그를 넘긴다
		if (const ADC1PalAIController* PalController = Cast<ADC1PalAIController>(Pal->GetController()))
		{
			ASC->CancelAbilityByTag(PalController->GetSelectedActivationTag());
		}
	}

	if (AAIController* AIController = Cast<AAIController>(Pal->GetController()))
	{
		AIController->PauseMove(FAIRequestID::CurrentRequest);
		AIController->ClearFocus(EAIFocusPriority::Gameplay); // 회전 금지
	}

	UAnimMontage* Montage = PalData->FindMontageByTag(DC1PalTags::Anim_Pal_Hit).LoadSynchronous();
	if (nullptr == Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No hit montage. Check PalData.Montages [Anim.Pal.Hit]."), *GetNameSafe(Pal));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	if (nullptr == MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// OnBlendOut은 안함. (회전이랑 몽타주랑 겹침)
	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->ReadyForActivation();
}

void UDC1GameplayAbility_PalHit::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const APawn* Pal = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (ADC1PalAIController* AIController = Cast<ADC1PalAIController>(Pal->GetController()))
		{
			AIController->ResumeMove(FAIRequestID::CurrentRequest);

			// 경직이 끝난 후 타겟 보기
			if (AActor* Target = AIController->GetCurrentTarget())
			{
				AIController->SetFocus(Target, EAIFocusPriority::Gameplay);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GameplayAbility_PalHit::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GameplayAbility_PalHit::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
