// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "DC1TeamTypes.generated.h"

UENUM(BlueprintType)
enum class EDC1Team : uint8
{
	None	UMETA(DisplayName = "None"),
	Player	UMETA(DisplayName = "Player"),
	Wild	UMETA(DisplayName = "Wild"),
};

namespace DC1Team
{
	ETeamAttitude::Type GetAttitude(EDC1Team InSelfTeam, EDC1Team InOtherTeam);
	EDC1Team ResolveTeam(const AActor& InActor);

	FORCEINLINE FGenericTeamId ToGenericTeamId(EDC1Team InTeam)
	{
		return (EDC1Team::None == InTeam)
			? FGenericTeamId::NoTeam
			: FGenericTeamId(static_cast<uint8>(InTeam));
	}

	FORCEINLINE EDC1Team FromGenericTeamId(const FGenericTeamId& InTeamId)
	{
		const uint8 Id = InTeamId.GetId();
		return (Id <= static_cast<uint8>(EDC1Team::Wild))
			? static_cast<EDC1Team>(Id)
			: EDC1Team::None;
	}
}
