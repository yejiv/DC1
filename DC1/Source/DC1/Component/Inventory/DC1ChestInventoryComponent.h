// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Component/Inventory/DC1InventoryComponentBase.h"
#include "DC1ChestInventoryComponent.generated.h"

/**
 * 상자 인벤토리 컴포넌트
 * 기본 인벤토리 기능만 사용하는 순수한 저장소
 *
 * ChestActor에 부착되어 월드의 상자들이 아이템을 보관
 * 복제 설정: COND_None (모든 클라이언트에 공유됨)
 */
UCLASS()
class DC1_API UDC1ChestInventoryComponent : public UDC1InventoryComponentBase
{
	GENERATED_BODY()

public:
	UDC1ChestInventoryComponent();

	// ========================
	// 라이프사이클
	// ========================
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ========================
	// 상자 설정 관련 함수
	// ========================

	/**
	 * 상자 이름 설정
	 * @param InChestName 상자 이름
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetChestName(FName InChestName) { ChestName = InChestName; }

	/**
	 * 상자 이름 조회
	 * @return 상자 이름
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	FName GetChestName() const { return ChestName; }

	/**
	 * 이 상자가 아이템을 보관(입고)할 수 있는지 조회
	 * false인 경우 꺼내기(RemoveItem)만 허용되는 상자
	 * @return 보관 가능 여부
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
	bool CanStoreItems() const { return bCanStoreItems; }

	/**
	 * 이 상자의 보관 가능 여부 설정
	 * @param InCanStoreItems 보관 가능 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetCanStoreItems(bool InCanStoreItems) { bCanStoreItems = InCanStoreItems; }

	// ========================
	// 아이템 보관 함수
	// ========================

	/**
	 * 아이템 보관 시도 ( 서버 권한 + 보관 가능 여부 체크 후 AddItem 호출 )
	 * @param InItemID 보관할 아이템 ID
	 * @param InCount 보관할 개수
	 * @return 보관 성공 여부
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryStoreItem(FName InItemID, int32 InCount = 1);

	// ========================
	// 멤버 변수
	// ========================

protected:
	/** 상자 이름 (에디터에서 설정 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ChestName = FName("Chest");

	/** 아이템 보관(입고) 가능 여부, false면 꺼내기 전용 상자 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	bool bCanStoreItems = true;
};
