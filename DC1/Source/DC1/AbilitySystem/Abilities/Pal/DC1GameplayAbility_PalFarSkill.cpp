// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalFarSkill.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_Damage.h"
#include "Projectile/Pal/DC1PalProjectile.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"

UDC1GameplayAbility_PalFarSkill::UDC1GameplayAbility_PalFarSkill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_Attacking);

	MontageTag = DC1PalTags::Anim_Pal_FarSkill;
	TriggerEventTag = DC1PalTags::Event_Pal_Attack_Hit;

	MaxRange = 1200.f;
}

void UDC1GameplayAbility_PalFarSkill::PerformSkillEffect()
{
	APawn* Pal = Cast<APawn>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (false == ::IsValid(Pal) || nullptr == SourceASC)
	{
		return;
	}

	if (nullptr == ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No ProjectileClass. Check the ability BP."), *GetName());
		return;
	}

	const FVector Direction = Pal->GetActorForwardVector().GetSafeNormal2D();
	const FVector MuzzleLocation = Pal->GetActorLocation() + (Direction * MuzzleForwardOffset) + (FVector::UpVector * MuzzleHeightOffset);

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.SetAbility(this);
	EffectContext.AddSourceObject(Pal);

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UDC1GameplayEffect_Damage::StaticClass(), GetAbilityLevel(), EffectContext);
	if (false == SpecHandle.IsValid())
	{
		return;
	}

	const float AttackPower = SourceASC->GetNumericAttribute(UDC1AttributeSet::GetAttackPowerAttribute());
	SpecHandle.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Damage, AttackPower);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Pal;
	SpawnParams.Instigator = Pal;

	ADC1PalProjectile* Projectile = Pal->GetWorld()->SpawnActor<ADC1PalProjectile>(ProjectileClass, MuzzleLocation, Direction.Rotation(), SpawnParams);
	if (nullptr != Projectile)
	{
		Projectile->DamageSpec = SpecHandle;
	}
}
