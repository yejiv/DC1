// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "DC1ANS_NockArrow.generated.h"

class ADC1ProjectileWeapon;

/*
 * 활 당기는 구간 동안 현재 무기의 NockedArrowMesh 표시 ( 발사 섹션 직전까지 )
 * 몽타주 복제로 다른 클라에서도 돈다. 노티파이 인스턴스는 애셋당 하나라 켠 무기를 메쉬별로 기억해 같은 무기를 끈다
 */
UCLASS(const, hidecategories = Object, collapsecategories, meta = (DisplayName = "DC1 Nock Arrow"))
class DC1_API UDC1ANS_NockArrow : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UDC1ANS_NockArrow();

	virtual void	NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void	NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

private:
	static ADC1ProjectileWeapon*	FindProjectileWeapon(USkeletalMeshComponent* MeshComp);

private:
	TMap<TWeakObjectPtr<USkeletalMeshComponent>, TWeakObjectPtr<ADC1ProjectileWeapon>>	NockedByMesh;
};
