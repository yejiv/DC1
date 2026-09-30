// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalBoxViewModel.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Engine/Texture2D.h"

namespace DC1PalBoxViewModel
{
	struct FResolvedPal
	{
		const FDC1PalInstance*		Pal = { nullptr };
		const UDC1PalData*			PalData = { nullptr };
		const UDC1PalRegistryData*	Registry = { nullptr };
	};

	// 슬롯 → 개체 + 종 데이터 + 레지스트리
	static bool Resolve(const UDC1PalPartyComponent* InParty, EDC1PalContainer InContainer, int32 InSlotIndex, FResolvedPal& OutResolved)
	{
		if (nullptr == InParty)
		{
			return false;
		}

		OutResolved.Pal = InParty->GetPalInfo(InContainer, InSlotIndex);
		if (nullptr == OutResolved.Pal)
		{
			return false;
		}

		OutResolved.Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
		if (false == IsValid(OutResolved.Registry))
		{
			return false;
		}

		OutResolved.PalData = OutResolved.Registry->FindPalDataByTag(OutResolved.Pal->SpeciesTag).LoadSynchronous();
		return IsValid(OutResolved.PalData);
	}

	static void FillSummary(const UDC1PalPartyComponent& InParty, EDC1PalContainer InContainer, int32 InSlotIndex, const FResolvedPal& InResolved, FDC1PalBoxSlotInfo& OutSlotInfo)
	{
		// 닉네임 없으면 종 이름
		OutSlotInfo.Name = InResolved.Pal->Nickname.IsEmpty() ? InResolved.PalData->DisplayName : FText::FromString(InResolved.Pal->Nickname);
		OutSlotInfo.Level = InResolved.Pal->Level;
		OutSlotInfo.Health = InParty.GetPalHealth(InContainer, InSlotIndex);
		OutSlotInfo.MaxHealth = InParty.GetPalMaxHealth(InContainer, InSlotIndex);
		OutSlotInfo.Icon = InResolved.PalData->Icon.LoadSynchronous();
	}
}

void UDC1PalBoxViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
	{
		return;
	}

	PalPartyComponent = InPlayerState->GetPalPartyComponent();
	if (false == PalPartyComponent.IsValid())
	{
		return;
	}

	PalPartyComponent->OnPalSlotChanged.AddUObject(this, &ThisClass::HandlePalChanged);
	PalPartyComponent->OnPalBoxChanged.AddUObject(this, &ThisClass::HandlePalChanged);

	HandlePalChanged();
}

void UDC1PalBoxViewModel::ClearSource()
{
	if (false == PalPartyComponent.IsValid())
	{
		return;
	}

	PalPartyComponent->OnPalSlotChanged.RemoveAll(this);
	PalPartyComponent->OnPalBoxChanged.RemoveAll(this);
	PalPartyComponent.Reset();

	CancelPick();
}

int32 UDC1PalBoxViewModel::GetCapacity() const
{
	return PalPartyComponent.IsValid() ? PalPartyComponent->GetCapacity() : 0;
}

int32 UDC1PalBoxViewModel::GetBoxCapacity() const
{
	return PalPartyComponent.IsValid() ? PalPartyComponent->GetBoxCapacity() : 0;
}

int32 UDC1PalBoxViewModel::GetBaseCapacity() const
{
	return PalPartyComponent.IsValid() ? PalPartyComponent->GetBaseCapacity() : 0;
}

bool UDC1PalBoxViewModel::HasBase() const
{
	return PalPartyComponent.IsValid() && PalPartyComponent->HasBase();
}

bool UDC1PalBoxViewModel::GetSlotInfo(EDC1PalContainer InContainer, int32 InSlotIndex, FDC1PalBoxSlotInfo& OutSlotInfo) const
{
	DC1PalBoxViewModel::FResolvedPal Resolved;
	if (false == DC1PalBoxViewModel::Resolve(PalPartyComponent.Get(), InContainer, InSlotIndex, Resolved))
	{
		return false;
	}

	DC1PalBoxViewModel::FillSummary(*PalPartyComponent, InContainer, InSlotIndex, Resolved, OutSlotInfo);
	return true;
}

bool UDC1PalBoxViewModel::GetDetailInfo(EDC1PalContainer InContainer, int32 InSlotIndex, FDC1PalBoxDetailInfo& OutDetailInfo) const
{
	DC1PalBoxViewModel::FResolvedPal Resolved;
	if (false == DC1PalBoxViewModel::Resolve(PalPartyComponent.Get(), InContainer, InSlotIndex, Resolved))
	{
		return false;
	}

	const FDC1PalInstance& Pal = *Resolved.Pal;
	const UDC1PalData& PalData = *Resolved.PalData;
	const UDC1PalRegistryData& Registry = *Resolved.Registry;

	DC1PalBoxViewModel::FillSummary(*PalPartyComponent, InContainer, InSlotIndex, Resolved, OutDetailInfo.Summary);

	OutDetailInfo.GenderIcon = Registry.FindGenderIcon(Pal.Gender).LoadSynchronous();
	OutDetailInfo.ElementIcon = Registry.FindElementIcon(PalData.Element).LoadSynchronous();
	OutDetailInfo.ElementIcon2 = (EDC1PalElement::Neutral == PalData.Element2) ? nullptr : Registry.FindElementIcon(PalData.Element2).LoadSynchronous();
	OutDetailInfo.Experience = Pal.Experience;
	OutDetailInfo.ExpToNextLevel = Registry.GetExpToNextLevel(Pal.Level);

	// 스탯
	float AttackMultiplier = 1.f;
	float DefenseMultiplier = 1.f;
	float WorkSpeedMultiplier = 1.f;
	Registry.GetPassiveMultipliers(Pal.PassiveTags, AttackMultiplier, DefenseMultiplier, WorkSpeedMultiplier);

	OutDetailInfo.Attack = Registry.GetAttackPowerFor(PalData, Pal.Level, Pal.TalentAttack) * AttackMultiplier;
	OutDetailInfo.Defense = Registry.GetDefensePowerFor(PalData, Pal.Level, Pal.TalentDefense) * DefenseMultiplier;
	OutDetailInfo.WorkSpeed = PalData.WorkSpeedStat * WorkSpeedMultiplier;

	// 작업 적성
	OutDetailInfo.Works.Reset();
	for (const FDC1PalWorkDef& WorkDef : Registry.WorkChart)
	{
		FDC1PalBoxWorkInfo& Work = OutDetailInfo.Works.AddDefaulted_GetRef();
		Work.Icon = WorkDef.Icon.LoadSynchronous();
		Work.Level = PalData.WorkSuitability.FindRef(WorkDef.WorkTag);
	}

	// 액티브 스킬
	OutDetailInfo.Skills.Reset();
	for (const FGameplayTag& SkillTag : PalData.SkillTags)
	{
		const FDC1PalSkillDef* SkillDef = Registry.FindSkillDef(SkillTag);
		if (nullptr == SkillDef)
		{
			continue;
		}

		FDC1PalBoxSkillInfo& Skill = OutDetailInfo.Skills.AddDefaulted_GetRef();
		Skill.Name = SkillDef->DisplayName;
		Skill.ElementIcon = Registry.FindElementIcon(SkillDef->Element).LoadSynchronous();
		Skill.Power = FMath::RoundToInt(SkillDef->Power * Registry.SkillPowerDisplayScale);
	}

	// 패시브
	OutDetailInfo.Passives.Reset();
	for (const FGameplayTag& PassiveTag : Pal.PassiveTags)
	{
		const FDC1PalPassiveDef* PassiveDef = Registry.FindPassiveDef(PassiveTag);
		if (nullptr == PassiveDef)
		{
			continue;
		}

		FDC1PalBoxPassiveInfo& Passive = OutDetailInfo.Passives.AddDefaulted_GetRef();
		Passive.Name = PassiveDef->DisplayName;
		Passive.RankIcon = Registry.FindPassiveRankIcon(PassiveDef->Rank).LoadSynchronous();
	}

	return true;
}

void UDC1PalBoxViewModel::BeginPick(EDC1PalContainer InContainer, int32 InSlotIndex)
{
	if (false == PalPartyComponent.IsValid() || nullptr == PalPartyComponent->GetPalInfo(InContainer, InSlotIndex))
	{
		return;
	}

	bPicking = true;
	PickContainer = InContainer;
	PickIndex = InSlotIndex;

	OnPickChanged.Broadcast();
}

void UDC1PalBoxViewModel::RequestDrop(EDC1PalContainer InContainer, int32 InSlotIndex)
{
	if (false == bPicking)
	{
		return;
	}

	if (true == PalPartyComponent.IsValid())
	{
		PalPartyComponent->Server_MovePal(PickContainer, PickIndex, InContainer, InSlotIndex);
	}

	CancelPick();
}

void UDC1PalBoxViewModel::CancelPick()
{
	if (false == bPicking)
	{
		return;
	}

	bPicking = false;
	PickIndex = INDEX_NONE;

	OnPickChanged.Broadcast();
}

void UDC1PalBoxViewModel::HandlePalChanged()
{
	OnPalsChanged.Broadcast();
}
