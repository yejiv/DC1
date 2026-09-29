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

void UDC1ChestInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 상자 데이터는 모든 클라이언트에 공유
	DOREPLIFETIME(UDC1ChestInventoryComponent, Slots);
}

bool UDC1ChestInventoryComponent::TryStoreItem(FName InItemID, int32 InCount)
{
	// 서버에서만 실행
	if (GetOwnerRole() != ROLE_Authority)
	{
		return false;
	}

	// 꺼내기 전용 상자는 보관 불가
	if (false == bCanStoreItems)
	{
		return false;
	}

	return AddItem(InItemID, InCount);
}

bool UDC1ChestInventoryComponent::CanAcceptItem(FName InItemID) const
{
	return false == IsValuableItem(InItemID);
}
