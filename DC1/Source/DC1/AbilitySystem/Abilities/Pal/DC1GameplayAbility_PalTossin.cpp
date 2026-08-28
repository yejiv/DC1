// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalTossin.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_Damage.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "GameFramework/RootMotionSource.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

UDC1GameplayAbility_PalTossin::UDC1GameplayAbility_PalTossin(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_Attacking);

	MontageTag = DC1PalTags::Anim_Pal_Tossin;
	TriggerEventTag = DC1PalTags::Event_Pal_Attack_Hit;

	MaxRange = 800.f;
	ApproachRange = 350.f;
	MinRange = 200.f;
}

void UDC1GameplayAbility_PalTossin::PerformSkillEffect()
{
	APawn* Pal = Cast<APawn>(GetAvatarActorFromActorInfo());
	if (false == ::IsValid(Pal))
	{
		return;
	}

	UWorld* World = Pal->GetWorld();
	if (nullptr == World)
	{
		return;
	}

	const FVector ChargeDirection = Pal->GetActorForwardVector().GetSafeNormal2D();

	if (AAIController* AIController = Cast<AAIController>(Pal->GetController()))
	{
		AIController->StopMovement();
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	DamagedActors.Reset();

	UAbilityTask_ApplyRootMotionConstantForce* ChargeTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
		this, NAME_None, ChargeDirection, ChargeSpeed, ChargeDuration,
		false,											// RVO 회피 우회한다
		nullptr,										// StrengthOverTime
		ERootMotionFinishVelocityMode::ClampVelocity,
		FVector::ZeroVector,
		0.f,											// 끝나면 즉시 정지
		true);											// bEnableGravity

	if (nullptr != ChargeTask)
	{
		ChargeTask->OnFinish.AddDynamic(this, &ThisClass::StopCharge);
		ChargeTask->ReadyForActivation();
	}

	World->GetTimerManager().SetTimer(SweepTimerHandle, this, &ThisClass::SweepAndApplyDamage, SweepInterval, true);
}

void UDC1GameplayAbility_PalTossin::SweepAndApplyDamage()
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
	const FVector End = Origin + (Forward * ChargeTraceLength);

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

		// 돌진 한 번에 대상당 한 번만
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

void UDC1GameplayAbility_PalTossin::StopCharge()
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SweepTimerHandle);
	}
}

void UDC1GameplayAbility_PalTossin::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	StopCharge();

	if (const APawn* Pal = Cast<APawn>(GetAvatarActorFromActorInfo()))
	{
		if (ADC1PalAIController* AIController = Cast<ADC1PalAIController>(Pal->GetController()))
		{
			if (AActor* Target = AIController->GetCurrentTarget())
			{
				AIController->SetFocus(Target, EAIFocusPriority::Gameplay);
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
