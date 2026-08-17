// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DC1GameplayEffect_Damage.generated.h"

// Instant. SetByCaller.Damage 값을 Damage 메타 어트리뷰트에 더한다.
// 실제 Health 감소는 UDC1AttributeSet::PostGameplayEffectExecute에서!

UCLASS()
class DC1_API UDC1GameplayEffect_Damage : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDC1GameplayEffect_Damage(const FObjectInitializer& ObjectInitializer);
};
