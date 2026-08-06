// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Pal/DC1PalAIController.h"
#include "Character/Pal/DC1PalAttributeSet.h"

#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionSystem.h"

ADC1PalCharacter::ADC1PalCharacter()
	: Super()
{
	AIControllerClass = ADC1PalAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	PalAttributeSet = CreateDefaultSubobject<UDC1PalAttributeSet>(TEXT("PalAttributeSet"));
}

void ADC1PalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADC1PalCharacter, Team);
}

ETeamAttitude::Type ADC1PalCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	return DC1Team::GetAttitude(Team, DC1Team::ResolveTeam(Other));
}

FGenericTeamId ADC1PalCharacter::GetGenericTeamId() const
{
	return DC1Team::ToGenericTeamId(Team);
}

void ADC1PalCharacter::SetTeam(EDC1Team NewTeam)
{
	if (false == HasAuthority())
	{
		return;
	}

	if (Team == NewTeam)
	{
		return;
	}

	Team = NewTeam;
	OnTeamChanged();
}

void ADC1PalCharacter::OnRep_Team()
{
	OnTeamChanged();
}

void ADC1PalCharacter::OnTeamChanged()
{
	// 적/아군 판정은 sight query 생성 시점에 한 번만 돈다.
	// 진영이 바뀌면 재등록해야 주변 Pal이 반응을 바꾼다.
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->RegisterSource(*this);
	}
}
