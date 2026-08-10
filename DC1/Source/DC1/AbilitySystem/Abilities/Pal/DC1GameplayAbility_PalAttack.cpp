// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalAttack.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Animation/AnimMontage.h"

UDC1GameplayAbility_PalAttack::UDC1GameplayAbility_PalAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	ActivationOwnedTags.AddTag(DC1PalTags::Pal_State_Attacking);

	MontageTag = DC1PalTags::Ability_Pal_Attack;
}

void UDC1GameplayAbility_PalAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No PalData on avatar."), *GetName());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스폰 시점에 미리 로드하는 쪽으로 바꿔야됨
	UAnimMontage* Montage = PalData->FindMontageByTag(MontageTag).LoadSynchronous();
	if (nullptr == Montage)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No montage for tag [%s]. Check PalData.Montages."), *GetNameSafe(Pal), *MontageTag.ToString());
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage);
	if (nullptr == MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
	MontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
	MontageTask->ReadyForActivation();
}

void UDC1GameplayAbility_PalAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GameplayAbility_PalAttack::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
