// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "DC1AnimNotify_PalWorkHit.generated.h"

class UAnimNotify_PlaySound;
class UAnimNotify_PlayNiagaraEffect;

// 자원 타격 (단타) - 메쉬 주인에게 Event.Pal.Work.Hit ( EventMagnitude = 배율 )
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "DC1 Pal Work Hit"))
class DC1_API UDC1AnimNotify_PalWorkHit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UDC1AnimNotify_PalWorkHit();

	virtual void	Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	// 노티파이 / 노티파이 스테이트 공용
	static void		SendWorkHit(USkeletalMeshComponent* InMeshComp, float InMultiplier);

protected:
	UPROPERTY(EditAnywhere, Category = "DC1", meta = (ClampMin = "0"))
	float			Multiplier = { 1.f };

	// 작업장(벌목/채굴장) 작업일 때만 재생
	UPROPERTY(EditAnywhere, Instanced, Category = "DC1|Feedback")
	TObjectPtr<UAnimNotify_PlaySound>			SoundNotify = { nullptr };
	// 작업장(벌목/채굴장) 작업일 때만 재생
	UPROPERTY(EditAnywhere, Instanced, Category = "DC1|Feedback")
	TObjectPtr<UAnimNotify_PlayNiagaraEffect>	FXNotify = { nullptr };

private:
	// 작업 대상이 리소스가 아닐 때만 소켓에서 사운드/에픽트 재생
	void			PlayWorkFeedback(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, const FAnimNotifyEventReference& InEventReference) const;
	void			PlayFeedbackNotifies(USkeletalMeshComponent* InMeshComp, UAnimSequenceBase* InAnimation, const FAnimNotifyEventReference& InEventReference) const;
};
