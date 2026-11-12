// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "DC1CharacterMovementComponent.generated.h"

struct FCollisionQueryParams;
struct FCollisionObjectQueryParams;

struct FSavedMove_DC1 : public FSavedMove_Character {

	typedef FSavedMove_Character Super;

	enum EMoveFlag {
		WALK = CompressedFlags::FLAG_Custom_0,
		SPRINT = CompressedFlags::FLAG_Custom_1,
		GLIDE = CompressedFlags::FLAG_Custom_2,
	};

	FSavedMove_DC1();

	virtual void Clear() override;
	virtual uint8 GetCompressedFlags() const override;
	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
	virtual void PrepMoveFor(ACharacter* C) override;

	uint8 Saved_bWantsToWalk : 1;
	uint8 Saved_bWantsToSprint : 1;
	uint8 Saved_bWantsToGlide : 1;
};

struct FNetworkPredictionData_Client_DC1 : public FNetworkPredictionData_Client_Character {

	typedef FNetworkPredictionData_Client_Character Super;

	FNetworkPredictionData_Client_DC1(const UCharacterMovementComponent& ClientMovement);

	virtual FSavedMovePtr AllocateNewMove() override;

};

enum ECustomMovementMode : int
{
	CMOVE_None,
	CMOVE_Climb,
	CMOVE_Glide,
};

UCLASS()
class DC1_API UDC1CharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	UDC1CharacterMovementComponent();

	virtual float GetMaxSpeed() const override;
	virtual class FNetworkPredictionData_Client* GetPredictionData_Client() const override;

private:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;

	virtual void	PhysCustom(float deltaTime, int32 Iterations) override;
	void			PhysClimbing(float deltaTime, int32 Iterations);
	void			PhysGliding(float deltaTime, int32 Iterations);

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;
	// 벽타기 중엔 회전을 PhysClimbing(RotateToWall)이 전담한다
	virtual void PhysicsRotation(float DeltaTime) override;

	virtual bool CanAttemptJump() const override;

public:
	void ToggleMove();
	void SetWalk(bool IsWalk);
	void SetSprint(bool Sprint);
	void SetGlide(bool IsGlide);

	bool IsClimbing() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Climb; }
	bool IsArriveAtTop() const { return bArriveAtTop; }
	bool CanMantle() const { return bCanMantle; }
	bool CanGliding() const;

	// 올라설 수 있는지 지금 위치에서 새로 판정한다(멤버를 건드리지 않음).
	// 이동 계산 밖(GA 활성화 등)에서 써도 된다. 성공 시 올라선 뒤 캡슐 중심 위치를 돌려준다
	bool FindMantleTarget(FVector& OutTargetLocation) const;
	const FVector& GetClimbWallNormal() const { return ClimbWallNormal; }

	// 벽 평면 기준 방향. 입력(PC OnMove)과 이동(PhysClimbing)이 같은 정의를 쓰도록 여기서만 계산한다
	// 벽 법선이 없으면 ZeroVector
	FVector GetClimbRightVector() const;
	FVector GetClimbUpVector() const;

	void ReleaseClimb();

	bool IsGliding() const { return MovementMode == MOVE_Custom && CustomMovementMode == CMOVE_Glide; }

	// 무기를 쥘 수 없는 이동 상태인지. 이동 모드 조건이 추가되면 여기만 고친다
	bool IsWeaponBlockedByMovement() const;

	// 서버에서 호출. 서버 회전을 바꾸고, 다음 보정에 회전을 실어 클라를 맞춘다
	// InbAlsoControlRotation : 카메라(ControlRotation)도 같은 방향으로
	void ForceRotation(const FRotator& InRotation, bool InbAlsoControlRotation);

protected:
	// 보정에 서버 회전을 포함할지 ( 강제 회전 후 클라가 따라잡을 때까지만 true )
	virtual bool ShouldCorrectRotation() const override;

	// 클라 이동이 정상 판정되면 강제 회전 보정 종료
	virtual void ServerMoveHandleClientError(float ClientTimeStamp, float DeltaTime, const FVector& Accel, const FVector& RelativeClientLocation, FMovementBaseInterfaceData* ClientMovementBaseInterfaceData, FName ClientBaseBoneName, uint8 ClientMovementMode) override;

private:
	void			ConfirmGlideMovement();
	void			ConfirmClimbMovement();

	bool			HasWallAtHeadHeight();
	bool			DetectWall();
	bool			CheckInputDirectionToWall();

	void			UpdateCanMantle();
	void			AttachToWall();
	void			RotateToWall(float DeltaTime);

	void			ResetClimbState();

	// 벽타기 판정 쿼리 공통 설정. 모든 판정이 같은 대상/제외 조건을 쓰도록 한 곳에서 채운다
	void			SetupClimbQueryParams(FCollisionQueryParams& InOutQueryParams, FCollisionObjectQueryParams& OutObjectParams) const;

public:

	uint8 Safe_bWantsToWalk : 1;
	uint8 Safe_bWantsToSprint : 1;
	uint8 Safe_bWantsToGlide : 1;

// ForceRotation
private:
	// 서버 전용. true인 동안 보정에 서버 회전을 실어 보낸다
	bool	bForceRotationCorrection = { false };

// Move
private:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	WalkSpeed = { 150.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	JogSpeed = { 300.f };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float	SprintSpeed = { 650.f };

// Climb
private:
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	ClimbSpeed = { 100.f };
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	ClimbJumpSpeed = { 300.f };
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	ClimbFriction = { 8.f };
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	BrakingDecelerationClimbing = { 1000.f };
	// 벽 타기 가능한 최대 각도
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	ClimbMaxAngle = { 120.f };
	// 다시 벽에 붙을 수 있는 Z 속도 임계점
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	ReattachZThreshold = { 10.f };
	// 벽 탐지 거리
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	DetectWallRange = { 50.f };
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float	ForwardDegree = { 45.f };

	// 꼭대기 바닥 탐색 시작 높이: 머리 위로 이만큼 여유
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Mantle")
	float	MantleTraceHeadMargin = { 10.f };
	// 착지 지점: 벽면에서 캡슐 반지름 + 이만큼 더 안쪽
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Mantle")
	float	MantleLandingInset = { 10.f };
	// 착지 공간 검사 시 바닥에서 캡슐을 띄우는 높이(붙이면 바닥과 닿아 항상 겹침)
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Mantle")
	float	MantleOverlapLift = { 2.f };

	// 벽에 붙어 있을 때 캡슐 표면과 벽 사이 간격(딱 붙이면 이동 Sweep이 매번 벽에 걸림)
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Attach")
	float	ClimbWallGap = { 2.f };
	// 목표 간격과의 오차가 이 값 이하면 보정하지 않음(울퉁불퉁한 벽에서 떨림 방지)
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Attach")
	float	ClimbAttachTolerance = { 0.5f };
	// 한 번에 보정하는 최대 거리(볼록 모서리 등에서 거리가 급변할 때 순간이동 방지)
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Attach")
	float	ClimbAttachMaxStep = { 5.f };
	// 벽을 바라보도록 도는 보간 속도(낮을수록 모서리에서 부드럽게 돈다)
	UPROPERTY(EditDefaultsOnly, Category = "Climb|Attach")
	float	ClimbRotationInterpSpeed = { 10.f };

	// 벽 탐지 결과(유효 히트 법선 평균). ZeroVector면 벽 없음
	FVector	ClimbWallNormal = { FVector::ZeroVector };

	// 벽 꼭대기 도착 여부
	bool	bArriveAtTop = { false };
	// 꼭대기 올라설 수 있는가 여부(소유 클라에서만 갱신. 착지 위치는 GA가 FindMantleTarget으로 직접 구한다)
	bool	bCanMantle = { false };
	// 벽 놓을 경우 다시 잡는 것을 막기위한 변수
	bool	bReleaseClimb = { false };

// Gliding
private:
	UPROPERTY(EditDefaultsOnly, Category = "Glide")
	float	GlideFriction = { 8.f };
	UPROPERTY(EditDefaultsOnly, Category = "Glide")
	float	BrakingDecelerationGliding = { 1000.f };
	UPROPERTY(EditDefaultsOnly, Category = "Glide")
	float	GlideFallSpeed = { 300.f };
};
