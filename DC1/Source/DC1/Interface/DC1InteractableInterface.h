// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "DC1InteractableInterface.generated.h"

class ADC1Character;

// INTERACT KEY
UENUM(BlueprintType)
enum class EDC1InteractKey : uint8
{
	F = 0 UMETA(DisplayName = "F"), // Default
	C = 1 UMETA(DisplayName = "C"),
	V = 2 UMETA(DisplayName = "V"),
};

// Interact input type
UENUM(BlueprintType)
enum class EDC1InteractInputType : uint8
{
	Press = 0 UMETA(DisplayName = "Press"),				// 누르는 순간 Interact
	HoldAbility = 1 UMETA(DisplayName = "HoldAbility"),	// 누르는 동안 Ability
	HoldConfirm = 2 UMETA(DisplayName = "HoldConfirm"),	// 특정 시간동안 누르면 Interact
};

USTRUCT(BlueprintType)
struct FDC1InteractOption
{
	GENERATED_BODY()

	// Default F, Interact Key ( F, C, V )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1InteractKey			Key = { EDC1InteractKey::F };

	// Interact UI DisplayText, ex) 상자 열기, 취소
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText					DisplayText;

	// Default Press, 상호작용 타입
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDC1InteractInputType	InputType = { EDC1InteractInputType::Press };

	// HoldAbility 활동 종류 ex) Interact.Work - 실행할 Ability는 행위자가 정함 ( UDC1InteractComponent::InteractionToAbility )
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Categories = "Interact", EditCondition = "InputType == EDC1InteractInputType::HoldAbility"))
	FGameplayTag			InteractionTag;
};

DECLARE_MULTICAST_DELEGATE(FDC1OnInteractOptionsChanged);

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
	virtual UPrimitiveComponent*			GetInteractCollider() const = 0;

	// 현재 상태에서 받는 상호작용 목록 ( 개수 = 월드 UI 줄 )
	virtual void							GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const = 0;

	// 옵션 목록이 바뀔 때 Broadcast - 포커스 중 UI/키 판정 갱신용
	virtual FDC1OnInteractOptionsChanged&	GetOnInteractOptionsChanged() = 0;

	// InputType - Press, HoldConfirm 옵션 실행
	virtual void							Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey) {}

	// (InputType == HoldAbility) 키를 누르는 동안 옵션 변경(갱신)을 키 Release까지 미룰 것인가
	virtual bool							ShouldKeepOptionsWhileHeld() const { return false; }

	// 로컬 플레이어 포커스 변경
	virtual void							OnLocalFocusChanged(bool InIsFocused) {}
};
