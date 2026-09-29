// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "UI/Tab/DC1TabWidget.h"
#include "DC1ChestLootScreenWidget.generated.h"

class UDC1InventoryViewModel;
class UDC1PlayerLootInventoryWidget;
class UDC1ChestInventoryWidget;
class UDC1ChestInventoryComponent;

/**
 * 상자 개방 시 UDC1UIManagerSubsystem::PushScreen으로 열리는 화면(Screen) 위젯
 * 좌측에 플레이어 인벤토리(UDC1PlayerLootInventoryWidget), 우측에 상자 인벤토리(UDC1ChestInventoryWidget)를 나란히 배치
 * 두 패널이 동일한 UDC1InventoryViewModel 인스턴스를 공유하여 플레이어<->상자 드래그 앤 드롭이 동작함
 */
UCLASS()
class DC1_API UDC1ChestLootScreenWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1ChestLootScreenWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void NativeOnDeactivated() override;

public:
	/** 이 화면을 열며 대상 상자를 지정한다. 인벤토리 VM은 VMSubsystem에서 가져와 두 패널에 공유한다 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void OpenChest(UDC1ChestInventoryComponent* InChestComp);

	/** CloseKeys(기본 Esc, F)로 화면을 닫는다 (DC1TabWidget과 동일한 관례) */
	virtual void HandleInputKey(const FKey& InKey) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1PlayerLootInventoryWidget> PlayerPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1ChestInventoryWidget> ChestPanel;

	// 화면을 닫는 키 목록 (상호작용 키 F를 다시 눌러도 닫히도록 기본 포함)
	UPROPERTY(EditAnywhere, Category = "Inventory")
	FDC1KeyList CloseKeys;

private:
	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel> CachedViewModel;
};
