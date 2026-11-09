// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1BaseInfoViewModel.generated.h"

class ADC1PlayerState;
class UDC1PalPartyComponent;
class UTexture2D;
struct FDC1PalInstance;

DECLARE_MULTICAST_DELEGATE(FOnBaseInfoChanged);

// 거점 정보 HUD 한 줄의 상태 문구. 위에서부터 우선
UENUM()
enum class EDC1BasePalCondition : uint8
{
	Resting,	// 자는 중 · 기절
	Tired,		// SAN 이 TiredSanityThreshold 미만
	Capable,	// 작업속도 배율 > 1 인 패시브가 있음
	Healthy,
};

USTRUCT()
struct FDC1BaseInfoRow
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UTexture2D>	Icon = { nullptr };

	UPROPERTY()
	EDC1BasePalCondition	Condition = { EDC1BasePalCondition::Healthy };

	// 일하는 중 ( 망치 아이콘 )
	UPROPERTY()
	bool					bWorking = { false };
};

/**
 * 거점 정보 HUD 용 ViewModel.
 * 로컬 플레이어가 거점 범위 안에 있으면 거점 칸의 팰마다 한 줄을 만든다. 거점은 서버 공용이라 주인과 무관
 */
UCLASS()
class DC1_API UDC1BaseInfoViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	bool							IsInBase() const { return bInBase; }
	int32							GetPalCount() const { return PalCount; }
	int32							GetCapacity() const { return Capacity; }
	const TArray<FDC1BaseInfoRow>&	GetRows() const { return Rows; }

public:
	FOnBaseInfoChanged				OnBaseInfoChanged;

private:
	// 범위 들어옴 / 나감 확인 ( 타이머 )
	void							CheckInBase();

	// 거점 칸 · 상태가 바뀔 때
	void							Refresh();

	EDC1BasePalCondition			FindCondition(const FDC1PalInstance& InPal, bool InSleeping) const;

private:
	TWeakObjectPtr<ADC1PlayerState>			PlayerState = { nullptr };
	TWeakObjectPtr<UDC1PalPartyComponent>	PalPartyComponent = { nullptr };

	FTimerHandle							InBaseCheckTimer;

	bool									bInBase = { false };
	int32									PalCount = { 0 };
	int32									Capacity = { 0 };

	UPROPERTY()
	TArray<FDC1BaseInfoRow>					Rows;
};
