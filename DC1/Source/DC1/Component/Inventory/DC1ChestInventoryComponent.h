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

	// ========================
	// 멤버 변수
	// ========================

protected:
	/** 상자 이름 (에디터에서 설정 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	FName ChestName = FName("Chest");
};
