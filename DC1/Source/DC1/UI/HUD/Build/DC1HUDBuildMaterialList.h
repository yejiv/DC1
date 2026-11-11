// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "DC1HUDBuildMaterialList.generated.h"

class UDC1CraftMaterialEntryWidget;
class UDC1InventoryComponentBase;
class UPanelWidget;

UCLASS()
class DC1_API UDC1HUDBuildMaterialList : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetCosts(const TArray<FDC1ItemCost>& InCosts);
	void SetAmounts(const TArray<FDC1ItemCost>& InCosts);
	void Clear();

private:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

private:
	void RefreshOwnedCounts();

	void HandleInventoryUpdated();

	void RebindInventories();
	void UnbindInventories();
	bool IsSameInventories(const TArray<UDC1InventoryComponentBase*>& InInventories) const;

	void StartInventoryRecheck();
	void StopInventoryRecheck();

private:
	// 사용/반환 재료 목록
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget>			SlotBox = { nullptr };

private:
	// 거점 인벤토리 갱신 주기
	UPROPERTY(EditAnywhere, Category = "HUD|Build", meta = (ClampMin = "0.1"))
	float								InventoryRecheckInterval = { 0.25f };

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDC1CraftMaterialEntryWidget>>	Slots;

	TArray<FDC1ItemCost>				CachedCosts;

	TArray<TWeakObjectPtr<UDC1InventoryComponentBase>>	BoundInventories;

	FTimerHandle						InventoryRecheckTimer;
};
