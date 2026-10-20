// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Executions/DC1PalDamageExecution.h"

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"

#include "AbilitySystemComponent.h"

struct FDC1PalDamageStatics
{
	FGameplayEffectAttributeCaptureDefinition AttackPowerDef;
	FGameplayEffectAttributeCaptureDefinition DefensePowerDef;

	FDC1PalDamageStatics()
	{
		// 공격력은 가해자에게서, 발사 시점에 스냅샷한다 (bSnapshot = true)
		AttackPowerDef = FGameplayEffectAttributeCaptureDefinition(
			UDC1CharacterAttributeSet::GetAttackPowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Source,
			true);

		// 방어력은 대상에게서, 적용되는 순간에 읽는다 (bSnapshot = false)
		DefensePowerDef = FGameplayEffectAttributeCaptureDefinition(
			UDC1AttributeSet::GetDefensePowerAttribute(),
			EGameplayEffectAttributeCaptureSource::Target,
			false);
	}
};

static const FDC1PalDamageStatics& DamageStatics()
{
	static FDC1PalDamageStatics Statics;
	return Statics;
}

UDC1PalDamageExecution::UDC1PalDamageExecution()
{
	RelevantAttributesToCapture.Add(DamageStatics().AttackPowerDef);
	RelevantAttributesToCapture.Add(DamageStatics().DefensePowerDef);
}

void UDC1PalDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
													FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	const UDC1GameplayAbility_PalSkill* PalSkill = Cast<UDC1GameplayAbility_PalSkill>(Spec.GetContext().GetAbility());

	// 팰 스킬이 아니면 SetByCaller 로 완성된 데미지를 보낸다. -> 이미 공격력이 적용됨
	if (nullptr == PalSkill)
	{
		const float PassThrough = Spec.GetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Damage, false, 0.f);
		if (PassThrough > 0.f)
		{
			OutExecutionOutput.AddOutputModifier(
				FGameplayModifierEvaluatedData(UDC1AttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, PassThrough));
		}

		return;
	}

	const float BasePower = PalSkill->GetSkillPower();
	if (BasePower <= 0.f)
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] DamageExecution: Can't find PalRegistryData. Check PDA_AssetData."));
		return;
	}

	// 데미지 = 위력 × (공격력 ÷ 대상 방어력)
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().AttackPowerDef, EvaluateParameters, AttackPower);

	float DefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().DefensePowerDef, EvaluateParameters, DefensePower);

	const float EffectiveDefense = (DefensePower > 0.f) ? DefensePower : Registry->StatGrowth.DefenseFlatBase;

	float Damage = BasePower * (AttackPower / EffectiveDefense);

	const EDC1PalElement SkillElement = PalSkill->GetSkillElement();

	// 상성 — 대상이 팰일 때만
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	if (const ADC1PalCharacter* TargetPal = Cast<ADC1PalCharacter>((nullptr != TargetASC) ? TargetASC->GetAvatarActor() : nullptr))
	{
		Damage *= Registry->GetElementMultiplier(SkillElement, TargetPal->GetElement(), TargetPal->GetElement2());
	}

	// 일치 보너스
	if (const ADC1PalCharacter* SourcePal = Cast<ADC1PalCharacter>(Spec.GetContext().GetEffectCauser()))
	{
		if (SkillElement == SourcePal->GetElement() || SkillElement == SourcePal->GetElement2())
		{
			Damage *= Registry->ElementMatchBonus;
		}
	}

	if (Damage <= 0.f)
	{
		return;
	}

	OutExecutionOutput.AddOutputModifier(
		FGameplayModifierEvaluatedData(UDC1AttributeSet::GetDamageAttribute(), EGameplayModOp::Additive, Damage));
}
