// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "DC1PlayerState.generated.h"

class UDC1AbilitySystemComponent;
class UDC1PlayerSet;
class UDC1TechTreeComponent;
class UDC1PlayerInventoryComponent;
class UDC1ValuableInventoryComponent;
class UDC1PalPartyComponent;
class UDC1PalSphereComponent;
class UDC1EquipComponent;
class UDC1AppearanceComponent;
class UDC1TeleportComponent;
class UDC1PlayerStatComponent;
class UDC1NotificationComponent;

class UDC1PlayerSaveGame;

/**
 * 
 */
UCLASS()
class DC1_API ADC1PlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
public:
	ADC1PlayerState(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent*	GetAbilitySystemComponent() const override;
	UDC1TechTreeComponent*				GetTechTreeComponent() const { return TechTreeComponent; }
	UDC1PlayerInventoryComponent*		GetInventoryComponent() const { return InventoryComponent; }
	UDC1ValuableInventoryComponent*		GetValuableInventoryComponent() const { return ValuableInventoryComponent; }
	UDC1PalPartyComponent*				GetPalPartyComponent() const { return PalPartyComponent; }
	UDC1PalSphereComponent*				GetPalSphereComponent() const { return PalSphereComponent; }
	UDC1EquipComponent*					GetEquipComponent() const { return EquipComponent; }
	UDC1AppearanceComponent*			GetAppearanceComponent() const { return AppearanceComponent; }
	UDC1TeleportComponent*				GetTeleportComponent() const { return TeleportComponent; }
	UDC1PlayerStatComponent*			GetStatComponent() const { return StatComponent; }
	UDC1NotificationComponent*			GetNotificationComponent() const { return NotificationComponent; }

public:
	void WriteSaveData(UDC1PlayerSaveGame* Out) const;
	void ReadSaveData(const UDC1PlayerSaveGame* In);

public:
	// 부활 전 상태 초기화 ( 서버 ) - 사망 태그 제거, 체력 InHealthRatio, 스태미나 최대
	void ResetForRespawn(float InHealthRatio);

private:
	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent> AbilitySystemComponent = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1PlayerSet> PlayerSet = { nullptr };
	
	UPROPERTY(VisibleAnywhere, Category = "Tech")
	TObjectPtr< UDC1TechTreeComponent>		TechTreeComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UDC1PlayerInventoryComponent> InventoryComponent = { nullptr };

	// 귀중품 전용 인벤토리. 가진 종류 수만큼만 칸이 생기고 무게에 포함되지 않는다
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UDC1ValuableInventoryComponent> ValuableInventoryComponent = { nullptr };

	// 잡은 팰 목록. 아이템 슬롯과 구조가 달라 따로 둔다
	UPROPERTY(VisibleAnywhere, Category = "Pal")
	TObjectPtr<UDC1PalPartyComponent> PalPartyComponent = { nullptr };

	// 던질 팰스피어 등급 목록과 선택
	UPROPERTY(VisibleAnywhere, Category = "Pal")
	TObjectPtr<UDC1PalSphereComponent> PalSphereComponent = { nullptr };

	// 장착 정보는 폰보다 오래 살아야 한다. 죽어서 드랍할 때 무엇을 들고 있었는지 읽어야 하기 때문
	UPROPERTY(VisibleAnywhere, Category = "Equip")
	TObjectPtr<UDC1EquipComponent> EquipComponent = { nullptr };

	// 폰보다 오래 살아야 함 + 추후 미리보기 액터도 같은 인스턴스를 공유해서 써야 함
	UPROPERTY(VisibleAnywhere, Category = "Appearance")
	TObjectPtr<UDC1AppearanceComponent> AppearanceComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Teleport")
	TObjectPtr<UDC1TeleportComponent> TeleportComponent = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Stat")
	TObjectPtr<UDC1PlayerStatComponent> StatComponent = { nullptr };

	// 서버 판정(획득, 쓰러뜨림)을 이 플레이어 화면 알림으로 보낸다
	UPROPERTY(VisibleAnywhere, Category = "Notification")
	TObjectPtr<UDC1NotificationComponent> NotificationComponent = { nullptr };
};
