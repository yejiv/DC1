// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DC1AnimInstance_Pal.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/DC1Character.h"
#include "Data/DA/Pal/DC1PalData.h"

UDC1AnimInstance_Pal::UDC1AnimInstance_Pal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance_Pal::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachePalData();

	if (true == IsValid(Owner))
	{
		PrevYaw = Owner->GetActorRotation().Yaw;
	}
}

void UDC1AnimInstance_Pal::CachePalData()
{
	const ADC1PalCharacter* PalOwner = Cast<ADC1PalCharacter>(TryGetPawnOwner());
	if (false == IsValid(PalOwner))
	{
		return;
	}

	const UDC1PalData* PalData = PalOwner->GetPalData();
	if (nullptr == PalData)
	{
		return;
	}

	MaxLocomotionSpeed = PalData->RunSpeed;
	WalkLocomotionSpeed = PalData->WalkSpeed;
}

void UDC1AnimInstance_Pal::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// BP에 메시가 있으면 ApplyPalData보다 먼저 한 번 초기화됨. -> PalData가 아직 없어서 호출
	if (MaxLocomotionSpeed <= 0.f)
	{
		CachePalData();
	}

	if (MaxLocomotionSpeed <= 0.f)
	{
		return;
	}

	// 회전 각속도를 가상의 이동 속도로 환산
	float TurnSpeed = 0.f;
	if (true == IsValid(Owner) && DeltaSeconds > 0.f)
	{
		const float CurrentYaw = Owner->GetActorRotation().Yaw;
		TurnSpeed = FMath::Abs(FMath::FindDeltaAngleDegrees(PrevYaw, CurrentYaw)) / DeltaSeconds * TurnSpeedScale;
		PrevYaw = CurrentYaw;
	}

	// 서서 도는 동안만 걷는 포즈로 대신한다. 도는 전용 애니가 없어서다.
	// 이동 중에는 이동 속도만 본다. 둘을 섞으면 걸으며 도는 코너마다 값이 튄다.
	// 정지 판정에 여유를 둔다. 감속 잔여 속도가 조금만 남아도 회전 몫이 통째로 빠져 미끄러지듯 돈다
	const bool bStanding = (GroundSpeed < WalkLocomotionSpeed * StandingSpeedRatio);
	const float EffectiveSpeed = (true == bStanding) ? FMath::Min(TurnSpeed, WalkLocomotionSpeed) : GroundSpeed;

	const float SpeedRatio = EffectiveSpeed / MaxLocomotionSpeed;

	NormalizedSpeed = FMath::FInterpTo(NormalizedSpeed, FMath::Clamp(SpeedRatio, 0.f, 1.f), DeltaSeconds, BlendInterpSpeed);

	// 달리기 애니가 표현하는 속도를 넘겨 달릴 때만 그 배율만큼 빨리 감아 발을 지면에 맞춘다
	LocomotionPlayRate = FMath::FInterpTo(LocomotionPlayRate, FMath::Clamp(SpeedRatio, 1.f, MaxPlayRate), DeltaSeconds, BlendInterpSpeed);
}
