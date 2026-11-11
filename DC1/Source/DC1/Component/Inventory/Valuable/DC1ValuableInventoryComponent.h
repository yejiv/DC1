// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "DC1ValuableInventoryComponent.generated.h"

/**
 * 귀중품 인벤토리 컴포넌트
 * 귀중품 DataTable(DataTable_ValuableItem)의 아이템만 보관하며, 고정 칸 없이 가진 아이템 종류 수만큼만 슬롯이 존재
 * 같은 아이템은 한 슬롯에 개수로 누적 (스택 제한 없음), 개수가 0이 되면 슬롯 제거
 * 무게 계산 제외, 상자 보관 / 버리기 불가
 *
 * APlayerState에 부착, 복제 설정: COND_OwnerOnly (플레이어 본인과 서버만 데이터 공유)
 */
UCLASS()
class DC1_API UDC1ValuableInventoryComponent : public UDC1InventoryComponentBase
{
	GENERATED_BODY()

public:
	UDC1ValuableInventoryComponent();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	virtual bool AddItem(FName InItemID, int32 InCount = 1) override;
	virtual int32 AddItemPartial(FName InItemID, int32 InCount) override;
	virtual bool RemoveItem(int32 InSlotIndex, int32 InCount = 1) override;
	virtual bool CanAcceptItem(FName InItemID) const override;

protected:
	virtual void InitializeSlots() override;
	virtual int32 GetStackLimit(const FDC1ItemRow& InItemRow) const override;
	virtual bool IsFixedCapacity() const override { return false; }
};
