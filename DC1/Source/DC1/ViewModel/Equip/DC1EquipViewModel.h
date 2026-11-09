// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1EquipViewModel.generated.h"

class UDC1EquipComponent;
class UDC1PlayerInventoryComponent;
class UDC1PalSphereComponent;
class ADC1Weapon;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE(FOnWeaponChanged);
DECLARE_MULTICAST_DELEGATE(FOnAmmoChanged);
DECLARE_MULTICAST_DELEGATE(FOnSphereChanged);

/**
 * 장비 HUD용 ViewModel.
 * PlayerState의 EquipComponent 변경을 구독해 표시용 값으로 가공한 뒤 위젯에 전달한다.
 */
UCLASS()
class DC1_API UDC1EquipViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	UTexture2D*					GetCurrentWeaponIcon();

	// 무기가 없으면 false. HUD는 이 값으로 잔탄 카운터와 무한대 표시를 고른다
	bool						UsesAmmo() const;

	// 장전 표시. 실제 장전은 없고 보유 탄이 있으면 1, 없으면 0
	int32						GetLoadedAmmo() const;

	// 들고 있는 무기의 탄(AmmoItemID) 인벤토리 보유 수
	int32						GetReserveAmmo() const;

	// 고른 스피어 등급(2 키)의 인벤토리 보유 수
	int32						GetSelectedSphereCount() const;

	// 고른 스피어 등급의 아이템 아이콘. 없으면 nullptr
	UTexture2D*					GetSelectedSphereIcon() const;

public:
	FOnWeaponChanged		OnWeaponChanged;

	// 장탄(무기)이나 예비탄(인벤토리)이 바뀔 때. 무기 교체는 OnWeaponChanged가 맡는다
	FOnAmmoChanged		OnAmmoChanged;

	// 스피어 등급 선택이나 보유 수가 바뀔 때
	FOnSphereChanged	OnSphereChanged;

private:
	void HandleWeaponChanged();
	void HandleAmmoChanged();
	void HandleInventoryChanged();
	void HandleSphereSelectionChanged();

private:
	TWeakObjectPtr<UDC1EquipComponent>				EquipComponent = { nullptr };

	// 화살·팰스피어는 장착이 아니라 인벤토리 보유량으로 표시한다
	TWeakObjectPtr<UDC1PlayerInventoryComponent>	InventoryComponent = { nullptr };

	// 스피어 등급 목록·선택
	TWeakObjectPtr<UDC1PalSphereComponent>			PalSphereComponent = { nullptr };
};