// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/DC1Equip.h"
#include "GameplayTagContainer.h"
#include "DC1Tool.generated.h"

class USkeletalMeshComponent;

UCLASS()
class DC1_API ADC1Tool : public ADC1Equip
{
	GENERATED_BODY()

public:
	ADC1Tool();

public:
	// 상속받은 ItemMesh(Static)를 쓰지 않고 스켈레탈 메시를 쓴다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tool")
	TObjectPtr<USkeletalMeshComponent>	ToolMesh = { nullptr };
	// 다른 손 SubTool Mesh 없으면 한 손 툴
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool")
	TObjectPtr<USkeletalMeshComponent>	SubToolMesh = { nullptr };

	// EquipComponent가 도구를 식별하는 키
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tool")
	FGameplayTag		ToolTag;

	// 캐릭터 메시에 부착할 소켓 - Tool Mesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Tool")
	FName				AttachSocketName;

	// 캐릭터 메시에 부착할 소켓 - SubTool Mesh
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool")
	FName				SubAttachSocketName;
};