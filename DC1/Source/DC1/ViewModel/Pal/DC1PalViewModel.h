// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "Define/DC1Enums.h"
#include "GameplayTagContainer.h"
#include "ViewModel/Pal/DC1PalDisplayTypes.h"
#include "DC1PalViewModel.generated.h"

class UDC1PalPartyComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPalSlotShifted, EDC1PalSlotShift);
DECLARE_MULTICAST_DELEGATE(FOnPalInfoChanged);

/**
 * 팰 HUD용 ViewModel.
 * PalPartyComponent의 선택 슬롯 변경을 구독해 이동 방향을 판별한 뒤 위젯에 전달한다.
 * 방향 판별은 한 번에 한 칸씩만 바뀌고 Capacity가 3 이상이라는 전제에 기댄다
 */
UCLASS()
class DC1_API UDC1PalViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	UTexture2D*				GetPalIconTexture(int32 Offset);
	bool						GetPalAlive(int32 Offset);
	void						GetPalInfo(FDC1PalDisplayInfo& OutDisplayInfo);

	// 위 함수들과 달리 선택 슬롯 오프셋이 아닌 슬롯 번호 기준. 빈 칸이면 EmptyTag
	// DC1PalInfoWidget에서 Preview 모델을 그리기 위해 구현
	FGameplayTag			GetSpeciesTag(int32 InSlotIndex) const;

	bool						IsDeployed();

public:
	FOnPalSlotShifted		OnSlotShifted;
	FOnPalInfoChanged	OnInfoChanged;

private:
	// 현재 선택된 슬롯 인덱스 변경
	void HandleSelectedSlotChanged();

	// 파티 내 팰 변경
	void HandlePalChanged();

private:
	TWeakObjectPtr<UDC1PalPartyComponent>	PalPartyComponent = { nullptr };

private:
	// 방향 판별용 직전 선택 슬롯
	int32											PrevSlot = { 0 };

	// 게임 중 안 바뀌므로 BindSource에서 한 번 캐싱
	int32											Capacity = { 0 };

private:
	// Pal Icon Cached
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>>		CachedPalIcons;
};