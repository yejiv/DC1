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

bool ADC1PalAIController::CanBackoff() const
{
	const UWorld* World = GetWorld();
	if (nullptr == World)
	{
		return true;
	}

	return ((World->GetTimeSeconds() - LastBackoffTime) >= BackoffCooldown);
}

void ADC1PalAIController::NotifyBackoffPerformed()
{
	if (const UWorld* World = GetWorld())
	{
		LastBackoffTime = World->GetTimeSeconds();
	}
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

	// 설 자리 — 주인 카메라 기준 로컬 오프셋을 월드로 편다.
	// X는 카메라 정면(멀어지는 방향), Y는 오른쪽. 두 값이 서로 얽히지 않는다.
	// 컨트롤 회전은 서버에도 복제되므로 화면 투영 없이 구한다
	static bool ChooseCameraOffset(const AActor& Owner, const FVector2D& LocalOffset, FVector& OutOffset)
	{
		const APawn* OwnerPawn = Cast<APawn>(&Owner);
		const AController* OwnerController = (nullptr != OwnerPawn) ? OwnerPawn->GetController() : nullptr;
		if (nullptr == OwnerController)
		{
			return false;
		}

		const FRotationMatrix YawMatrix(FRotator(0.f, OwnerController->GetControlRotation().Yaw, 0.f));

		OutOffset = (YawMatrix.GetUnitAxis(EAxis::X) * LocalOffset.X)
			+ (YawMatrix.GetUnitAxis(EAxis::Y) * LocalOffset.Y);

		return true;
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

	// 아직 자리를 정한 적이 없다. 0이면 Goal 이 주인 위치가 되어 몸에 붙는다.
	// 화면 밖 3초를 기다리지 않고 지금 바로 잡는다
	if (true == FollowAnchor.IsNearlyZero())
	{
		return DC1PalFollowAnchor::ChooseCameraOffset(*FollowTarget, FollowOffset, FollowAnchor);
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
	if (false == DC1PalFollowAnchor::ChooseCameraOffset(*FollowTarget, FollowOffset, NewAnchor))
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

		// 표적으로 삼은 것만으로는 부족하다. 야생 팰은 발견만 해도 주인을 표적에 넣는데,
		// 도발되기 전까지는 싸우지 않고 도망친다. 실제로 교전 중일 때만 참전한다
		if (FollowTarget == OtherController->GetCurrentTarget() && true == OtherController->IsProvoked())
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
