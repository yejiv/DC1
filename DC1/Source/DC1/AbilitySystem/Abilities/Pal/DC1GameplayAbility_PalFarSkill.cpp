// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalFarSkill.h"

#include "AbilitySystem/Effects/DC1GameplayEffect_PalDamage.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Projectile/Pal/DC1PalProjectile.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

UDC1GameplayAbility_PalFarSkill::UDC1GameplayAbility_PalFarSkill(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActivationOwnedTags.AddTag(DC1PalTags::State_Pal_Attacking);

	MontageTag = DC1PalTags::Anim_Pal_FarSkill;
	TriggerEventTag = DC1PalTags::Event_Pal_Attack_Hit;
	ProjectileTag = DC1PalTags::Ability_Pal_FarSkill;

	MaxRange = 1200.f;
	ApproachRange = 700.f;
	MinRange = 400.f;
}

void UDC1GameplayAbility_PalFarSkill::PerformSkillEffect()
{
	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (false == ::IsValid(Pal) || nullptr == SourceASC)
	{
		return;
	}

	const UDC1PalData* PalData = Pal->GetPalData();
	if (nullptr == PalData)
	{
		return;
	}

	// 탄은 스킬이 소유한다 - 아이스 미사일은 어느 팰이 쏘든 얼음 탄이다.
	// 표에 없으면 종별 PalData.Projectiles 로 폴백한다
	const TSubclassOf<ADC1PalProjectile> ProjectileClass = (true == bHasSkillDef && nullptr != SkillDef.ProjectileClass)
		? SkillDef.ProjectileClass
		: PalData->FindProjectileByTag(ProjectileTag);

	if (nullptr == ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No projectile for skill. Check SkillChart.ProjectileClass or PalData.Projectiles [%s]."),
			*GetNameSafe(Pal), *ProjectileTag.ToString());
		return;
	}

	const FVector Direction = Pal->GetActorForwardVector().GetSafeNormal2D();

	//발사 위치 찾기
	const USkeletalMeshComponent* MeshComp = Pal->GetMesh();
	const FVector MuzzleLocation = (nullptr != MeshComp && true == MeshComp->DoesSocketExist(MuzzleSocketName))
		? MeshComp->GetSocketLocation(MuzzleSocketName)
		: Pal->GetActorLocation() + (Direction * MuzzleForwardOffset) + (FVector::UpVector * MuzzleHeightOffset);

	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
	EffectContext.SetAbility(this);
	EffectContext.AddSourceObject(Pal);

	const FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(UDC1GameplayEffect_PalDamage::StaticClass(), GetAbilityLevel(), EffectContext);
	if (false == SpecHandle.IsValid())
	{
		return;
	}


	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Pal;
	SpawnParams.Instigator = Pal;

	ADC1PalProjectile* Projectile = Pal->GetWorld()->SpawnActor<ADC1PalProjectile>(ProjectileClass, MuzzleLocation, Direction.Rotation(), SpawnParams);
	if (nullptr == Projectile)
	{
		return;
	}

	Projectile->DamageSpec = SpecHandle;
}
