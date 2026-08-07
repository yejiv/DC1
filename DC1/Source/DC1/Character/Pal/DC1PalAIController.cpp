// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalAIController.h"

#include "AI/DC1TeamTypes.h"

#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ADC1PalAIController::ADC1PalAIController()
	: Super()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
	
	//하드코딩! 하고나서 없앨 임의값들!
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 2000.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->SetMaxAge(5.f);
	SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = false;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = false;

	UAIPerceptionComponent* PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(SightConfig->GetSenseImplementation());
	SetPerceptionComponent(*PerceptionComp);
}

void ADC1PalAIController::BeginPlay()
{
	Super::BeginPlay();

	if (UAIPerceptionComponent* PerceptionComp = GetAIPerceptionComponent())
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &ThisClass::OnTargetPerceptionUpdated);
	}
}

ETeamAttitude::Type ADC1PalAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
	if (const IGenericTeamAgentInterface* PawnTeamAgent = Cast<const IGenericTeamAgentInterface>(GetPawn()))
	{
		return PawnTeamAgent->GetTeamAttitudeTowards(Other);
	}

	return ETeamAttitude::Neutral;
}

FGenericTeamId ADC1PalAIController::GetGenericTeamId() const
{
	if (const IGenericTeamAgentInterface* PawnTeamAgent = Cast<const IGenericTeamAgentInterface>(GetPawn()))
	{
		return PawnTeamAgent->GetGenericTeamId();
	}

	return FGenericTeamId::NoTeam;
}

void ADC1PalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	// 감지 결과는 CurrentTarget에만 담는다. 상태 선택은 StateTree가 이 값의 유효성으로 판단한다.
	if (true == Stimulus.WasSuccessfullySensed())
	{
		CurrentTarget = Actor;
	}
	else if (CurrentTarget == Actor)
	{
		CurrentTarget = nullptr;
	}

	UE_LOG(LogTemp, Warning, TEXT("[Pal] %s %s %s"), *GetNameSafe(GetPawn()), Stimulus.WasSuccessfullySensed() ? TEXT("SENSED") : TEXT("LOST"), *GetNameSafe(Actor));
}

void ADC1PalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UE_LOG(LogTemp, Warning, TEXT("[Pal] %s possessed by %s (Team=%s)"), *GetNameSafe(InPawn), *GetName(), *UEnum::GetValueAsString(DC1Team::FromGenericTeamId(GetGenericTeamId())));
}
