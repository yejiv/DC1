// Fill out your copyright notice in the Description page of Project Settings.

#include "Animation/AnimNotifyState/DC1ANS_WorkHitWindow.h"

#include "Animation/Notify/DC1AnimNotify_PalWorkHit.h"

#include "Components/SkeletalMeshComponent.h"

UDC1ANS_WorkHitWindow::UDC1ANS_WorkHitWindow()
{
	// 몽타주 에디터 프리뷰 액터는 ASC가 없음 - Interval마다 엔진 Error 로그만 쌓인다
	bShouldFireInEditor = false;

#if WITH_EDITORONLY_DATA
	NotifyColor = FColor(230, 120, 40);
#endif
}

void UDC1ANS_WorkHitWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	// End 없이 파괴된 메쉬 정리
	for (TMap<TWeakObjectPtr<USkeletalMeshComponent>, float>::TIterator It = ElapsedByMesh.CreateIterator(); It; ++It)
	{
		if (false == It->Key.IsValid())
		{
			It.RemoveCurrent();
		}
	}

	if (false == ::IsValid(MeshComp))
	{
		return;
	}

	ElapsedByMesh.Add(MeshComp, 0.f);
}

void UDC1ANS_WorkHitWindow::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	float* Elapsed = ElapsedByMesh.Find(MeshComp);
	if (nullptr == Elapsed)
	{
		return;
	}

	// 횟수를 먼저 센다 - 보내는 도중 GA 종료 등으로 맵이 바뀌어도 포인터를 쓰지 않게
	*Elapsed += FrameDeltaTime;

	int32 HitCount = 0;
	while (*Elapsed >= Interval)
	{
		*Elapsed -= Interval;
		++HitCount;
	}

	for (int32 Index = 0; Index < HitCount; ++Index)
	{
		UDC1AnimNotify_PalWorkHit::SendWorkHit(MeshComp, Multiplier);
	}
}

void UDC1ANS_WorkHitWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ElapsedByMesh.Remove(MeshComp);
}

FString UDC1ANS_WorkHitWindow::GetNotifyName_Implementation() const
{
	return TEXT("Work Hit Window");
}
