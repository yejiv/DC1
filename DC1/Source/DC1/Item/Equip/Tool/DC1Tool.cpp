// Fill out your copyright notice in the Description page of Project Settings.

#include "Item/Equip/Tool/DC1Tool.h"

#include "Components/SkeletalMeshComponent.h"

ADC1Tool::ADC1Tool()
	: Super()
{
	ToolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ToolMesh");
	ToolMesh->SetupAttachment(ItemMesh);
	ToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	SubToolMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SubToolMesh");
	if (true == ::IsValid(SubToolMesh))
	{
		SubToolMesh->SetupAttachment(ItemMesh);
		SubToolMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// SubTool은 EquipComp::EquipTool에서 ItemMesh가 아닌 Player에게 직접 붙음 ( EquipTool이 서버에서만 불림 == 복제 켜야 서버환경에서 제대로 붙음 )
		SubToolMesh->SetIsReplicated(true);
	}
}