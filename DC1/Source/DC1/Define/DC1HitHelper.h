#pragma once

#include "CoreMinimal.h"

struct FGameplayEffectSpec;
class UPrimitiveComponent;

/*
* 맞은 지점 계산 Helper
* Context Hit -> 공격자 높이 수평 -> 피격자 중심
*/ 
namespace DC1HitHelper
{
	bool FindHitLocation(const FGameplayEffectSpec& InSpec, const AActor* InTarget, UPrimitiveComponent* InSurface, FVector& OutPoint, FVector& OutNormal);
}
