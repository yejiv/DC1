// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_RangedAttack.h"

#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

#include "Item/Equip/Weapon/RangedWeapon/DC1ProjectileWeapon.h"
#include "Tags/DC1GamePlayTags.h"

#include "Character/DC1Character.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"

UDC1GA_Player_RangedAttack::UDC1GA_Player_RangedAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	DrawMontageTag = DC1AnimTags::Anim_Attack_Bow;
}

void UDC1GA_Player_RangedAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 뗄 때 격발. ASC가 InputPressed/Released를 전달해줘야 이 태스크가 깨어난다
	if (UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	// 드로우 몽타주. 끝나도 어빌리티는 유지된다 (뗄 때까지 당긴 채로 대기)
	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		if (UAnimMontage* Montage = AnimData->FindMontageByTag(DrawMontageTag))
		{
			if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, "Start"))
			{
				PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
				PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
				PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
				PlayMontageTask->ReadyForActivation();
			}
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}
	}
}

void UDC1GA_Player_RangedAttack::OnInputReleased(float TimeHeld)
{
	MontageJumpToSection("Fire");
	// 격발은 서버만. 클라 인스턴스는 어빌리티 종료만 한다
	if (nullptr != CurrentActorInfo && true == CurrentActorInfo->IsNetAuthority())
	{
		if (ADC1ProjectileWeapon* Weapon = Cast<ADC1ProjectileWeapon>(GetEquippedWeapon()))
		{
			// 데미지는 발사 시점 값으로 굳혀서 화살에 들려보낸다. 조준 방향은 무기가 직접 뽑는다
			Weapon->FireProjectile(MakeDamageSpec());
		}
	}
}

void UDC1GA_Player_RangedAttack::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_RangedAttack::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
