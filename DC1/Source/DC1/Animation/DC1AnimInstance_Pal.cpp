// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DC1AnimInstance_Pal.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Data/DA/Pal/DC1PalData.h"

UDC1AnimInstance_Pal::UDC1AnimInstance_Pal(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UDC1AnimInstance_Pal::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CachePalData();
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

	const float TargetSpeed = (MaxLocomotionSpeed > 0.f) ? FMath::Clamp(GroundSpeed / MaxLocomotionSpeed, 0.f, 1.f) : 0.f;

	NormalizedSpeed = FMath::FInterpTo(NormalizedSpeed, TargetSpeed, DeltaSeconds, BlendInterpSpeed);
}
