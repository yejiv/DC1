// Fill out your copyright notice in the Description page of Project Settings.

#include "Controller/DC1PalAIController.h"

#include "AI/DC1TeamTypes.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "AbilitySystem/Abilities/Pal/DC1GameplayAbility_PalSkill.h"
#include "Data/DA/Pal/DC1PalData.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "Tags/State/DC1GameplayTag_State.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/StateTreeAIComponent.h"
#include "GameFramework/PlayerController.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"
#include "StateTree.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

namespace DC1PalAIController
{
	static bool IsDead(const AActor* Actor)
	{
		const UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
		return (nullptr != ASC) && ASC->HasMatchingGameplayTag(DC1StateTags::State_Dead);
	}

	// 보임 + Alive
	static bool IsSensed(const TArray<TWeakObjectPtr<AActor>>& InSensedTargets, const AActor* InActor)
	{
		if (false == ::IsValid(InActor) || true == IsDead(InActor))
		{
			return false;
		}

		for (const TWeakObjectPtr<AActor>& Weak : InSensedTargets)
		{
			if (Weak.Get() == InActor)
			{
				return true;
			}
		}

		return false;
	}

	static const UDC1GameplayAbility_PalSkill* FindSkillByTag(const UAbilitySystemComponent& ASC, const FGameplayTag& SkillTag)
	{
		for (const FGameplayAbilitySpec& Spec : ASC.GetActivatableAbilities())
		{
			if (false == Spec.GetDynamicSpecSourceTags().HasTagExact(SkillTag))
			{
				continue;
			}

			if (const UDC1GameplayAbility_PalSkill* Skill = Cast<UDC1GameplayAbility_PalSkill>(Spec.Ability))
			{
				return Skill;
			}
		}

		return nullptr;
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
	ProvokeMemoryDuration = PalData->ProvokeMemoryDuration;

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

	const float Now = World->GetTimeSeconds();

	// 한 번 때린 상대를 다시 보면 도발이 되살아난다
	if (true == DC1PalAIController::IsSensed(SensedTargets, LastProvokedBy))
	{
		ProvokedTarget = LastProvokedBy;
	}

	// Player 적
	if (AActor* OwnerEnemy = GetFollowTargetEnemy())
	{
		SetCurrentTarget(OwnerEnemy);
		LastSeenTime = Now;
		return;
	}

	// 나를 떄린 적
	if (true == DC1PalAIController::IsSensed(SensedTargets, ProvokedTarget))
	{
		SetCurrentTarget(ProvokedTarget);
		LastSeenTime = Now;
		return;
	}

	// 잡고 있던 표적이 아직 보이면 안 바꾼다.
	if (true == DC1PalAIController::IsSensed(SensedTargets, CurrentTarget))
	{
		LastSeenTime = Now;
		return;
	}

	// 표적이 없을 때
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

	if (nullptr != Closest)
	{
		SetCurrentTarget(Closest);
		LastSeenTime = Now;
		return;
	}

	// ---- 아무것도 안 보인다. 기억으로 버틴다 ----

	if (true == IsValid(ProvokedTarget)
		&& false == DC1PalAIController::IsDead(ProvokedTarget)
		&& ((Now - LastSeenTime) < ProvokeMemoryDuration))
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

void ADC1PalAIController::UpdateSelectedSkill()
{
	if (true == SelectedSkillTag.IsValid())
	{
		return;
	}

	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetPawn());
	const UDC1PalData* PalData = IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData)
	{
		return;
	}

	const UAbilitySystemComponent* ASC = Pal->GetAbilitySystemComponent();
	if (nullptr == ASC)
	{
		return;
	}

	TArray<TPair<FGameplayTag, const UDC1GameplayAbility_PalSkill*>> Candidates;

	for (const FGameplayTag& SkillTag : PalData->SkillTags)
	{
		if (const UDC1GameplayAbility_PalSkill* Skill = DC1PalAIController::FindSkillByTag(*ASC, SkillTag))
		{
			Candidates.Emplace(SkillTag, Skill);
		}
	}

	if (0 == Candidates.Num())
	{
		return;
	}

	const int32 PickedIndex = FMath::RandRange(0, Candidates.Num() - 1);

	SelectedSkillTag = Candidates[PickedIndex].Key;
	SelectedMaxRange = Candidates[PickedIndex].Value->GetMaxRange();
	SelectedApproachRange = Candidates[PickedIndex].Value->GetApproachRange();
	SelectedMinRange = Candidates[PickedIndex].Value->GetMinRange();
}

void ADC1PalAIController::ClearSelectedSkill()
{
	SelectedSkillTag = FGameplayTag::EmptyTag;
	SelectedMaxRange = 0.f;
	SelectedApproachRange = 0.f;
	SelectedMinRange = 0.f;
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
	LastProvokedBy = InInstigator;

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

	ClearTargetState();
}

void ADC1PalAIController::ClearTargetState()
{
	SensedTargets.Reset();
	ProvokedTarget = nullptr;
	LastProvokedBy = nullptr;

	// SetCurrentTarget은 타겟이 이미 없으면 그냥 반환하므로 스킬은 따로 비운다
	ClearSelectedSkill();
	SetCurrentTarget(nullptr);
}

void ADC1PalAIController::RefreshPerception()
{
	// 리스너를 다시 등록해야 sight query의 적대 판정이 재평가된다
	ApplyPerceptionSettings();

	// 옛 진영 기준으로 쌓인 감지 결과와 원한은 버린다
	ClearTargetState();
}

void ADC1PalAIController::ApplyStateTreeForTeam()
{
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetPawn());
	if (false == IsValid(Pal) || false == IsValid(StateTreeComponent))
	{
		return;
	}

	UStateTree* Desired = (EDC1Team::Player == Pal->GetTeam()) ? PartnerStateTree : WildStateTree;

	// BP에 안 넣었으면 컴포넌트에 이미 물려 있는 트리를 그대로 쓴다
	if (nullptr == Desired)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: No state tree for team [%s]. Check the controller BP."),
			*GetNameSafe(Pal), *UEnum::GetValueAsString(Pal->GetTeam()));
		return;
	}

	StateTreeComponent->SetStateTree(Desired);
}

namespace DC1PalFollowAnchor
{
	// 자리 후보 방향. 주인이 향하는 쪽이 0도. 0/180도는 진행선을 막아 제외
	static constexpr float AnchorAngles[] = { 40.f, -40.f, 75.f, -75.f, 110.f, -110.f, 145.f, -145.f };

	// 앞쪽 자리 가중치. 1이면 이동 거리와 동등
	static constexpr float FrontBias = 1.5f;

	// 후보를 NavMesh로 당길 때 찾는 반경
	static constexpr float ProjectExtent = 300.f;

	// 후보를 놓을 반경. 실제 유지 거리는 태스크가 정하므로 방향만 얻으면 됨
	static constexpr float SampleRadius = 500.f;

	// 주인이 향하는 쪽. 서 있으면 보는 쪽
	static FVector GetOwnerHeading(const AActor& Owner)
	{
		const FVector Velocity = Owner.GetVelocity().GetSafeNormal2D();
		return (false == Velocity.IsNearlyZero()) ? Velocity : Owner.GetActorForwardVector().GetSafeNormal2D();
	}

	// 카메라를 알 수 없으면 보이는 것으로 봄
	static bool IsInOwnerView(const AActor& Owner, const FVector& Point, float ScreenMargin)
	{
		const APawn* OwnerPawn = Cast<APawn>(&Owner);
		const APlayerController* PlayerController = (nullptr != OwnerPawn) ? Cast<APlayerController>(OwnerPawn->GetController()) : nullptr;
		if (nullptr == PlayerController)
		{
			return true;
		}

		// 카메라 뒤면 실패
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		if (false == PlayerController->ProjectWorldLocationToScreen(Point, ScreenPosition))
		{
			return false;
		}

		int32 ViewportX = 0;
		int32 ViewportY = 0;
		PlayerController->GetViewportSize(ViewportX, ViewportY);
		if (ViewportX <= 0 || ViewportY <= 0)
		{
			return true;
		}

		// 가장자리에 걸친 건 제외
		const float InsetX = ViewportX * ScreenMargin;
		const float InsetY = ViewportY * ScreenMargin;

		return (ScreenPosition.X >= InsetX && ScreenPosition.X <= (ViewportX - InsetX)
			&& ScreenPosition.Y >= InsetY && ScreenPosition.Y <= (ViewportY - InsetY));
	}

	// 링에서 화면에 보이고 갈 수 있는 자리 선택. 가깝고 앞쪽인 곳 선호. 없으면 실패
	static bool ChooseVisible(const AActor& Owner, const FVector& PalLocation, float ScreenMargin, FVector& OutDirection)
	{
		UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(Owner.GetWorld());
		if (nullptr == NavSystem)
		{
			return false;
		}

		const FVector OwnerLocation = Owner.GetActorLocation();
		const FVector Heading = GetOwnerHeading(Owner);
		const FVector Extent(ProjectExtent);

		float BestScore = -1.f;

		for (const float Angle : AnchorAngles)
		{
			const FVector Direction = Heading.RotateAngleAxis(Angle, FVector::UpVector);
			const FVector Candidate = OwnerLocation + (Direction * SampleRadius);

			if (false == IsInOwnerView(Owner, Candidate, ScreenMargin))
			{
				continue;
			}

			FNavLocation Destination;
			if (false == NavSystem->ProjectPointToNavigation(Candidate, Destination, Extent))
			{
				continue;
			}

			// 둘 다 0~1
			const float TravelScore = 1.f - FMath::Clamp(FVector::Dist2D(PalLocation, Destination.Location) / (SampleRadius * 2.f), 0.f, 1.f);
			const float FrontScore = (FVector::DotProduct(Direction, Heading) + 1.f) * 0.5f;
			const float Score = TravelScore + (FrontScore * FrontBias);

			if (Score > BestScore)
			{
				BestScore = Score;
				OutDirection = Direction;
			}
		}

		return (BestScore >= 0.f);
	}
}

bool ADC1PalAIController::UpdateFollowAnchor(float DeltaTime)
{
	const APawn* Pal = GetPawn();
	const AActor* FollowTarget = GetFollowTarget();
	if (false == ::IsValid(Pal) || false == ::IsValid(FollowTarget))
	{
		return false;
	}

	const FVector PalLocation = Pal->GetActorLocation();

	if (true == FollowAnchor.IsNearlyZero())
	{
		FollowAnchor = (PalLocation - FollowTarget->GetActorLocation()).GetSafeNormal2D();
		if (true == FollowAnchor.IsNearlyZero())
		{
			FollowAnchor = -DC1PalFollowAnchor::GetOwnerHeading(*FollowTarget);
		}
	}

	if (true == DC1PalFollowAnchor::IsInOwnerView(*FollowTarget, PalLocation, ScreenMargin))
	{
		OutOfViewTime = 0.f;
		return false;
	}

	OutOfViewTime += DeltaTime;
	if (OutOfViewTime < OutOfViewDelay)
	{
		return false;
	}

	OutOfViewTime = 0.f;

	FVector NewAnchor = FollowAnchor;
	if (false == DC1PalFollowAnchor::ChooseVisible(*FollowTarget, PalLocation, ScreenMargin, NewAnchor))
	{
		return false;
	}

	FollowAnchor = NewAnchor;
	return true;
}

AActor* ADC1PalAIController::GetFollowTarget() const
{
	// 포획이 붙으면 팰이 들고 있는 주인 참조로 바꾼다. 지금은 플레이어 하나뿐이다
	return UGameplayStatics::GetPlayerPawn(this, 0);
}

AActor* ADC1PalAIController::GetFollowTargetEnemy() const
{
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(GetPawn());
	if (false == IsValid(Pal) || EDC1Team::Player != Pal->GetTeam())
	{
		return nullptr;
	}

	const AActor* FollowTarget = GetFollowTarget();
	if (false == IsValid(FollowTarget))
	{
		return nullptr;
	}

	// 내가 보고 있는 적만
	for (const TWeakObjectPtr<AActor>& Weak : SensedTargets)
	{
		AActor* Candidate = Weak.Get();

		const APawn* CandidatePawn = Cast<APawn>(Candidate);
		if (false == IsValid(CandidatePawn) || true == DC1PalAIController::IsDead(Candidate))
		{
			continue;
		}

		const ADC1PalAIController* OtherController = Cast<ADC1PalAIController>(CandidatePawn->GetController());
		if (nullptr == OtherController)
		{
			continue;
		}

		if (FollowTarget == OtherController->GetCurrentTarget())
		{
			return Candidate;
		}
	}

	return nullptr;
}

void ADC1PalAIController::HandleTeamChanged()
{
	// 진영이 바뀌면 트리가 통째로 달라진다. 멈추고 다시 시작한다
	if (IsValid(StateTreeComponent))
	{
		StateTreeComponent->StopLogic(TEXT("TeamChanged"));
	}

	RefreshPerception();

	TryStartBehavior();
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

	// 시작 직전에 진영에 맞는 트리를 물린다
	ApplyStateTreeForTeam();

	StateTreeComponent->StartLogic();
}

void ADC1PalAIController::SetCurrentTarget(AActor* NewTarget)
{
	if (CurrentTarget == NewTarget)
	{
		return;
	}

	CurrentTarget = NewTarget;

	ClearSelectedSkill();

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
