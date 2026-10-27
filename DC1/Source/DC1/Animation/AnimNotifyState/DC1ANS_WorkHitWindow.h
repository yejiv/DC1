// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "DC1ANS_WorkHitWindow.generated.h"

/*
 * 연속 타격 창 ( 회전 등 ) - 창 안에서 Interval마다 Event.Pal.Work.Hit 1회. 첫 타격은 Begin 후 Interval
 * 노티파이 인스턴스는 애셋당 하나라 같은 몽타주를 트는 펠들이 공유한다 → 경과 시간은 메쉬별로
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "DC1 Work Hit Window"))
class DC1_API UDC1ANS_WorkHitWindow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UDC1ANS_WorkHitWindow();

	virtual void	NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void	NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void	NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

protected:
	UPROPERTY(EditAnywhere, Category = "DC1", meta = (ClampMin = "0.01"))
	float			Interval = { 0.1f };

	UPROPERTY(EditAnywhere, Category = "DC1", meta = (ClampMin = "0"))
	float			Multiplier = { 1.f };

private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, float>	ElapsedByMesh;
};
