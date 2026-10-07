// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DC1EquipComponent.generated.h"

class ADC1Weapon;
class ADC1Tool;
class ADC1Player;
class UDC1PlayerInventoryComponent;

DECLARE_MULTICAST_DELEGATE(FOnEquipWeaponChanged);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1EquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDC1EquipComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// 인벤토리 무기 칸 기준으로 무기를 스폰하고 장착하는 함수 (폰이 붙을 때 서버)
	void		Init_Equip();

	// 폰이 이 PlayerState에 붙은 시점에 폰이 직접 호출한다.
	// 컴포넌트가 폰보다 오래 살기 때문에 폰이 필요한 작업은 전부 여기를 거친다
	void		HandlePawnReady();

public:
	UFUNCTION(Server, Reliable)
	void		Server_ChangeWeapon(int32 Delta);

public:
	ADC1Weapon*				GetCurrentWeapon() { return CurrentWeapon; }

public:
	FOnEquipWeaponChanged		OnWeaponChanged;

private:
	// 소유자는 PlayerState다. 폰은 아직 없거나 교체됐을 수 있어 캐싱하지 않고 매번 얻는다
	ADC1Player*	GetOwningPlayer() const;

	// 같은 PlayerState의 인벤토리 조회 함수
	UDC1PlayerInventoryComponent*	GetInventoryComponent() const;

	// 장비 칸 변경 시 무기 액터를 교체하는 함수 (서버)
	void		HandleEquipmentSlotChanged(int32 InEquipSlotIndex);

	// 무기 칸 하나를 인벤토리 기준으로 다시 스폰하는 함수 (비면 주먹)
	void		RefreshWeaponSlot(int32 InWeaponIndex);

	// 무기 칸 전체를 인벤토리 기준으로 다시 맞추고 현재 칸 무기를 드는 함수 (서버, 폰 필요)
	void		SyncWeaponsFromInventory();

	// 휠로 순환할 수 있는 무기 칸 수 조회 함수 (인벤토리의 열린 무기 칸 수)
	int32		GetUnlockedWeaponCount() const;

	UFUNCTION()
	void		OnRep_CurrentWeapon();

	void		ApplyWeaponCollider();
	void		ApplyWeaponStateTag();
	void		ApplyWeaponAttach();

public:
	// 슬롯 선택은 서버에서만 굴린다. 클라는 CurrentWeapon만 보면 되므로 복제하지 않는다
	UPROPERTY()
	int32								CurrentWeaponIndex = {};
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<ADC1Weapon>	CurrentWeapon = {};

private:
	enum class WEAPONSTATE { DETACH, ATTACH };
	void		SetWeaponState(WEAPONSTATE WeaponState);

private:
	UPROPERTY()
	TArray<TObjectPtr<ADC1Weapon>>		WeaponSlots;


// Tool
public:
	// GA가 도구 태그를 넘겨 착용/해제를 요청한다. 서버에서만 유효
	void		EquipTool(const FGameplayTag& ToolTag);
	void		UnequipTool(const FGameplayTag& ToolTag);

public:
	// 무기와 달리 교체 대상이 아닌 고정 목록. 식별 태그는 각 ADC1Tool이 들고 있다
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	TArray<TSubclassOf<ADC1Tool>>		ToolClass;

private:
	UPROPERTY()
	TArray<TObjectPtr<ADC1Tool>>		ToolSlots;

	// 표시만 바꾸므로 복제하지 않는다. 서버에서만 갱신된다
	UPROPERTY()
	TObjectPtr<ADC1Tool>				CurrentTool = { nullptr };


// PalSphere
public:
	// 팰스피어를 드는 동안 무기를 넣어둔다. 팰스피어 액터 자체는 여기서 다루지 않는다
	void		EquipPalSphere();
	void		UnequipPalSphere();

private:
	// 무기 표시 보류 판단용. 서버에서만 갱신된다
	bool		IsPalSphereEquipped = { false };
};
