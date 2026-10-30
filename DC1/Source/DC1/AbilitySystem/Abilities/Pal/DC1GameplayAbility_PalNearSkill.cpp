// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalNearSkill.h"

#include "AbilitySystem/Effects/DC1GameplayEffect_PalDamage.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"
#include "Define/DC1Enums.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"

UDC1GameplayAbility_PalNearSkill::UDC1GameplayAbility_PalNearSkill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_Attacking);

	MontageTag = DC1PalTags::Anim_Pal_NearSkill;
	TriggerEventTag = DC1PalTags::Event_Pal_Attack_Hit;

	MaxRange = 250.f;
	ApproachRange = 150.f;
	MinRange = 0.f;
}

void UDC1GameplayAbility_PalNearSkill::PerformSkillEffect()
{
	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (false == ::IsValid(Pal) || nullptr == SourceASC)
	{
		return;
	}

	UWorld* World = Pal->GetWorld();
	if (nullptr == World)
	{
		return;
	}

	const FVector Forward = Pal->GetActorForwardVector().GetSafeNormal2D();
	const FVector Center = Pal->GetActorLocation() + (Forward * (Pal->GetSimpleCollisionRadius() + GetHitReach()));

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Player));
	ObjectParams.AddObjectTypesToQuery(static_cast<ECollisionChannel>(EDC1CollisionLayer::ECC_Pal));

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = false;
	QueryParams.AddIgnoredActor(Pal);

	TArray<FOverlapResult> Overlaps;
	World->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjectParams, FCollisionShape::MakeSphere(GetHitRadius()), QueryParams);

	// 한 액터의 컴포넌트가 여러 개 걸려도 이번 노티파이에선 한 번만
	TSet<AActor*> HitActors;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (false == ::IsValid(HitActor) || true == HitActors.Contains(HitActor))
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

		HitActors.Add(HitActor);

		FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
		EffectContext.SetAbility(this);
		EffectContext.AddSourceObject(Pal);

		const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UDC1GameplayEffect_PalDamage::StaticClass(), GetAbilityLevel(), EffectContext);
		if (false == SpecHandle.IsValid())
		{
			continue;
		}

		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
	}
}
