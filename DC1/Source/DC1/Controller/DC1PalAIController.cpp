// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/DC1PalAIController.h"

#include "AI/DC1TeamTypes.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Tags/State/DC1GameplayTag_State.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/StateTreeAIComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

namespace DC1PalAIController
{
	static bool IsDead(const AActor* Actor)
	{
		const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		return (nullptr != ASC) && ASC->HasMatchingGameplayTag(DC1StateTags::State_Dead);
	}
}

ADC1PalAIController::ADC1PalAIController()
	: Super()
{
	StateTreeComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeComponent"));
	StateTreeComponent->SetStartLogicAutomatically(false);

	// 실제 값은 OnPossess에서 종별 PalData로 덮어씀
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.f;
	SightConfig->LoseSightRadius = 2000.f;
	SightConfig->PeripheralVisionAngleDegrees = 70.f;
	SightConfig->SetMaxAge(5.f);

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

	TryStartBehavior();
}

void ADC1PalAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsValid(InPawn))
	{
		HomeLocation = InPawn->GetActorLocation();
	}

	ApplyPerceptionSettings();

	TryStartBehavior();

	UE_LOG(LogTemp, Log, TEXT("[Pal] %s possessed by %s (Team=%s)"), *GetNameSafe(InPawn), *GetName(), *UEnum::GetValueAsString(DC1Team::FromGenericTeamId(GetGenericTeamId())));
}

void ADC1PalAIController::ApplyPerceptionSettings()
{
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetPawn());
	if (false == IsValid(Pal))
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
	PerceptionComp->ConfigureSense(*SightConfig);	//config → digest
	PerceptionComp->RequestStimuliListenerUpdate();	//component → listener
}

void ADC1PalAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (false == IsValid(Actor))
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
	}

	// 누구를 쫓을지 정함
	UpdateTarget();
}

void ADC1PalAIController::UpdateTarget()
{
	const APawn* SelfPawn = GetPawn();
	const UWorld* World = GetWorld();
	if (false == IsValid(SelfPawn) || nullptr == World)
	{
		SetCurrentTarget(nullptr);
		return;
	}

	SensedTargets.RemoveAll([](const TWeakObjectPtr<AActor>& Weak)
		{
			return (false == Weak.IsValid());
		});

	// 가장 가까운 대상 고르기
	const FVector SelfLocation = SelfPawn->GetActorLocation();

	AActor* Closest = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();

	for (const TWeakObjectPtr<AActor>& Weak : SensedTargets)
	{
		AActor* Candidate = Weak.Get();

		if (true == DC1PalAIController::IsDead(Candidate))
		{
			continue;
		}

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
		SetCurrentTarget(Closest);
		LastSeenTime = Now;
		return;
	}

	if (true == IsValid(ProvokedTarget)
		&& false == DC1PalAIController::IsDead(ProvokedTarget)
		&& ((Now - LastSeenTime) < TargetMemoryDuration))
	{
		SetCurrentTarget(ProvokedTarget);
		return;
	}


	if (true == IsValid(CurrentTarget)
		&& false == DC1PalAIController::IsDead(CurrentTarget)
		&& ((Now - LastSeenTime) < TargetMemoryDuration))
	{
		return;
	}

	ProvokedTarget = nullptr;
	SetCurrentTarget(nullptr);
}

void ADC1PalAIController::ProvokeBy(AActor* InInstigator)
{
	if (false == IsValid(InInstigator))
	{
		return;
	}

	const UWorld* World = GetWorld();
	if (nullptr == World)
	{
		return;
	}

	ProvokedTarget = InInstigator;

	// 맞을 때마다 기억 시간이 초기화된다
	LastSeenTime = World->GetTimeSeconds();

	UpdateTarget();
}

void ADC1PalAIController::StopBehavior()
{
	// StateTree 정지
	if (IsValid(StateTreeComponent))
	{
		StateTreeComponent->StopLogic(TEXT("StopBehavior"));
	}

	// 타겟/포커스 해제
	SensedTargets.Reset();
	ProvokedTarget = nullptr;
	SetCurrentTarget(nullptr);
}

void ADC1PalAIController::TryStartBehavior()
{
	if (false == IsValid(GetPawn()) || false == HasActorBegunPlay())
	{
		return;
	}

	if (false == IsValid(StateTreeComponent))
	{
		return;
	}

	StateTreeComponent->StartLogic();
}

void ADC1PalAIController::SetCurrentTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget)
	{
		return;
	}

	CurrentTarget = NewTarget;

	if (true == IsValid(NewTarget))
	{
		SetFocus(NewTarget, EAIFocusPriority::Gameplay);
	}
	else
	{
		ClearFocus(EAIFocusPriority::Gameplay);
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
