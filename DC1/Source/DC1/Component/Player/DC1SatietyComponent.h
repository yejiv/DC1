#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DC1SatietyComponent.generated.h"

struct FOnAttributeChangeData;

class UDC1PlayerInventoryComponent;
class UDC1AbilitySystemComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1SatietyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1SatietyComponent();

public:
	// HUD 배고픔 표시 기준
	float	GetAutoEatRatio() const { return AutoEatRatio; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void	RequestEatFood(int32 InSlotIndex);

	bool	CanEatFood(FName InItemID) const;



private:
	UDC1PlayerInventoryComponent*	GetInventoryComponent() const;
	UDC1AbilitySystemComponent*		GetDC1AbilitySystemComponent() const;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void	Server_EatFood(int32 InSlotIndex);

	bool	TryEatFood(FName InItemID);

	void	HandleSatietyChanged(const FOnAttributeChangeData& InData);

	void	UpdateStarvingState(float InSatiety);

	void	HandleEquipmentSlotChanged(int32 InEquipSlotIndex);

	void	TryAutoEatFromPouch();

private:
	// 자동 섭취 포만감 비율
	UPROPERTY(EditDefaultsOnly, Category = "Satiety", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float	AutoEatRatio = { 0.3f };

	bool	bAutoEating = { false };
};
