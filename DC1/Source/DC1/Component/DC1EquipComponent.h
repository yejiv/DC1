// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DC1EquipComponent.generated.h"

class ADC1Weapon;
class ADC1Tool;

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

public:
	UFUNCTION(Server, Reliable)
	void		Server_ChangeWeapon(int32 Delta);

private:
	UFUNCTION()
	void		OnRep_CurrentWeaponIndex();
	UFUNCTION()
	void		OnRep_CurrentWeapon();

	void		ApplyWeaponCollider();
	void		ApplyWeaponStateTag();
	void		ApplyWeaponAttach();

public:
	// Temp
	UPROPERTY(EditDefaultsOnly, Category = "Equip")
	TArray<TSubclassOf<ADC1Weapon>>		WeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponIndex)
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
};
