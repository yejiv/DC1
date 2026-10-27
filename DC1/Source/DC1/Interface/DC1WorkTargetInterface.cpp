// Fill out your copyright notice in the Description page of Project Settings.


#include "Interface/DC1WorkTargetInterface.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/State/DC1GameplayTag_State.h"

#include "AbilitySystemComponent.h"

float DC1WorkTarget::GetWorkerSpeed(const ADC1Character* InWorker, const FGameplayTag& InWorkType)
{
	if (false == ::IsValid(InWorker))
	{
		return 0.f;
	}

	// 기절은 속도만 0 - 작업자에서 빼지 않음 ( 복귀 )
	const UAbilitySystemComponent* ASC = InWorker->GetAbilitySystemComponent();
	if (false == ::IsValid(ASC) || true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Dead))
	{
		return 0.f;
	}

	float WorkSpeed = ASC->GetNumericAttribute(UDC1CharacterAttributeSet::GetWorkSpeedAttribute());

	// WorkSpeed 계산 - * 적성 레벨
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(InWorker);
	if (true == ::IsValid(Pal))
	{
		const UDC1PalData* PalData = Pal->GetPalData();
		if (true == ::IsValid(PalData))
		{
			WorkSpeed *= PalData->GetWorkSuitabilityLevel(InWorkType);
		}
	}

	return WorkSpeed;
}

bool DC1WorkTarget::CanPalWorkAt(const ADC1PalCharacter* InPal, const FGameplayTag& InWorkType, const FVector& InLocation)
{
	if (false == ::IsValid(InPal) || EDC1Team::Player != InPal->GetTeam())
	{
		return false;
	}

	if (false == InWorkType.IsValid())
	{
		return false;
	}

	const UDC1PalData* PalData = InPal->GetPalData();
	if (false == ::IsValid(PalData) || false == PalData->CanDoWork(InWorkType))
	{
		return false;
	}

	// 거점 배치 팰
	const ADC1PalBoxActor* PalBox = InPal->GetBasePalBox();
	if (true == ::IsValid(PalBox))
	{
		return PalBox->IsInBase(InLocation);
	}

	// 소환된 팰, TODO 거리 제한
	return true;
}
