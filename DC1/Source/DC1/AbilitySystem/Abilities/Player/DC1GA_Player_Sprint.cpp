// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/Abilities/Player/DC1GA_Player_Sprint.h"

#include "Tags/DC1GamePlayTags.h"
#include "Character/Player/DC1Player.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1CharacterMovementComponent.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"

UDC1GA_Player_Sprint::UDC1GA_Player_Sprint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// 어빌리티 수명 = 달리는 동안
	ActivationOwnedTags.AddTag(DC1StateTags::State_Sprinting);

	// 재진입 금지. 조준 가드는 PC가 아니라 여기가 주인이다
	ActivationBlockedTags.AddTag(DC1StateTags::State_Sprinting);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Aiming);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Dead);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Climbing);
	ActivationBlockedTags.AddTag(DC1StateTags::State_Overweight);
}

bool UDC1GA_Player_Sprint::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
	if (false == Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const ADC1Player* Player = GetPlayer();
	if (nullptr == Player)
	{
		return false;
	}

	// 웅크림은 태그가 아니라 CMC 상태라 여기서 본다
	if (const UCharacterMovementComponent* CMC = Player->GetCharacterMovement())
	{
		if (true == CMC->IsCrouching())
			return false;
	}

	// 지속 소모라 CostGameplayEffectClass를 쓰지 않는다. 그래서 CheckCost가 돌지 않아
	// 스태미나 고갈 시 진입 차단을 여기서 직접 해 준다
	if (const UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent())
	{
		if (ASC->GetNumericAttribute(UDC1CharacterAttributeSet::GetStaminaAttribute()) <= 0.f)
			return false;
	}

	return true;
}

void UDC1GA_Player_Sprint::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (false == CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ADC1Player* Player = GetPlayer();
	if (nullptr == Player)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// 스프린트 의도의 주인은 소유 클라다. 서버 쪽 쓰기는 다음 ServerMove가 덮으므로 무해하다
	Player->SetSprint(true);

	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	if (nullptr == ASC)
	{
		return;
	}

	StaminaChangedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetStaminaAttribute())
		.AddUObject(this, &ThisClass::HandleStaminaChanged);
	// OverWeight Tag 이벤트 구독, State_OverWeight 부여되면 EndAbility
	OverweightTagChangedHandle = ASC->RegisterGameplayTagEvent(DC1StateTags::State_Overweight, EGameplayTagEventType::NewOrRemoved)
		.AddUObject(this, &ThisClass::HandleOverweightTagChanged);

	// 어트리뷰트 변경은 서버 권한이다
	if (nullptr != CostEffectClass && true == ASC->IsOwnerActorAuthoritative())
	{
		FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(CostEffectClass, GetAbilityLevel(), ASC->MakeEffectContext());
		if (true == Spec.IsValid())
			CostEffectHandle = ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void UDC1GA_Player_Sprint::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (ADC1Player* Player = GetPlayer())
	{
		Player->SetSprint(false);
	}

	// Super가 ActorInfo를 정리하기 전에 뒷정리를 끝낸다
	if (nullptr != ActorInfo)
	{
		if (UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get())
		{
			if (true == StaminaChangedHandle.IsValid())
			{
				ASC->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetStaminaAttribute()).Remove(StaminaChangedHandle);
				StaminaChangedHandle.Reset();
			}

			ASC->RegisterGameplayTagEvent(DC1StateTags::State_Overweight, EGameplayTagEventType::NewOrRemoved).Remove(OverweightTagChangedHandle);
			OverweightTagChangedHandle.Reset();

			// 회복 지연은 이 GE의 OnCompleteAlways가 이어받는다
			if (true == CostEffectHandle.IsValid())
			{
				ASC->RemoveActiveGameplayEffect(CostEffectHandle);
				CostEffectHandle = FActiveGameplayEffectHandle();
			}
		}
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDC1GA_Player_Sprint::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	if (Data.NewValue > 0.f)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UDC1GA_Player_Sprint::HandleOverweightTagChanged(const FGameplayTag InTag, int32 InNewCount)
{
	if (InNewCount <= 0)
	{
		return;
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

ADC1Player* UDC1GA_Player_Sprint::GetPlayer() const
{
	return Cast<ADC1Player>(GetAvatarActorFromActorInfo());
}