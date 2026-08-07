// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/DC1TeamTypes.h"

#include "GameFramework/Pawn.h"

ETeamAttitude::Type DC1Team::GetAttitude(EDC1Team InSelfTeam, EDC1Team InOtherTeam)
{
	if (EDC1Team::None == InSelfTeam || EDC1Team::None == InOtherTeam)
	{
		return ETeamAttitude::Neutral;
	}

	if (InSelfTeam != InOtherTeam)
	{
		return ETeamAttitude::Hostile;
	}

	// 야생끼리는 서로 공격하지 않지만 서로 돕지도 않는다.
	return (EDC1Team::Player == InSelfTeam) ? ETeamAttitude::Friendly : ETeamAttitude::Neutral;
}

EDC1Team DC1Team::ResolveTeam(const AActor& InActor)
{
	if (const IGenericTeamAgentInterface* TeamAgent = Cast<const IGenericTeamAgentInterface>(&InActor))
	{
		return FromGenericTeamId(TeamAgent->GetGenericTeamId());
	}

	// TODO : ADC1Character가 IGenericTeamAgentInterface를 구현하면 제거
	if (const APawn* Pawn = Cast<const APawn>(&InActor))
	{
		if (Pawn->IsPlayerControlled())
		{
			return EDC1Team::Player;
		}
	}

	return EDC1Team::None;
}
