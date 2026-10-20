// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DC1CharacterMovementComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"

#include "Character/Player/DC1Player.h"

#pragma region SavedMove
FSavedMove_DC1::FSavedMove_DC1()
	: Super()
{
}

void FSavedMove_DC1::Clear()
{
	Super::Clear();

	Saved_bWantsToWalk = false;
	Saved_bWantsToSprint = false;
	Saved_bWantsToGlide = false;
}

uint8 FSavedMove_DC1::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	if (Saved_bWantsToWalk) Result |= WALK;
	if (Saved_bWantsToSprint) Result |= SPRINT;
	if (Saved_bWantsToGlide) Result |= GLIDE;
	return Result;
}

bool FSavedMove_DC1::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	if (GetCompressedFlags() != NewMove->GetCompressedFlags()) { return false; }

	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void FSavedMove_DC1::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(C->GetCharacterMovement());

	if (nullptr != CMC)
	{
		Saved_bWantsToWalk = CMC->Safe_bWantsToWalk;
		Saved_bWantsToSprint = CMC->Safe_bWantsToSprint;
		Saved_bWantsToGlide = CMC->Safe_bWantsToGlide;
	}
}

void FSavedMove_DC1::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(C->GetCharacterMovement());

	if (nullptr != CMC)
	{
		CMC->Safe_bWantsToWalk = Saved_bWantsToWalk;
		CMC->Safe_bWantsToSprint = Saved_bWantsToSprint;
		CMC->Safe_bWantsToGlide = Saved_bWantsToGlide;
	}
}
#pragma endregion

#pragma region NetworkPredictionData
FNetworkPredictionData_Client_DC1::FNetworkPredictionData_Client_DC1(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_DC1::AllocateNewMove()
{
	return MakeShared<FSavedMove_DC1>();
}
#pragma endregion

UDC1CharacterMovementComponent::UDC1CharacterMovementComponent()
	: Super()
{
}

float UDC1CharacterMovementComponent::GetMaxSpeed() const
{
	if (IsClimbing())
	{
		return ClimbSpeed;
	}

	if (MOVE_Walking != MovementMode && MOVE_NavWalking != MovementMode && MOVE_Falling != MovementMode)
	{
		return Super::GetMaxSpeed();
	}

	float Speed = {};

	if (Safe_bWantsToSprint)
	{
		Speed = SprintSpeed;
	}
	else
	{
		Speed = Safe_bWantsToWalk ? WalkSpeed : JogSpeed;
	}

	return Speed;
}

FNetworkPredictionData_Client* UDC1CharacterMovementComponent::GetPredictionData_Client() const
{
	if (nullptr == PawnOwner) { return nullptr; }

	if (nullptr == ClientPredictionData)
	{
		UDC1CharacterMovementComponent* CMC = const_cast<UDC1CharacterMovementComponent*>(this);

		CMC->ClientPredictionData = new FNetworkPredictionData_Client_DC1(*this);
		CMC->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		CMC->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UDC1CharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	Safe_bWantsToWalk = (0 != (Flags & FSavedMove_DC1::EMoveFlag::WALK));
	Safe_bWantsToSprint = (0 != (Flags & FSavedMove_DC1::EMoveFlag::SPRINT));
	Safe_bWantsToGlide = (0 != (Flags & FSavedMove_DC1::EMoveFlag::GLIDE));
}

void UDC1CharacterMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);

	// SimulatedProxy인 경우 Movement 계산 생략
	if (ENetRole::ROLE_SimulatedProxy == GetOwnerRole())
		return;

	ConfirmGlideMovement();
	ConfirmClimbMovement();
}

void UDC1CharacterMovementComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	if (ENetRole::ROLE_SimulatedProxy == GetOwnerRole())
	{
		FHitResult Hit = {};
		SafeMoveUpdatedComponent(Velocity * deltaTime, UpdatedComponent->GetComponentQuat(), true, Hit);
		return;
	}

	switch (CustomMovementMode)
	{
	case ECustomMovementMode::CMOVE_Climb:
		PhysClimbing(deltaTime, Iterations);
		break;
	case ECustomMovementMode::CMOVE_Glide:
		PhysGliding(deltaTime, Iterations);
		break;
	}
}

void UDC1CharacterMovementComponent::PhysClimbing(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
		return;

	float remainingTime = deltaTime;

	while (remainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations)
	{
		ResetClimbState();

		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// ① 벽 재탐지 (DetectWall)
		if (false == DetectWall())
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		// ② 꼭대기 확인
		//    꼭대기면 위쪽 이동 차단 + 도착 표시만 (GA_ClimbAtTop 활성화는 Phys 밖에서, 리플레이 중복 방지)
		if (false == HasWallAtHeadHeight())
		{
			bArriveAtTop = true;
			UpdateCanMantle();
		}

		// ③ 속도 계산
		//    Acceleration을 벽 평면에 투영(VectorPlaneProject) → CalcVelocity(timeTick, ...)
		Acceleration = FVector::VectorPlaneProject(Acceleration, ClimbWallNormal);
		CalcVelocity(timeTick, ClimbFriction, false, BrakingDecelerationClimbing);
		Velocity = FVector::VectorPlaneProject(Velocity, ClimbWallNormal);
		const FVector WallUp = FVector::VectorPlaneProject(FVector::UpVector, ClimbWallNormal).GetSafeNormal();
		const float UpSpeed = FVector::DotProduct(Velocity, WallUp);
		if (bArriveAtTop && UpSpeed > 0.f)
		{
			Velocity -= WallUp * UpSpeed;
		}

		// ④ 이동
		//    SafeMoveUpdatedComponent(Velocity * timeTick, ...), 막히면 SlideAlongSurface
		bJustTeleported = false;
		const FVector OldLocation = UpdatedComponent->GetComponentLocation();
		const FVector Delta = Velocity * timeTick;
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);
		const FHitResult MoveHit = Hit;

		if (MoveHit.Time < 1.f)
		{
			SlideAlongSurface(Delta, 1.f - MoveHit.Time, MoveHit.Normal, Hit, false);
		}

		// ⑤ 벽에 붙이기
		//    벽과의 거리를 일정하게 유지(-ClimbWallNormal 방향으로 보간)
		AttachToWall();

		// ⑥ 회전
		//    -ClimbWallNormal을 바라보도록 보간 (캡슐은 Yaw만)
		RotateToWall(timeTick);

		// ⑦ 실제 속도 재계산
		//    Velocity = (현재 위치 - 이동 전 위치) / timeTick
		if (false == bJustTeleported)
		{
			Velocity = (UpdatedComponent->GetComponentLocation() - OldLocation) / timeTick;
			// ⑤에서 벽 쪽으로 당긴 성분은 빼서 벽 평면 속도만 남긴다(애니메이션 블렌드 입력)
			Velocity = FVector::VectorPlaneProject(Velocity, ClimbWallNormal);
		}

		// ⑧ 바닥 확인
		//    걸을 수 있는 바닥 도착 → SetMovementMode(MOVE_Walking)
		//                         → StartNewPhysics(remainingTime, Iterations) 후 return (이번 단계 이동은 이미 함)
		//    내려가는 중에 처음 이동(MoveHit)이나 미끄러진 이동(Hit)이 걸을 수 있는 면에 막혔으면 바닥 도착
		if (Delta.Z < 0.f && (true == IsWalkable(MoveHit) || true == IsWalkable(Hit)))
		{
			SetMovementMode(MOVE_Walking);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
	}
}

void UDC1CharacterMovementComponent::PhysGliding(float deltaTime, int32 Iterations)
{
	if (deltaTime < MIN_TICK_TIME)
		return;

	float remainingTime = deltaTime;

	while (remainingTime >= MIN_TICK_TIME && Iterations < MaxSimulationIterations)
	{
		Iterations++;
		const float timeTick = GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime -= timeTick;

		// Glide Off
		if (false == Safe_bWantsToGlide)
		{
			SetMovementMode(MOVE_Falling);
			StartNewPhysics(remainingTime + timeTick, Iterations - 1);
			return;
		}

		// Calc Velocity
		Velocity.Z = 0.f;
		CalcVelocity(timeTick, GlideFriction, false, BrakingDecelerationGliding);
		Velocity.Z = -GlideFallSpeed;

		const FVector Delta = Velocity * timeTick;
		FHitResult Hit = {};
		SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

		FHitResult MoveHit = Hit;
		if (MoveHit.Time < 1.f)
		{
			SlideAlongSurface(Delta, 1.f - MoveHit.Time, MoveHit.Normal, MoveHit, false);
		}

		// 바닥에 닿으면 Glide 종료
		if (Delta.Z < 0.f && true == IsWalkable(Hit))
		{
			SetMovementMode(MOVE_Walking);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}
	}
}

void UDC1CharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (true == IsGliding())
	{
		Velocity.Z = 0.f;
	}
	else
	{
		Safe_bWantsToGlide = false;
	}

	ResetClimbState();

	// State.Climbing 태그 등 게임플레이 쪽 반응은 ADC1Player::OnMovementModeChanged가 맡는다(Super에서 호출됨)
	if (IsClimbing())
	{
		Velocity = FVector::ZeroVector;
	}
}

void UDC1CharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	// 벽타기 중엔 bOrientRotationToMovement가 속도 방향(위쪽)으로 돌려 RotateToWall 결과를 덮어쓰므로 건너뛴다
	if (true == IsClimbing() || true == CurrentRootMotion.HasOverrideVelocity())
	{
		return;
	}

	Super::PhysicsRotation(DeltaTime);
}

void UDC1CharacterMovementComponent::ForceRotation(const FRotator& InRotation, bool InbAlsoControlRotation)
{
	if (nullptr == CharacterOwner || false == CharacterOwner->HasAuthority())
		return;

	// 캐릭터는 Yaw만
	const FRotator YawOnly(0.f, InRotation.Yaw, 0.f);

	// 서버
	CharacterOwner->SetActorRotation(YawOnly);

	// 카메라
	if (true == InbAlsoControlRotation)
	{
		if (APlayerController* PC = Cast<APlayerController>(CharacterOwner->GetController()))
		{
			PC->ClientSetRotation(YawOnly);
		}
	}

	// 원격 클라만 보정이 필요하다 ( 리슨 서버 호스트는 위에서 끝 )
	if (true == CharacterOwner->IsLocallyControlled())
		return;

	// 다음 보정에 서버 회전 포함 + 오차가 없어도 보정을 보낸다 ( 제자리 회전 대응 )
	bForceRotationCorrection = true;

	if (FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character())
	{
		ServerData->bForceClientUpdate = true;
	}
}

bool UDC1CharacterMovementComponent::ShouldCorrectRotation() const
{
	return true == bForceRotationCorrection || Super::ShouldCorrectRotation();
}

void UDC1CharacterMovementComponent::ServerMoveHandleClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLocation, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName ClientBaseBoneName, uint8 ClientMovementMode)
{
	Super::ServerMoveHandleClientError(ClientTimeStamp, DeltaTime, Accel, RelativeClientLocation, ClientMovementBaseInterfaceData, ClientBaseBoneName, ClientMovementMode);

	if (false == bForceRotationCorrection)
		return;

	// 클라 이동이 서버와 맞으면 = 클라가 보정을 받고 따라잡음 → 종료
	const FNetworkPredictionData_Server_Character* ServerData = GetPredictionData_Server_Character();
	if (nullptr != ServerData && true == ServerData->PendingAdjustment.bAckGoodMove)
	{
		bForceRotationCorrection = false;
	}
}

bool UDC1CharacterMovementComponent::CanAttemptJump() const
{
	bool CanJump = Super::CanAttemptJump();

	if (IsClimbing())
		CanJump = true;

	return CanJump;
}

bool UDC1CharacterMovementComponent::IsWeaponBlockedByMovement() const
{
	return IsGliding() || IsClimbing();
}

void UDC1CharacterMovementComponent::ToggleMove()
{
	Safe_bWantsToWalk = ~Safe_bWantsToWalk;
}

void UDC1CharacterMovementComponent::SetWalk(bool IsWalk)
{
	Safe_bWantsToWalk = IsWalk;
}

void UDC1CharacterMovementComponent::SetSprint(bool Sprint)
{
	Safe_bWantsToSprint = Sprint;
}

void UDC1CharacterMovementComponent::SetGlide(bool IsGlide)
{
	Safe_bWantsToGlide = IsGlide;
}

void UDC1CharacterMovementComponent::ConfirmGlideMovement()
{
	if (ADC1Player* OwnerPlayer = Cast<ADC1Player>(GetOwner()))
	{
		if (true == IsGliding() && false == OwnerPlayer->CanGlide())
		{
			SetMovementMode(MOVE_Falling);
		}

		if (true == IsFalling() && true == Safe_bWantsToGlide && true == OwnerPlayer->CanGlide())
		{
			SetMovementMode(MOVE_Custom, CMOVE_Glide);
		}
	}
}

void UDC1CharacterMovementComponent::ConfirmClimbMovement()
{
	ResetClimbState();

	ADC1Player* OwnerPlayer = Cast<ADC1Player>(GetOwner());
	if (false == IsValid(OwnerPlayer))
		return;

	if (true == IsClimbing() && false == OwnerPlayer->CheckRemainStamina(0.f))
	{
		SetMovementMode(MOVE_Falling);
		return;
	}

	// 벽을 놓은 경우 W키를 다시 뗐다가 누를 때까지 벽타기 제한
	if (bReleaseClimb)
	{
		if (true == Acceleration.IsNearlyZero())
		{
			bReleaseClimb = false;
		}
		else
		{
			return;
		}
	}

	if (MOVE_Falling != MovementMode && false == IsGliding())
	{
		return;
	}

	if (IsFalling() && Velocity.Z > ReattachZThreshold)
	{
		return;
	}

	// 진입 판정
	if (true == Acceleration.IsNearlyZero())
		return;

	if (true == DetectWall() && true == CheckInputDirectionToWall())
	{
		if (true == HasWallAtHeadHeight())
		{
			if (true == OwnerPlayer->CanClimb())
			{
				SetMovementMode(MOVE_Custom, CMOVE_Climb);
			}
		}
		else
		{
			UpdateCanMantle();
		}
	}
}

bool UDC1CharacterMovementComponent::HasWallAtHeadHeight()
{
	if (nullptr == CharacterOwner || nullptr == UpdatedComponent)
	{
		return false;
	}

	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	const FVector Forward = -ClimbWallNormal;
	const FVector Start = UpdatedComponent->GetComponentLocation() + FVector::UpVector * CapsuleHalfHeight;
	const FVector End = Start + Forward * DetectWallRange * 1.5f;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClimbHeadTrace));
	FCollisionObjectQueryParams ObjectParams;
	SetupClimbQueryParams(QueryParams, ObjectParams);

	FHitResult Hit;
	GetWorld()->LineTraceSingleByObjectType(Hit, Start, End, ObjectParams, QueryParams);

	return Hit.bBlockingHit;
}

bool UDC1CharacterMovementComponent::DetectWall()
{
	ClimbWallNormal = FVector::ZeroVector;

	if (nullptr == CharacterOwner || nullptr == UpdatedComponent)
	{
		return false;
	}

	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	// 본 캡슐보다 작은 캡슐로 앞쪽을 훑는다
	// 반지름을 줄여 이미 붙어 있는 벽에서 시작 관통(bStartPenetrating)을 피하고, 높이를 줄여 바닥/천장에 걸리지 않게 한다
	const FCollisionShape SweepShape = FCollisionShape::MakeCapsule(CapsuleRadius * 0.8f, CapsuleHalfHeight * 0.6f);

	const FVector Forward = UpdatedComponent->GetForwardVector();
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + Forward * DetectWallRange;

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClimbWallSweep));
	FCollisionObjectQueryParams ObjectParams;
	SetupClimbQueryParams(QueryParams, ObjectParams);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, Start, End, UpdatedComponent->GetComponentQuat(), ObjectParams, SweepShape, QueryParams);

	// 벽 각도 범위: Walkable 각도 초과 ~ ClimbMaxAngle 이하 (법선 Z 기준)
	const float MaxWallZ = GetWalkableFloorZ();
	const float MinWallZ = FMath::Cos(FMath::DegreesToRadians(ClimbMaxAngle));

	FVector NormalSum = FVector::ZeroVector;
	int32 ValidHitCount = {};

	for (const FHitResult& Hit : Hits)
	{
		const FVector Normal = Hit.ImpactNormal;

		// 걸을 수 있는 바닥 / 너무 젖혀진 오버행 제외
		if (Normal.Z >= MaxWallZ || Normal.Z < MinWallZ)
		{
			continue;
		}

		// 정면을 향한 면만(옆면·등 뒤 면 제외)
		if (FVector::DotProduct(Normal, -Forward) <= 0.f)
		{
			continue;
		}

		NormalSum += Normal;
		++ValidHitCount;
	}

	if (0 < ValidHitCount)
	{
		ClimbWallNormal = NormalSum.GetSafeNormal();
	}

	if (ClimbWallNormal == FVector::ZeroVector)
	{
		return false;
	}

	return true;
}

bool UDC1CharacterMovementComponent::CheckInputDirectionToWall()
{
	if (FVector::DotProduct(-ClimbWallNormal.GetSafeNormal2D(), Acceleration.GetSafeNormal2D()) <= cos(FMath::DegreesToRadians(ForwardDegree)))
	{
		return false;
	}

	return true;
}

void UDC1CharacterMovementComponent::UpdateCanMantle()
{
	bCanMantle = false;

	// bCanMantle은 OnMove(소유 클라)의 입력 판단에만 쓰인다.
	// 착지 위치는 GA가 활성화될 때 FindMantleTarget으로 직접 구하므로 여기선 가능 여부만 남긴다
	if (nullptr == CharacterOwner || false == CharacterOwner->IsLocallyControlled())
		return;

	FVector UnusedTargetLocation;
	bCanMantle = FindMantleTarget(UnusedTargetLocation);
}

bool UDC1CharacterMovementComponent::CanGliding() const
{
	ADC1Player* Player = Cast<ADC1Player>(CharacterOwner);
	if (false == IsValid(Player))
		return false;

	return Player->CanGlide();
}

bool UDC1CharacterMovementComponent::FindMantleTarget(FVector& OutTargetLocation) const
{
	OutTargetLocation = FVector::ZeroVector;

	if (nullptr == CharacterOwner || nullptr == UpdatedComponent || true == ClimbWallNormal.IsZero())
		return false;

	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	const FVector Location = UpdatedComponent->GetComponentLocation();
	const float FeetZ = Location.Z - CapsuleHalfHeight;

	// 착지 지점은 벽 안쪽 수평 방향으로 찾는다(기울어진 벽이어도 올라선 뒤엔 수평으로 서야 하므로)
	const FVector IntoWall = -ClimbWallNormal.GetSafeNormal2D();

	// 꼭대기 탐색 높이 범위: 계단 오르기 높이 위 ~ 머리 위 여유
	// 계단 높이 이하의 턱은 걸어서 넘으므로 제외한다
	const float MantleMinZ = FeetZ + MaxStepHeight;
	const float MantleMaxZ = Location.Z + CapsuleHalfHeight + MantleTraceHeadMargin;

	FCollisionObjectQueryParams ObjectParams;

	// 1) 벽까지의 수평 거리: 꼭대기 탐색 최저 높이에서 벽 안쪽으로 쏜다
	//    유효한 꼭대기라면 이 높이엔 반드시 벽면이 있다
	FCollisionQueryParams WallQueryParams(SCENE_QUERY_STAT(ClimbMantleWallTrace));
	SetupClimbQueryParams(WallQueryParams, ObjectParams);

	const FVector WallTraceStart = FVector(Location.X, Location.Y, MantleMinZ);
	const FVector WallTraceEnd = WallTraceStart + IntoWall * (DetectWallRange + CapsuleRadius);

	FHitResult WallHit;
	if (false == GetWorld()->LineTraceSingleByObjectType(WallHit, WallTraceStart, WallTraceEnd, ObjectParams, WallQueryParams))
	{
		return false;
	}
	// 걸어서 넘을 수 있는 지형은 벽으로 판정하지 않는다.
	if (WallHit.ImpactNormal.Z >= GetWalkableFloorZ())
	{
		return false;
	}

	const float WallDistance = FVector::Dist2D(WallTraceStart, WallHit.ImpactPoint);

	// 2) 꼭대기 바닥: 벽면에서 캡슐 반지름 + 여유만큼 안쪽 지점을 위에서 아래로 쏜다
	//    모서리에 걸치지 않고 캡슐이 온전히 올라설 위치를 겨눈다
	const FVector LandingXY = Location + IntoWall * (WallDistance + CapsuleRadius + MantleLandingInset);
	const FVector FloorTraceStart = FVector(LandingXY.X, LandingXY.Y, MantleMaxZ);
	const FVector FloorTraceEnd = FVector(LandingXY.X, LandingXY.Y, MantleMinZ);

	FCollisionQueryParams FloorQueryParams(SCENE_QUERY_STAT(ClimbMantleFloorTrace));
	SetupClimbQueryParams(FloorQueryParams, ObjectParams);

	FHitResult FloorHit;
	if (false == GetWorld()->LineTraceSingleByObjectType(FloorHit, FloorTraceStart, FloorTraceEnd, ObjectParams, FloorQueryParams))
	{
		return false;
	}

	// 걸을 수 있는 바닥이어야 올라선다
	if (FloorHit.ImpactNormal.Z < GetWalkableFloorZ())
	{
		return false;
	}

	// 3) 착지 공간: 바닥 위에 캡슐을 살짝 띄워 겹침 검사(붙이면 바닥과 닿아 항상 겹침)
	//    시작점이 처마 등 물체 안이라 바닥 Ray가 오판한 경우도 여기서 걸러진다
	const FVector CapsuleCenter = FloorHit.ImpactPoint + FVector::UpVector * (CapsuleHalfHeight + MantleOverlapLift);
	const FCollisionShape LandingShape = FCollisionShape::MakeCapsule(CapsuleRadius * MantleOverlapRadiusScale, CapsuleHalfHeight);

	FCollisionQueryParams OverlapQueryParams(SCENE_QUERY_STAT(ClimbMantleOverlap));
	SetupClimbQueryParams(OverlapQueryParams, ObjectParams);

	if (true == GetWorld()->OverlapAnyTestByObjectType(CapsuleCenter, FQuat::Identity, ObjectParams, LandingShape, OverlapQueryParams))
	{
		return false;
	}

	OutTargetLocation = CapsuleCenter;
	return true;
}

void UDC1CharacterMovementComponent::AttachToWall()
{
	if (nullptr == CharacterOwner || nullptr == UpdatedComponent || true == ClimbWallNormal.IsZero())
	{
		return;
	}

	const UCapsuleComponent* Capsule = CharacterOwner->GetCapsuleComponent();
	const float CapsuleRadius = Capsule->GetScaledCapsuleRadius();
	const float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	// 1) 캡슐 중심에서 벽 법선 반대 방향으로 벽면까지 거리 측정
	const FVector IntoWall = -ClimbWallNormal;
	const FVector Start = UpdatedComponent->GetComponentLocation();
	const FVector End = Start + IntoWall * (CapsuleRadius + DetectWallRange);

	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ClimbAttachTrace));
	FCollisionObjectQueryParams ObjectParams;
	SetupClimbQueryParams(QueryParams, ObjectParams);

	FHitResult WallHit;
	if (false == GetWorld()->LineTraceSingleByObjectType(WallHit, Start, End, ObjectParams, QueryParams))
	{
		// 벽 상실은 다음 서브스텝 ①(DetectWall)에서 처리
		return;
	}

	// 2) 목표 거리(캡슐 표면이 벽에서 ClimbWallGap만큼 떨어진 위치)와의 오차
	//    캡슐은 항상 서 있으므로 기울어진 벽에선 위/아래 반구가 중심보다 벽에 가깝다
	//    → 반구 중심(세로 선분 끝)이 벽 쪽으로 들어가는 만큼(HalfHeight - Radius) * |Normal.Z| 를 더한다
	//    양수면 멀어서 당기고, 음수면 가까워서 밀어낸다
	const float SegmentHalfLength = FMath::Max(0.f, CapsuleHalfHeight - CapsuleRadius);
	const float TargetDistance = CapsuleRadius + ClimbWallGap + SegmentHalfLength * FMath::Abs(ClimbWallNormal.Z);
	const float Error = WallHit.Distance - TargetDistance;

	if (FMath::Abs(Error) <= ClimbAttachTolerance)
	{
		return;
	}

	// 3) 한 번에 최대 ClimbAttachMaxStep만 보정. Sweep으로 이동하므로 벽을 뚫지 않는다
	const float Correction = FMath::Clamp(Error, -ClimbAttachMaxStep, ClimbAttachMaxStep);

	FHitResult AttachHit;
	SafeMoveUpdatedComponent(IntoWall * Correction, UpdatedComponent->GetComponentQuat(), true, AttachHit);
}

void UDC1CharacterMovementComponent::RotateToWall(float DeltaTime)
{
	if (nullptr == UpdatedComponent || true == ClimbWallNormal.IsZero())
	{
		return;
	}

	// 수평 성분만 써서 Yaw만 돌린다(캡슐을 기울이면 바닥 판정 등 엔진 코드가 깨진다)
	const FVector FaceDirection = -ClimbWallNormal.GetSafeNormal2D();
	if (true == FaceDirection.IsZero())
	{
		return;
	}

	const FQuat CurrentQuat = UpdatedComponent->GetComponentQuat();
	const FQuat TargetQuat = FaceDirection.ToOrientationQuat();

	if (true == CurrentQuat.Equals(TargetQuat, UE_KINDA_SMALL_NUMBER))
	{
		return;
	}

	const FQuat NewQuat = FMath::QInterpTo(CurrentQuat, TargetQuat, DeltaTime, ClimbRotationInterpSpeed);

	// 엔진 PhysicsRotation과 같은 방식: 이동량 0 + 회전만 적용
	// 세로축 기준 Yaw 회전은 캡슐이 차지하는 공간이 같아 Sweep이 필요 없다
	MoveUpdatedComponent(FVector::ZeroVector, NewQuat, false);
}

FVector UDC1CharacterMovementComponent::GetClimbRightVector() const
{
	// 법선 × 월드 Up → 항상 수평. 기울어진 벽에서도 좌우 이동은 수평으로 한다
	// 예) 법선 -X(캐릭터가 +X를 봄): (-X) × Z = +Y(오른쪽)
	return FVector::CrossProduct(ClimbWallNormal, FVector::UpVector).GetSafeNormal();
}

FVector UDC1CharacterMovementComponent::GetClimbUpVector() const
{
	// Right × 법선 → 벽 평면을 따라 올라가는 방향(VectorPlaneProject(Up, 법선)과 같음)
	// 예) Right +Y, 법선 -X: Y × (-X) = +Z
	// 서로 수직인 단위벡터의 외적이라 길이는 이미 1
	return FVector::CrossProduct(GetClimbRightVector(), ClimbWallNormal);
}

void UDC1CharacterMovementComponent::ResetClimbState()
{
	bArriveAtTop = false;
	bCanMantle = false;
}

void UDC1CharacterMovementComponent::ReleaseClimb()
{
	if (false == IsClimbing())
		return;

	SetMovementMode(MOVE_Falling);
	bReleaseClimb = true;
}

void UDC1CharacterMovementComponent::SetupClimbQueryParams(FCollisionQueryParams& InOutQueryParams, FCollisionObjectQueryParams& OutObjectParams) const
{
	// 캡슐이 실제로 막히는 Simple 콜리전 기준으로 판정
	InOutQueryParams.bTraceComplex = false;

	if (nullptr != CharacterOwner)
	{
		InOutQueryParams.AddIgnoredActor(CharacterOwner);

		// 손에 붙은 무기 등 부착 액터가 벽으로 잡히지 않게 제외
		TArray<AActor*> AttachedActors;
		CharacterOwner->GetAttachedActors(AttachedActors);
		InOutQueryParams.AddIgnoredActors(AttachedActors);
	}

	// 임시: 정적/동적 지형만 대상(팰·플레이어 등 Pawn 제외). Climbable 전용 채널을 만들면 교체
	OutObjectParams = FCollisionObjectQueryParams();
	OutObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	OutObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
}

