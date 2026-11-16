// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "DC1EquipComponent.generated.h"

class ADC1Equip;
class ADC1Weapon;
class ADC1Glider;
class ADC1Tool;
class ADC1Player;
class UDC1PlayerInventoryComponent;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE(FOnEquipWeaponChanged);
DECLARE_MULTICAST_DELEGATE(FOnEquipArmorChanged);

// 장비 칸 하나에 실제로 적용된 상태. 인벤토리 칸이 바뀌면 이전 적용을 되돌리는 기준이 된다
USTRUCT()
struct FDC1EquipSlot
{
	GENERATED_BODY()

	// 이 칸에 적용된 아이템 (비었으면 NAME_None)
	UPROPERTY()
	FName ItemID = { NAME_None };

	// 액터로 스폰되는 타입(무기, 글라이더)만 채워진다
	UPROPERTY()
	TObjectPtr<ADC1Equip> EquipActor = { nullptr };

	// 방어구, 방패 칸이 적용한 스탯 GE Handle, 제거 위해 등록
	UPROPERTY()
	FActiveGameplayEffectHandle StatEffectHandle;
};

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
	ADC1Weapon*					GetCurrentWeapon() const { return CurrentWeapon; }
	ADC1Glider*					GetCurrentGlider() const { return CurrentGlider; }

	float						GetCurrentWeaponAttackPower() const;
	float						GetCurrentWeaponDamageRate(const FGameplayTag& InWorkTypeTag) const;
	void						SetGliderVisibility(bool IsVisiblity);

	// 무기 표시 최종 판단 (이동 조건 + 도구 + 팰스피어). 조건이 추가되면 여기만 고친다
	bool						CanShowWeapon() const;
	// CanShowWeapon 결과대로 지금 든 무기의 표시를 맞춘다
	void						RefreshWeaponVisibility();

public:
	FOnEquipWeaponChanged		OnWeaponChanged;

	// 방어구 메시가 실제로 교체된 뒤 (미리보기 갱신용)
	FOnEquipArmorChanged			OnArmorChanged;

private:
	// 소유자는 PlayerState다. 폰은 아직 없거나 교체됐을 수 있어 캐싱하지 않고 매번 얻는다
	ADC1Player*	GetOwningPlayer() const;

	// 같은 PlayerState의 인벤토리 조회 함수
	UDC1PlayerInventoryComponent*	GetInventoryComponent() const;

	UAbilitySystemComponent* GetOwningAbilitySystem() const;

	// 장비 칸 변경 시 이전 적용을 되돌리고 칸 타입별로 다시 적용하는 함수 (서버)
	void		HandleEquipmentSlotChanged(int32 InEquipSlotIndex);

	// 모든 칸 공통. 이전 적용(액터 등)을 해제하고 인벤토리 기준으로 ItemID를 갱신하는 함수
	void		RefreshEquipSlot(int32 InEquipSlotIndex);

	// 액터로 스폰되는 타입(무기, 글라이더)용. 칸의 ItemID로 액터를 스폰해 캐릭터에 붙이고 칸에 저장한다 (비었거나 실패면 nullptr)
	ADC1Equip*	SpawnEquipActor(int32 InEquipSlotIndex);

	// 장비 칸 전체를 인벤토리 기준으로 다시 맞추는 함수 (서버, 폰 필요)
	void		SyncEquipsFromInventory();

	// 휠로 순환할 수 있는 무기 칸 수 조회 함수 (인벤토리의 열린 무기 칸 수)
	int32		GetUnlockedWeaponCount() const;

	// 지금 든 무기의 장비 칸 인덱스 조회 함수 (무기 순번 -> EquipSlots 인덱스, 없으면 -1)
	int32		GetCurrentWeaponEquipIndex() const;

	UFUNCTION()
	void		OnRep_CurrentWeapon();
	UFUNCTION()
	void		OnRep_CurrentBodyArmor();
	UFUNCTION()
	void		OnRep_CurrentHairEquipArmor();

	void		ApplyWeaponCollider();
	void		ApplyWeaponStateTag();
	void		ApplyWeaponAttach();

private:
	// 무기 칸 적용: 액터 스폰 후 숨김, 지금 든 칸이면 바로 교체 (비었으면 주먹)
	void		HandleWeaponChanged(int32 InEquipSlotIndex);

	// 글라이더 칸 적용: 액터 스폰 후 숨김
	void		HandleGliderChanged(int32 InEquipSlotIndex);

	// 방어구 칸 적용: DefensePower, Health GE 처리
	void		HandleArmorChanged(int32 InEquipSlotIndex);

	// 실드 칸 적용: Shiled GE 처리
	void		HandleShieldChanged(int32 InEquipSlotIndex);

	// GE 생성 후 적용 함수
	void		ApplyEquipStatEffect(UAbilitySystemComponent* InASC, int32 InEquipSlotIndex, float InDefensePower, float InMaxHealth, float InMaxShield);

public:
	// 슬롯 선택은 서버에서만 굴린다. 클라는 CurrentWeapon만 보면 되므로 복제하지 않는다
	// 무기 순번(0 ~ 열린 무기 칸 수 - 1). EquipSlots 접근 시 GetCurrentWeaponEquipIndex로 변환
	UPROPERTY()
	int32								CurrentWeaponIndex = {};
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	TObjectPtr<ADC1Weapon>	CurrentWeapon = {};

	// 장착한 글라이더. 클라에서도 활공 모드 전환 시 표시를 토글해야 해서 모두에게 복제한다
	UPROPERTY(Replicated)
	TObjectPtr<ADC1Glider>		CurrentGlider = {};

	UPROPERTY(ReplicatedUsing = OnRep_CurrentBodyArmor)
	FName							CurrentBodyArmorID = {};

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHairEquipArmor)
	FName							CurrentHairEquipID = {};

private:
	enum class WEAPONSTATE { DETACH, ATTACH };
	void		SetWeaponState(WEAPONSTATE WeaponState);

private:
	// 장비 칸 인덱스(DC1EquipSlotLayout)로 접근. 크기는 SlotCount
	UPROPERTY()
	TArray<FDC1EquipSlot>			EquipSlots;

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
