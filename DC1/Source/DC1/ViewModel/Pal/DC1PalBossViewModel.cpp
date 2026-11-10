// Fill out your copyright notice in the Description page of Project Settings.

#include "ViewModel/Pal/DC1PalBossViewModel.h"

#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1PlayerState.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "AbilitySystemComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "TimerManager.h"

namespace DC1PalBossViewModel
{
	// 범위 확인 주기
	static constexpr float BossCheckInterval = 0.25f;
}

void UDC1PalBossViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	if (nullptr == InPlayerState)
		return;

	PlayerState = InPlayerState;

	if (UWorld* World = InPlayerState->GetWorld())
	{
		World->GetTimerManager().SetTimer(BossCheckTimer, FTimerDelegate::CreateUObject(this, &ThisClass::CheckBoss), DC1PalBossViewModel::BossCheckInterval, true);
	}

	CheckBoss();
}

void UDC1PalBossViewModel::ClearSource()
{
	if (true == PlayerState.IsValid())
	{
		if (UWorld* World = PlayerState->GetWorld())
		{
			World->GetTimerManager().ClearTimer(BossCheckTimer);
		}
	}

	SetBoss(nullptr);
	PlayerState.Reset();
}

UTexture2D* UDC1PalBossViewModel::GetElementIcon(int32 InIndex) const
{
	return (0 == InIndex) ? ElementIcon0.Get() : ElementIcon1.Get();
}

int32 UDC1PalBossViewModel::GetLevel() const
{
	return Boss.IsValid() ? Boss->GetLevel() : 0;
}

float UDC1PalBossViewModel::GetHealth() const
{
	const UAbilitySystemComponent* ASC = Boss.IsValid() ? Boss->GetAbilitySystemComponent() : nullptr;
	return (nullptr != ASC) ? ASC->GetNumericAttribute(UDC1AttributeSet::GetHealthAttribute()) : 0.f;
}

float UDC1PalBossViewModel::GetMaxHealth() const
{
	const UAbilitySystemComponent* ASC = Boss.IsValid() ? Boss->GetAbilitySystemComponent() : nullptr;
	return (nullptr != ASC) ? ASC->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute()) : 0.f;
}

void UDC1PalBossViewModel::CheckBoss()
{
	// 탑승 중이면 탄 팰 위치 ( PlayerState 의 폰 )
	const APawn* Pawn = PlayerState.IsValid() ? PlayerState->GetPawn() : nullptr;
	UWorld* World = PlayerState.IsValid() ? PlayerState->GetWorld() : nullptr;

	ADC1PalCharacter* Nearest = nullptr;

	if (nullptr != Pawn && nullptr != World)
	{
		const FVector Origin = Pawn->GetActorLocation();
		float NearestDistSq = TNumericLimits<float>::Max();

		for (TActorIterator<ADC1PalCharacter> It(World); It; ++It)
		{
			ADC1PalCharacter* Pal = *It;

			// 잡은 객체
			if (false == Pal->IsAlpha() || EDC1Team::Wild != Pal->GetTeam() || true == Pal->IsHidden() || Pal->GetHealthRatio() <= 0.f)
				continue;

			const UDC1PalData* PalData = Pal->GetPalData();
			if (nullptr == PalData)
				continue;

			const float DistSq = FVector::DistSquared(Origin, Pal->GetActorLocation());
			if (DistSq > FMath::Square(PalData->AlphaBarRadius) || DistSq >= NearestDistSq)
				continue;

			Nearest = Pal;
			NearestDistSq = DistSq;
		}
	}

	if (Nearest != Boss.Get())
	{
		SetBoss(Nearest);
	}
}

void UDC1PalBossViewModel::SetBoss(ADC1PalCharacter* InBoss)
{
	if (UAbilitySystemComponent* OldASC = Boss.IsValid() ? Boss->GetAbilitySystemComponent() : nullptr)
	{
		OldASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).RemoveAll(this);
		OldASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetMaxHealthAttribute()).RemoveAll(this);
		OldASC->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetLevelAttribute()).RemoveAll(this);
	}

	Boss = InBoss;
	Title = FText::GetEmpty();
	Name = FText::GetEmpty();
	ElementIcon0 = nullptr;
	ElementIcon1 = nullptr;

	const UDC1PalData* PalData = (nullptr != InBoss) ? InBoss->GetPalData() : nullptr;
	if (nullptr != PalData)
	{
		Title = PalData->AlphaTitle;
		Name = PalData->DisplayName;

		if (const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry))
		{
			ElementIcon0 = Registry->FindElementIcon(PalData->Element).LoadSynchronous();

			// 두 번째 속성이 없으면 Neutral
			if (EDC1PalElement::Neutral != PalData->Element2)
			{
				ElementIcon1 = Registry->FindElementIcon(PalData->Element2).LoadSynchronous();
			}
		}
	}

	if (UAbilitySystemComponent* NewASC = (nullptr != InBoss) ? InBoss->GetAbilitySystemComponent() : nullptr)
	{
		NewASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleStatChanged);
		NewASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetMaxHealthAttribute()).AddUObject(this, &ThisClass::HandleStatChanged);
		NewASC->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetLevelAttribute()).AddUObject(this, &ThisClass::HandleStatChanged);
	}

	OnBossChanged.Broadcast();
}

void UDC1PalBossViewModel::HandleStatChanged(const FOnAttributeChangeData& Data)
{
	OnBossStatChanged.Broadcast();
}
