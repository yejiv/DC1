// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_PalSphere.h"

#include "Animation/AnimMontage.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitInputRelease.h"

#include "Tags/DC1GamePlayTags.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Projectile/Pal/DC1PalSphere.h"

#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDC1GA_Player_PalSphere::UDC1GA_Player_PalSphere(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ThrowMontageTag = DC1AnimTags::Anim_Action_Throw;
}

void UDC1GA_Player_PalSphere::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 들고 있던 무기는 넣어두고 팰스피어를 손에 들려준다. 권한 확인은 EquipComponent가 한다
	if (UDC1EquipComponent* EquipComp = GetEquipComponent())
	{
		EquipComp->EquipPalSphere();
	}

	// 뗄 때 던지기. ASC가 InputPressed/Released를 전달해줘야 이 태스크가 깨어난다
	if (UAbilityTask_WaitInputRelease* ReleaseTask = UAbilityTask_WaitInputRelease::WaitInputRelease(this, true))
	{
		ReleaseTask->OnRelease.AddDynamic(this, &ThisClass::OnInputReleased);
		ReleaseTask->ReadyForActivation();
	}

	// 드로우 몽타주. 끝나도 어빌리티는 유지된다 (뗄 때까지 당긴 채로 대기)
	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		if (UAnimMontage* Montage = AnimData->FindMontageByTag(ThrowMontageTag))
		{
			if (UAbilityTask_PlayMontageAndWait* PlayMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, Montage, 1.f, "Loop"))
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

void UDC1GA_Player_PalSphere::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 캔슬/피격중단/사망 경로도 여기로 모이므로 무기 복원 누락이 없다
	if (UDC1EquipComponent* EquipComp = GetEquipComponent())
	{
		EquipComp->UnequipPalSphere();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UDC1EquipComponent* UDC1GA_Player_PalSphere::GetEquipComponent() const
{
	const ADC1Player* Player = Cast<ADC1Player>(GetAvatarActorFromActorInfo());
	if (false == ::IsValid(Player))
	{
		return nullptr;
	}

	return Player->Equipment;
}

void UDC1GA_Player_PalSphere::OnInputReleased(float TimeHeld)
{
	MontageJumpToSection("Throw");

	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	ADC1Player* Player = Cast<ADC1Player>(GetAvatarActorFromActorInfo());
	UWorld* World = ::IsValid(Player) ? Player->GetWorld() : nullptr;
	if (nullptr == World)
	{
		return;
	}

	if (nullptr == SphereClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: SphereClass is not set. Check GA_Player_PalSphere."), *GetName());
		return;
	}

	const FRotator AimRotation = Player->GetControlRotation();

	const USkeletalMeshComponent* MeshComp = Player->GetMesh();
	const FVector SpawnLocation = (nullptr != MeshComp && true == MeshComp->DoesSocketExist(ThrowSocketName))
		? MeshComp->GetSocketLocation(ThrowSocketName)
		: Player->GetPawnViewLocation() + (AimRotation.Vector() * ThrowForwardOffset);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADC1PalSphere* Sphere = World->SpawnActor<ADC1PalSphere>(SphereClass, SpawnLocation, AimRotation, SpawnParams);
	if (nullptr == Sphere)
	{
		return;
	}

	// 잡히면 파티에 들어간다. 결과는 명중한 뒤에 나오므로 델리게이트로 받는다
	if (const ADC1PlayerState* PlayerState = Player->GetPlayerState<ADC1PlayerState>())
	{
		if (UDC1PalPartyComponent* Party = PlayerState->GetPalPartyComponent())
		{
			Sphere->OnPalCaptured.AddUObject(Party, &UDC1PalPartyComponent::HandlePalCaptured);
		}
	}
}

void UDC1GA_Player_PalSphere::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GA_Player_PalSphere::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
