// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_CallPal.h"

#include "Tags/DC1GamePlayTags.h"
#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Projectile/Pal/DC1PalSphere.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"


UDC1GA_Player_CallPal::UDC1GA_Player_CallPal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FAbilityTriggerData Trigger = {};
	Trigger.TriggerTag = DC1PlayerTags::Event_Player_CallPal;
	Trigger.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(Trigger);
}

void UDC1GA_Player_CallPal::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_CallPal::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

bool UDC1GA_Player_CallPal::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UDC1GA_Player_CallPal::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (nullptr == TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ADC1PlayerState* PS = Cast<ADC1PlayerState>(ActorInfo->OwnerActor);
	if (false == IsValid(PS))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (UDC1PalPartyComponent* Component = PS->GetPalPartyComponent())
	{
		UAnimMontage* Montage = {};
		USoundBase* CallSound = {};
		// 소환
		if (nullptr == Component->GetDeployedPal())
		{
			Montage = DeployMontage;
			CallSound = DeploySound;
		}
		// 해제
		else
		{
			Montage = RetrieveMontage;
			CallSound = RetrieveSound;
		}

		if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f))
		{
			PlayMontageTask->OnCompleted.AddDynamic(this, &ThisClass::OnMontageCompleted);
			PlayMontageTask->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageCompleted);
			PlayMontageTask->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageAborted);
			PlayMontageTask->OnCancelled.AddDynamic(this, &ThisClass::OnMontageAborted);
			PlayMontageTask->ReadyForActivation();

			// 재생 실패 시 ReadyForActivation 안에서 OnCancelled → EndAbility 가 이미 불린다
			if (false == IsActive())
			{
				return;
			}

			const int32 SlotIndex = FMath::RoundToInt(TriggerEventData->EventMagnitude);
			Component->ToggleDeploy(SlotIndex);

			// 서버에서만 -> 플레이어 Multicast로 전원 재생 ( 예측 실행 클라와 중복 방지 )
			ADC1Player* Player = Cast<ADC1Player>(ActorInfo->AvatarActor.Get());
			if (true == ActorInfo->IsNetAuthority() && nullptr != Player)
			{
				Player->PlayWorldSound(CallSound, Player->GetActorLocation());

				// 몽타주 동안 손에 스피어를 든다. 액터·부착이 복제되어 다른 클라에도 보인다
				USkeletalMeshComponent* MeshComp = Player->GetMesh();
				UWorld* World = Player->GetWorld();
				if (nullptr != HeldSphereClass && nullptr != MeshComp && nullptr != World)
				{
					FActorSpawnParameters SpawnParams;
					SpawnParams.Owner = Player;
					SpawnParams.Instigator = Player;
					SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

					ADC1PalSphere* Sphere = World->SpawnActor<ADC1PalSphere>(HeldSphereClass, MeshComp->GetSocketTransform(HeldSocketName), SpawnParams);
					if (nullptr != Sphere)
					{
						Sphere->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HeldSocketName);
						Sphere->SetActorRelativeLocation(HeldSphereOffset);
						HeldSphere = Sphere;
					}
				}
			}
		}
		else
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}

void UDC1GA_Player_CallPal::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ADC1PalSphere* Sphere = HeldSphere.Get();
	if (true == ::IsValid(Sphere))
	{
		Sphere->Destroy();
	}
	HeldSphere.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
