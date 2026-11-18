// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/DC1Item.h"
#include "Interface/DC1InteractableInterface.h"
#include "Interface/DC1SaveInterface.h"
#include "DC1PickupItem.generated.h"

class USphereComponent;
class ADC1Player;
class UDC1PlayerInventoryComponent;

/*
월드에 떨어진 아이템 스택 ( 채집 드랍 · 인벤토리 버리기 · 이후 팰 드랍 공용 )
- ItemMesh ( 루트 ) = 픽업 몸체 : 서버 물리로 떨어지고 픽업끼리 충돌, 플레이어와 겹치면 자동 줍기 ( Item 프로필 )
- InteractCollider : 범위 안에서 F로 줍기 ( Interaction 프로필 )
- 자동 줍기 막힘 : 플레이어가 버린 것 ( 누구든 F 전용 ) · 넣으면 최대 무게 초과. F는 무게를 보지 않는다
- 멈추면 물리 off + DORM_DormantAll. 아무도 안 주우면 PickupLifeSpan 뒤 사라진다
*/
UCLASS()
class DC1_API ADC1PickupItem : public ADC1Item, public IDC1InteractableInterface, public IDC1SaveInterface
{
	GENERATED_BODY()

public:
	ADC1PickupItem();

public:
	// 서버 - 드랍 진입점, InInteractOnly = 플레이어가 버린 것 ( F 입력 전용 )
	static ADC1PickupItem*	SpawnDrop(UWorld* InWorld, FName InItemID, int32 InCount, const FVector& InLocation, bool InInteractOnly = false);

	// 서버 - 줍기, InByInteract = F 입력으로 들어왔는지
	void					TryPickup(ADC1Player* InPlayer, bool InByInteract);

public:
	// IDC1InteractableInterface
	virtual UPrimitiveComponent*			GetInteractCollider() const override;
	virtual void							GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;
	virtual FDC1OnInteractOptionsChanged&	GetOnInteractOptionsChanged() override { return OnInteractOptionsChanged; }
	virtual void							Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey) override;

protected:
	virtual void	BeginPlay() override;
	virtual void	EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void	GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// 서버 - FinishSpawning 전에 ( 첫 번들에 ItemID가 실리도록 )
	void			InitPickup(FName InItemID, int32 InCount, bool InInteractOnly);

	bool			CanCarryWeight(const UDC1PlayerInventoryComponent* InInventory) const;

	UFUNCTION()
	void			OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void			OnPhysicsSleep(UPrimitiveComponent* SleepingComponent, FName BoneName);
	void			StopPhysics();

	UFUNCTION()
	void			OnRep_ItemID();
	void			ApplyItemMesh();

	// 서버 - 줍기로 사라질 때의 단일 출구 ( 풀 도입 시 여기서 반납 )
	void			ReleasePickup();

protected:
	// Interact 범위
	UPROPERTY(VisibleAnywhere, Category = "Pickup")
	TObjectPtr<USphereComponent>	InteractCollider = { nullptr };

	// Interact Prompt 옵션 ( F만이라 단일 변수 )
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	FDC1InteractOption				PickupOption;

	// 줍지 않았을 때 수명
	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = "1"))
	float							PickupLifeSpan = { 300.f };

	// 이 시간 안에 잠들지 않으면 물리를 강제로 끔
	UPROPERTY(EditDefaultsOnly, Category = "Pickup", meta = (ClampMin = "0.5"))
	float							PhysicsSettleTimeout = { 5.f };

private:
	UPROPERTY(ReplicatedUsing = OnRep_ItemID, SaveGame)
	FName							ItemID;

	// 서버만 - 복제 안 함
	UPROPERTY(SaveGame)
	int32							Count = { 0 };
	UPROPERTY(SaveGame)
	bool							IsInteractOnly = { false };

	FTimerHandle					SettleTimer;

	FDC1OnInteractOptionsChanged	OnInteractOptionsChanged;
};
