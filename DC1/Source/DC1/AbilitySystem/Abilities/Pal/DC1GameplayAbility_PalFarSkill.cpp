// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalFarSkill.h"

#include "AbilitySystem/Effects/DC1GameplayEffect_PalDamage.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Projectile/Pal/DC1PalProjectile.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

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
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	const UDC1PalData* PalData = ::IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		return;
	}

	// 탄은 스킬이 소유한다 - 아이스 미사일은 어느 팰이 쏘든 얼음 탄이다.
	// 표에 없으면 종별 PalData.Projectiles 로 폴백한다
	ActiveProjectileClass = (true == bHasSkillDef && nullptr != SkillDef.ProjectileClass)
		? SkillDef.ProjectileClass
		: PalData->FindProjectileByTag(ProjectileTag);

	if (nullptr == ActiveProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No projectile for skill. Check SkillChart.ProjectileClass or PalData.Projectiles [%s]."),
			*GetNameSafe(Pal), *ProjectileTag.ToString());
		return;
	}

	const int32 ShotCount = GetShotCount();
	const float ShotInterval = bHasSkillDef ? SkillDef.ShotInterval : 0.f;
	const float SpreadAngle = bHasSkillDef ? SkillDef.SpreadAngle : 0.f;

	// 연사 -- 첫 발은 지금, 나머지는 타이머
	if (1 < ShotCount && 0.f < ShotInterval)
	{
		RemainingShots = ShotCount - 1;
		FireProjectile(FMath::FRandRange(-0.5f, 0.5f) * SpreadAngle);
		GetWorld()->GetTimerManager().SetTimer(ShotTimerHandle, this, &ThisClass::FireNextShot, ShotInterval, true);
		return;
	}

	// 한꺼번에 -- 부채꼴로 고르게
	for (int32 Index = 0; Index < ShotCount; ++Index)
	{
		const float Alpha = (1 < ShotCount) ? static_cast<float>(Index) / (ShotCount - 1) : 0.5f;
		FireProjectile((Alpha - 0.5f) * SpreadAngle);
	}
}

void UDC1GameplayAbility_PalFarSkill::FireNextShot()
{
	const float SpreadAngle = bHasSkillDef ? SkillDef.SpreadAngle : 0.f;
	FireProjectile(FMath::FRandRange(-0.5f, 0.5f) * SpreadAngle);

	--RemainingShots;
	if (RemainingShots <= 0)
	{
		GetWorld()->GetTimerManager().ClearTimer(ShotTimerHandle);
	}
}

void UDC1GameplayAbility_PalFarSkill::FireProjectile(float InYawOffset)
{
	ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetAvatarActorFromActorInfo());
	UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
	if (false == ::IsValid(Pal) || nullptr == SourceASC || nullptr == ActiveProjectileClass)
	{
		return;
	}

	const FVector Direction = Pal->GetActorForwardVector().GetSafeNormal2D().RotateAngleAxis(InYawOffset, FVector::UpVector);

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

	ADC1PalProjectile* Projectile = Pal->GetWorld()->SpawnActor<ADC1PalProjectile>(ActiveProjectileClass, MuzzleLocation, Direction.Rotation(), SpawnParams);
	if (nullptr == Projectile)
	{
		return;
	}

	Projectile->DamageSpec = SpecHandle;
}

void UDC1GameplayAbility_PalFarSkill::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ShotTimerHandle);
	}

	RemainingShots = 0;

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
