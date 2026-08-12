// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalAttack.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_PalDamage.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"

UDC1GameplayAbility_PalAttack::UDC1GameplayAbility_PalAttack(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerInitiated;

	ActivationOwnedTags.AddTag(DC1PalTags::Pal_State_Attacking);

	MontageTag = DC1PalTags::Ability_Pal_Attack;
	HitEventTag = DC1PalTags::Event_Pal_Attack_Hit;
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

	// 타격 이벤트 대기
	if (true == HitEventTag.IsValid())
	{
		UAbilityTask_WaitGameplayEvent* HitEventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, HitEventTag, nullptr, false, true);
		if (nullptr != HitEventTask)
		{
			HitEventTask->EventReceived.AddDynamic(this, &ThisClass::OnHitEventReceived);
			HitEventTask->ReadyForActivation();
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

void UDC1GameplayAbility_PalAttack::OnHitEventReceived(FGameplayEventData Payload)
{
	// AnimNotify는 몽타주가 도는 모든 머신에서 발화한다. (판정, GE 적용은 서버에서 한 번만)
	if (nullptr == CurrentActorInfo || false == CurrentActorInfo->IsNetAuthority())
	{
		return;
	}

	PerformAttackTrace();
}

void UDC1GameplayAbility_PalAttack::PerformAttackTrace()
{
	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	if (false == ::IsValid(Pal))
	{
		return;
	}

	const UDC1PalData* PalData = Pal->GetPalData();
	if (nullptr == PalData)
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
	const FVector End = Origin + (Forward * PalData->AttackRange);

	// 오브젝트 쿼리 쓰기
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(Pal);

	TArray<FHitResult> Hits;
	World->SweepMultiByObjectType(Hits, Origin, End, FQuat::Identity, ObjectParams,
		FCollisionShape::MakeSphere(PalData->AttackTraceRadius), QueryParams);

	TSet<AActor*> DamagedActors;

	for (const FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (false == ::IsValid(HitActor) || HitActor == Pal)
		{
			continue;
		}

		if (true == DamagedActors.Contains(HitActor))
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

		DamagedActors.Add(HitActor);

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.SetAbility(this);
		EffectContext.AddSourceObject(Pal);
		EffectContext.AddHitResult(Hit);

		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UDC1GameplayEffect_PalDamage::StaticClass(), GetAbilityLevel(), EffectContext);
		if (false == SpecHandle.IsValid())
		{
			continue;
		}

		// 공격력은 어트리뷰트가 진실이다. (PalData는 초기값만 공급)
		const float AttackPower = SourceASC->GetNumericAttribute(UDC1AttributeSet::GetAttackPowerAttribute());
		SpecHandle.Data->SetSetByCallerMagnitude(DC1PalTags::SetByCaller_Damage, AttackPower);
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}

void UDC1GameplayAbility_PalAttack::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UDC1GameplayAbility_PalAttack::OnMontageAborted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
