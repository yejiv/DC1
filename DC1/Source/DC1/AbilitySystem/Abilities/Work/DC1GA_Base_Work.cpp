// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/Work/DC1GA_Base_Work.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1EquipComponent.h"

void UDC1GA_Base_Work::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	// 도구를 쓰지 않는 Work도 있으므로 태그가 비어 있으면 무기를 그대로 둔다
	if (false == EquipToolTag.IsValid())
	{
		return;
	}

	UDC1EquipComponent* EquipComp = GetEquipComponent();
	if (nullptr == EquipComp)
	{
		return;
	}

	// 착용/해제는 EquipComponent가 서버 권한을 확인한다
	EquipComp->EquipTool(EquipToolTag);
}

void UDC1GA_Base_Work::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (true == EquipToolTag.IsValid())
	{
		if (UDC1EquipComponent* EquipComp = GetEquipComponent())
		{
			EquipComp->UnequipTool(EquipToolTag);
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UDC1EquipComponent* UDC1GA_Base_Work::GetEquipComponent() const
{
	// ASC가 PlayerState에 있으므로 OwningActor가 곧 PlayerState다
	const ADC1PlayerState* PS = Cast<ADC1PlayerState>(GetOwningActorFromActorInfo());
	if (false == ::IsValid(PS))
	{
		return nullptr;
	}

	return PS->GetEquipComponent();
}
