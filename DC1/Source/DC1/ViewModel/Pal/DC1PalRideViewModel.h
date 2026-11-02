// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1PalRideViewModel.generated.h"

class UDC1PalPartyComponent;
class UDC1PalRideComponent;
class UDC1PalData;

DECLARE_MULTICAST_DELEGATE(FOnPalRideInfoChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPalRideHoldChanged, bool);
DECLARE_MULTICAST_DELEGATE(FOnPalRideHoldCompleted);

/**
 * 라이딩 HUD용 ViewModel.
 * 파티의 소환 팰과 그 팰의 탑승 단계를 구독하고, F 홀드 입력은 PlayerController 가 넣어 준다
 */
UCLASS()
class DC1_API UDC1PalRideViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	// 탈 수 있는 팰이 소환되어 있다
	bool					IsRideAvailable() const { return bRideAvailable; }

	// 올라타는 중 ~ 다 내릴 때까지
	bool					IsRiding() const { return bRiding; }

	// PalData.SkillTags 순서. 탑승 스킬 입력 순서와 같다
	const TArray<FText>&	GetRideSkillNames() const { return RideSkillNames; }

	// F 를 누름 / 뗌 (PlayerController)
	void					SetRideHoldPressed(bool InPressed);

	// F 를 끝까지 누름 (PlayerController)
	void					NotifyRideHoldCompleted();

public:
	FOnPalRideInfoChanged	OnRideInfoChanged;
	FOnPalRideHoldChanged	OnRideHoldChanged;
	FOnPalRideHoldCompleted	OnRideHoldCompleted;

private:
	// 소환 팰이 바뀌면 탑승 컴포넌트 구독을 옮긴다
	void					HandleDeployedPalChanged();
	void					HandleRideStateChanged();

	void					RefreshRideSkillNames(const UDC1PalData* InPalData);

private:
	TWeakObjectPtr<UDC1PalPartyComponent>	PalPartyComponent = { nullptr };
	TWeakObjectPtr<UDC1PalRideComponent>	RideComponent = { nullptr };

	bool									bRideAvailable = { false };
	bool									bRiding = { false };
	TArray<FText>							RideSkillNames;
};
