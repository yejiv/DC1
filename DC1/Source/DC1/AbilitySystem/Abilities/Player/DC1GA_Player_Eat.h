#pragma once

#include "AbilitySystem/Abilities/DC1GameplayAbility.h"
#include "DC1GA_Player_Eat.generated.h"

UCLASS()
class DC1_API UDC1GA_Player_Eat : public UDC1GameplayAbility
{
	GENERATED_BODY()

public:
	UDC1GA_Player_Eat(const FObjectInitializer& ObjectInitializer);

protected:
	virtual bool	CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr, OUT FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;
	virtual void	ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

private:
	void	ApplyFoodEffects(float InSatietyAmount);

	UFUNCTION()
	void	OnMontageFinished();
};
