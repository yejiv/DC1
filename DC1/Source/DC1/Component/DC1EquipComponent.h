// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DC1EquipComponent.generated.h"

class ADC1Weapon;
class ADC1Tool;
class ADC1Player;

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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	// ASC 준비 후 서버에서 1회. 슬롯 무기 스폰 + 초기 장착
	// TODO : 본문은 인벤토리 연동 전 임시 구현 (.cpp 의 동명 TODO 참고)
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

	UFUNCTION()
	void		OnRep_CurrentWeapon();

	void		ApplyWeaponCollider();
	void		ApplyWeaponStateTag();
	void		ApplyWeaponAttach();

public:
	// Temp
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	TArray<TSubclassOf<ADC1Weapon>>		WeaponClass;

	// 슬롯 선택은 서버에서만 굴린다. 클라는 CurrentWeapon만 보면 되므로 복제하지 않는다
	UPROPERTY()
	int32								CurrentWeaponIndex = {};
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<ADC1Weapon>	CurrentWeapon = {};

private:
	enum class WEAPONSTATE { DETACH, ATTACH };
	void		SetWeaponState(WEAPONSTATE WeaponState);

private:
	int32								MaxWeaponEquip = { 6 };
	int32								CanWeaponEquip = { 4 };

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
