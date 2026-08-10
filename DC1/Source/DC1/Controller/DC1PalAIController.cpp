// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/DC1PalAIController.h"

#include "AI/DC1TeamTypes.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"

#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ADC1PalAIController::ADC1PalAIController()
	: Super()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));

	// 실제 값은 OnPossess에서 종별 PalData로 덮어씀
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

void ADC1PalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ApplyPerceptionSettings();

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s possessed by %s (Team=%s)"), *GetNameSafe(InPawn), *GetName(), *UEnum::GetValueAsString(DC1Team::FromGenericTeamId(GetGenericTeamId())));
}

void ADC1PalAIController::ApplyPerceptionSettings()
{
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetPawn());
	if (false == ::IsValid(Pal))
	{
		return;
	}

	const UDC1PalData* PalData = Pal->GetPalData();
	if (nullptr == PalData)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No PalData. Perception keeps constructor defaults."), *GetNameSafe(GetPawn()));
		return;
	}

	SightConfig->SightRadius = PalData->SightRadius;
	SightConfig->LoseSightRadius = PalData->LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = PalData->PeripheralVisionAngleDegrees;
	SightConfig->SetMaxAge(PalData->StimulusMaxAge);

	TargetMemoryDuration = PalData->TargetMemoryDuration;

	UAIPerceptionComponent* PerceptionComp = GetAIPerceptionComponent();
	if (nullptr == PerceptionComp)
	{
		return;
	}

	// 재설정 + 리스너 갱신해야 sight query에 반영됨
	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->RequestStimuliListenerUpdate();
}

void ADC1PalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (false == ::IsValid(Actor))
	{
		return;
	}

	// 감지 목록 갱신
	if (true == Stimulus.WasSuccessfullySensed())
	{
		SensedTargets.AddUnique(Actor);
	}
	else
	{
		SensedTargets.RemoveAll([Actor](const TWeakObjectPtr<AActor>& Weak)
			{
				return (Weak.Get() == Actor);
			});

		// 놓친 지점 남기기
		LastKnownTargetLocation = Stimulus.StimulusLocation;
	}

	// 누구를 쫓을지 정함
	UpdateTarget();
}

void ADC1PalAIController::UpdateTarget()
{
	const APawn* SelfPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (false == ::IsValid(SelfPawn) || nullptr == World)
	{
		CurrentTarget = nullptr;
		return;
	}

	SensedTargets.RemoveAll([](const TWeakObjectPtr<AActor>& Weak)
		{
			return (false == Weak.IsValid());
		});

	//  Health > 0 검사 추가 필요

	// 가장 가까운 대상 고르기
	const FVector SelfLocation = SelfPawn->GetActorLocation();

	AActor* Closest = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<AActor>& Weak : SensedTargets)
	{
		AActor* Candidate = Weak.Get();
		const float DistanceSq = FVector::DistSquared(SelfLocation, Candidate->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			Closest = Candidate;
		}
	}

	const float Now = World->GetTimeSeconds();

	if (nullptr != Closest)
	{
		CurrentTarget = Closest;
		LastKnownTargetLocation = Closest->GetActorLocation();
		LastSeenTime = Now;
		return;
	}

	// 지금 보이는 대상이 없어도 기억 시간 안이면 직전 대상을 유지한다.
	if (true == ::IsValid(CurrentTarget) && ((Now - LastSeenTime) < TargetMemoryDuration))
	{
		return;
	}

	CurrentTarget = nullptr;
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
