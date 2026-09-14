// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1EquipViewModel.generated.h"

class UDC1EquipComponent;
class UDC1PlayerInventoryComponent;
class ADC1Weapon;
class UTexture2D;

DECLARE_MULTICAST_DELEGATE(FOnWeaponChanged);
DECLARE_MULTICAST_DELEGATE(FOnAmmoChanged);

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

	// 장전된 탄. 예비탄은 인벤토리 연동 때 따로 받는다
	int32						GetLoadedAmmo() const;

public:
	FOnWeaponChanged		OnWeaponChanged;

	// 장탄(무기)이나 예비탄(인벤토리)이 바뀔 때. 무기 교체는 OnWeaponChanged가 맡는다
	FOnAmmoChanged		OnAmmoChanged;

private:
	void HandleWeaponChanged();
	void HandleAmmoChanged();

private:
	TWeakObjectPtr<UDC1EquipComponent>				EquipComponent = { nullptr };

	// 팰스피어는 장착이 아니라 인벤토리 보유량으로 표시한다. 실제 배선은 팰스피어 로직 이후
	TWeakObjectPtr<UDC1PlayerInventoryComponent>	InventoryComponent = { nullptr };
};