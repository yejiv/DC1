// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_Damage.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"

UDC1GameplayAbility_PalSkill::UDC1GameplayAbility_PalSkill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;
}

void UDC1GameplayAbility_PalSkill::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
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

	if (true == TriggerEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* TriggerTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, TriggerEventTag, nullptr, false, true);
		if (nullptr != TriggerTask)
		{
			TriggerTask->EventReceived.AddDynamic(this, &ThisClass::OnTriggerEventReceived);
			TriggerTask->ReadyForActivation();
		}
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

void UDC1GameplayAbility_PalSkill::OnTriggerEventReceived(FGameplayEventData Payload)
{
	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	PerformSkillEffect();
}

void UDC1GameplayAbility_PalSkill::SweepAndApplyDamage(float Length, TSet<TObjectPtr<AActor>>& InOutDamagedActors)
{
	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	if (false == IsValid(Pal))
	{
		return;
	}

	UWorld* World = Pal->GetWorld();
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (nullptr == World || nullptr == SourceASC)
	{
		return;
	}

	const FVector Forward = Pal->GetActorForwardVector();
	const FVector Origin = Pal->GetActorLocation() + (Forward * Pal->GetSimpleCollisionRadius());
	const FVector End = Origin + (Forward * Length);

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(Pal);

	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(Hits, Origin, End, FQuat::Identity, ObjectParams,
		FCollisionShape::MakeSphere(TraceRadius), QueryParams);

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (false == IsValid(HitActor) || HitActor == Pal)
		{
			continue;
		}

		if (true == InOutDamagedActors.Contains(HitActor))
		{
			continue;
		}

		if (ETeamAttitude::Hostile != Pal->GetTeamAttitudeTowards(*HitActor))
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(HitActor);
		if (nullptr == TargetASC)
		{
			continue;
		}

		InOutDamagedActors.Add(HitActor);

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.SetAbility(this);
		EffectContext.AddSourceObject(Pal);
		EffectContext.AddHitResult(Hit);

		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UDC1GameplayEffect_Damage::StaticClass(), GetAbilityLevel(), EffectContext);
		if (false == SpecHandle.IsValid())
		{
			continue;
		}

		const float AttackPower = SourceASC->GetNumericAttribute(UDC1AttributeSet::GetAttackPowerAttribute());
		SpecHandle.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Damage, AttackPower);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void UDC1GameplayAbility_PalSkill::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GameplayAbility_PalSkill::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
