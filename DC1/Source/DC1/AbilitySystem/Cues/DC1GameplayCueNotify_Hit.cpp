// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Cues/DC1GameplayCueNotify_Hit.h"

#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "Actor/Resource/DC1ResourceActor.h"
#include "Character/Player/DC1Player.h"
#include "Define/DC1HitHelper.h"
#include "Tags/DC1GamePlayTags.h"

#include "AbilitySystemComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Character.h"
#include "GameplayEffectExtension.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

bool UDC1GameplayCueNotify_Hit::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	if (false == ::IsValid(MyTarget) || NM_DedicatedServer == MyTarget->GetNetMode())
	{
		return false;
	}

	// 플레이어 실드 감소 - 플레이어 중심에
	if (true == Parameters.OriginalTag.MatchesTagExact(DC1EffectTags::GameplayCue_Hit_Shield))
	{
		if (false == ::IsValid(ShieldFX))
		{
			return false;
		}

		UNiagaraComponent* Shield = UNiagaraFunctionLibrary::SpawnSystemAttached(ShieldFX, MyTarget->GetRootComponent(), NAME_None,
			FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		if (false == ::IsValid(Shield))
		{
			return false;
		}

		// 파편 2~3개가 같은 방향으로 - 방향만 매번 랜덤
		Shield->SetVariableVec3(TEXT("ShardDir"), FMath::VRand());
		return true;
	}

	// 리소스 - 맞은 표면에
	const bool IsWood = Parameters.OriginalTag.MatchesTagExact(DC1EffectTags::GameplayCue_Hit_Wood);
	if (true == IsWood || true == Parameters.OriginalTag.MatchesTagExact(DC1EffectTags::GameplayCue_Hit_Stone))
	{
		UNiagaraSystem* ResourceFX = (true == IsWood) ? WoodFX : StoneFX;
		if (false == ::IsValid(ResourceFX))
		{
			return false;
		}

		return nullptr != UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget, ResourceFX, Parameters.Location, Parameters.Normal.Rotation());
	}

	// 캐릭터 체력 감소 - 맞은 표면에
	const int32 ElementIndex = FMath::RoundToInt(Parameters.RawMagnitude);
	const EDC1PalElement Element = (0 <= ElementIndex && ElementIndex < static_cast<int32>(EDC1PalElement::Max)) ? static_cast<EDC1PalElement>(ElementIndex) : EDC1PalElement::Neutral;

	UNiagaraSystem* FX = GetElementFX(Element, nullptr != Cast<ADC1Player>(MyTarget));
	if (false == ::IsValid(FX))
	{
		return false;
	}

	return nullptr != UNiagaraFunctionLibrary::SpawnSystemAtLocation(MyTarget, FX, Parameters.Location, Parameters.Normal.Rotation());
}

void UDC1GameplayCueNotify_Hit::NotifyHit(const FGameplayEffectModCallbackData& InData)
{
	const AActor* Target = InData.Target.GetAvatarActor();

	// 리소스 - 플레이어 타격, 팰 작업 모두 같은 Damage GE 경로로 들어옴
	if (const ADC1ResourceActor* Resource = Cast<ADC1ResourceActor>(Target))
	{
		NotifyResourceHit(InData, Resource);
		return;
	}

	// 캐릭터 ( 팰, 플레이어 )
	if (true == ::IsValid(Cast<ACharacter>(Target)))
	{
		NotifyCharacterHit(InData, Target);
	}
}

void UDC1GameplayCueNotify_Hit::NotifyCharacterHit(const FGameplayEffectModCallbackData& InData, const AActor* InTarget)
{
	FVector HitPoint = InTarget->GetActorLocation();
	FVector HitNormal = FVector::ZeroVector;
	UPrimitiveComponent* Surface = Cast<UPrimitiveComponent>(InTarget->GetRootComponent());
	DC1HitHelper::FindHitLocation(InData.EffectSpec, InTarget, Surface, HitPoint, HitNormal);

	FGameplayCueParameters Params;
	Params.Location = HitPoint;
	Params.Normal = HitNormal;
	Params.RawMagnitude = static_cast<float>(GetHitElement(InData.EffectSpec));

	InData.Target.ExecuteGameplayCue(DC1EffectTags::GameplayCue_Hit_Element, Params);
}

void UDC1GameplayCueNotify_Hit::NotifyResourceHit(const FGameplayEffectModCallbackData& InData, const ADC1ResourceActor* InResource)
{
	const FGameplayTag WorkType = InResource->GetWorkTypeTag();

	FGameplayTag CueTag;
	if (true == WorkType.MatchesTagExact(DC1WorkTags::Work_Lumbering))
	{
		CueTag = DC1EffectTags::GameplayCue_Hit_Wood;
	}
	else if (true == WorkType.MatchesTagExact(DC1WorkTags::Work_Mining))
	{
		CueTag = DC1EffectTags::GameplayCue_Hit_Stone;
	}
	else
	{
		return;
	}

	// 데미지 숫자와 같은 위치 ( 공격자 쪽 표면 )
	FGameplayCueParameters Params;
	Params.Location = InResource->GetDropOrigin(&InData.EffectSpec);

	InData.Target.ExecuteGameplayCue(CueTag, Params);
}

void UDC1GameplayCueNotify_Hit::NotifyShieldHit(const FGameplayEffectModCallbackData& InData)
{
	const AActor* Target = InData.Target.GetAvatarActor();
	if (false == ::IsValid(Target))
	{
		return;
	}

	// 위치는 OnExecute 에서 대상에 부착하므로 참고용
	FGameplayCueParameters Params;
	Params.Location = Target->GetActorLocation();

	InData.Target.ExecuteGameplayCue(DC1EffectTags::GameplayCue_Hit_Shield, Params);
}

EDC1PalElement UDC1GameplayCueNotify_Hit::GetHitElement(const FGameplayEffectSpec& InSpec)
{
	const UGameplayAbility* SourceAbility = InSpec.GetContext().GetAbilityInstance_NotReplicated();
	if (nullptr == SourceAbility)
	{
		SourceAbility = InSpec.GetContext().GetAbility();
	}

	const UDC1GameplayAbility_PalSkill* PalSkill = Cast<UDC1GameplayAbility_PalSkill>(SourceAbility);
	return (nullptr != PalSkill) ? PalSkill->GetSkillElement() : EDC1PalElement::Neutral;
}

UNiagaraSystem* UDC1GameplayCueNotify_Hit::GetElementFX(EDC1PalElement InElement, bool InIsPlayer) const
{
	const TMap<EDC1PalElement, TObjectPtr<UNiagaraSystem>>& Map = (true == InIsPlayer) ? PlayerElementFX : ElementFX;
	const TObjectPtr<UNiagaraSystem>* Found = Map.Find(InElement);
	if (nullptr != Found && true == ::IsValid(*Found))
	{
		return *Found;
	}

	return (true == InIsPlayer) ? PlayerDefaultFX : DefaultFX;
}
