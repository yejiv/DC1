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
}

void UDC1AnimInstance_Pal::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// BP에 메시가 있으면 ApplyPalData보다 먼저 한 번 초기화됨. -> PalData가 아직 없어서 호출
	if (MaxLocomotionSpeed <= 0.f)
	{
		CachePalData();
	}

	// 회전 각속도를 속도로 환산 (미끄러짐 애니메이션 방지)
	float TurnSpeed = 0.f;
	if (true == IsValid(Owner) && DeltaSeconds > 0.f)
	{
		const float CurrentYaw = Owner->GetActorRotation().Yaw;
		TurnSpeed = FMath::Abs(FMath::FindDeltaAngleDegrees(PrevYaw, CurrentYaw)) / DeltaSeconds * TurnSpeedScale;
		PrevYaw = CurrentYaw;
	}

	const float EffectiveSpeed = FMath::Max(GroundSpeed, TurnSpeed);
	const float TargetSpeed = (MaxLocomotionSpeed > 0.f) ? FMath::Clamp(EffectiveSpeed / MaxLocomotionSpeed, 0.f, 1.f) : 0.f;

	NormalizedSpeed = FMath::FInterpTo(NormalizedSpeed, TargetSpeed, DeltaSeconds, BlendInterpSpeed);
}
