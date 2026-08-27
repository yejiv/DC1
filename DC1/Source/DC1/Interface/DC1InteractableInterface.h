// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DC1InteractableInterface.generated.h"

class ADC1Character;

UINTERFACE(MinimalAPI)
class UDC1InteractableInterface : public UInterface
{
	GENERATED_BODY()
};

class DC1_API IDC1InteractableInterface
{
	GENERATED_BODY()

public:
	// 시선 판정 기준점을 위한 InteractCollider 가져오기
	virtual UPrimitiveComponent*	GetInteractCollider() const = 0;

	// Ability로 상호작용 처리 시 사용할 Tag - ex) 건설 Ability 실행 - Ability_Player_BuildWork, 즉발(아이템 줍기, UI 열기) - EmptyTag
	virtual FGameplayTag GetInteractAbilityTag() const { return FGameplayTag::EmptyTag; }

	// 상호작용 GetInteractAbilityTag가 EmptyTag라면 실행될 상호작용
	virtual void					Interact(ADC1Character* InInteractOwner) {}

	// 상호작용 타겟이 되었을때 PushScreen UI 
	virtual void					OnInteractableFocused() {};

	// 상호작용 타겟에서 벗어났을때 PopScreen UI
	virtual void					OnInteractableUnfocused() {};
};
