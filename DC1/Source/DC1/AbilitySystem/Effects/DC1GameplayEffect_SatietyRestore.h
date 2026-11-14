#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "DC1GameplayEffect_SatietyRestore.generated.h"

// 음식 섭취 시 포만도 회복
// 값은 SetByCaller로(Instance, Additive)
UCLASS()
class DC1_API UDC1GameplayEffect_SatietyRestore : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UDC1GameplayEffect_SatietyRestore(const FObjectInitializer& ObjectInitializer);
};
