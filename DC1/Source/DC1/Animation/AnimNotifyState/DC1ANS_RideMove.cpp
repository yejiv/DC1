// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotifyState/DC1ANS_RideMove.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Component/Pal/DC1PalRideComponent.h"

#include "Components/SkeletalMeshComponent.h"

namespace DC1ANSRideMove
{
	// 탄 사람이 붙어 있는 팰의 탑승 컴포넌트
	static UDC1PalRideComponent* FindRideComponent(const USkeletalMeshComponent* InMeshComp)
	{
		const AActor* Rider = (nullptr != InMeshComp) ? InMeshComp->GetOwner() : nullptr;
		const ADC1PalCharacter* Pal = (nullptr != Rider) ? Cast<ADC1PalCharacter>(Rider->GetAttachParentActor()) : nullptr;
		return (nullptr != Pal) ? Pal->GetRideComponent() : nullptr;
	}
}

UDC1ANS_RideMove::UDC1ANS_RideMove()
{
	bShouldFireInEditor = false;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(80, 160, 230);
#endif
}

void UDC1ANS_RideMove::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (UDC1PalRideComponent* RideComponent = DC1ANSRideMove::FindRideComponent(MeshComp))
	{
		RideComponent->BeginRideMove(TotalDuration);
	}
}

void UDC1ANS_RideMove::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (UDC1PalRideComponent* RideComponent = DC1ANSRideMove::FindRideComponent(MeshComp))
	{
		RideComponent->EndRideMove();
	}
}

FString UDC1ANS_RideMove::GetNotifyName_Implementation() const
{
	return TEXT("Ride Move");
}
