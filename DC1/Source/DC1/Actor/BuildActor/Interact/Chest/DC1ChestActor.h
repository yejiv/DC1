// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "DC1ChestActor.generated.h"

class UDC1ChestInventoryComponent;

/**
 * 상자 액터
 * DC1BuildActor를 상속받아 건설 가능한 상자를 표현
 * UDC1ChestInventoryComponent를 부착하여 아이템 보관 기능 제공
 *
 * 보관 가능 여부는 UDC1ChestInventoryComponent::CanStoreItems() 로 구분
 * ( 꺼내기만 가능한 상자 / 넣고 뺄 수 있는 상자 )
 */

UCLASS()
class DC1_API ADC1ChestActor : public ADC1InteractBuildActor
{
	GENERATED_BODY()

public:
	ADC1ChestActor();

public:
	UDC1ChestInventoryComponent*	GetChestInventoryComponent() const { return ChestInventoryComp; }

public:
	// 아이템 보관, 서버 권한 + 상자의 보관 가능 여부를 확인 후 처리
	bool							StoreItem(FName InItemID, int32 InCount = 1);

	// 아이템 꺼내기, 서버 권한 확인 후 처리
	bool							TakeItem(int32 InSlotIndex, int32 InCount = 1);

protected:
	virtual void					BeginPlay() override;

private:
	// 아이템 DataTable의 앞쪽 행을 상자에 미리 채워 넣음 (서버 전용)
	void							AddDefaultItems();

	// Interact
	virtual void					OnCompletedInteract(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;
	// HandlleBuildCompleted 바인딩
	virtual void					OnPlaced(const APlayerState* InBuilder) override;
	// 완성 시 거점 상자 등록 ( BaseRegistryComp )
	void							HandleBuildCompleted();
protected:
	// 상자 종류별 수납 칸 수 ( BP 서브클래스에서 상자 종류에 맞게 설정, 예: 나무 10 / 금속 24 / 제련 금속 40 )
	UPROPERTY(EditDefaultsOnly, Category = "Chest")
	int32							ChestCapacity = { 10 };

	// 상자 인벤토리 UI 열 개수
	UPROPERTY(EditDefaultsOnly, Category = "Chest")
	int32							ChestColumns = { 5 };

	UPROPERTY(VisibleAnywhere, Category = "Chest")
	TObjectPtr<UDC1ChestInventoryComponent>	ChestInventoryComp = { nullptr };
};
