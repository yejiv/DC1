// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Inventory/DC1ChestInventoryComponent.h"
#include "Net/UnrealNetwork.h"

UDC1ChestInventoryComponent::UDC1ChestInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	// 상자 기본값
	TotalCapacity = 42;
	Columns = 6;
	ChestName = FName("Chest");
}

void UDC1ChestInventoryComponent::GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 상자 데이터는 모든 클라이언트에 공유
	// Slots는 이미 부모에서 복제 설정됨 (COND_None)
}
