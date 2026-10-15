#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "Define/DC1Enums.h"
#include "Component/Player/DC1PlayerStatComponent.h"
#include "DC1PlayerStatsViewModel.generated.h"

class UAbilitySystemComponent;

struct FOnAttributeChangeData;

DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnPlayerStatChanged, EDC1PlayerStat /*Stat*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FDC1OnRemainingStatPointsChanged, int32 /*RemainingPoints*/);

/**
 * 인벤토리 스탯 패널용 ViewModel.
 * 스탯 값은 ASC 어트리뷰트에서, 포인트와 투자 횟수는 PlayerStatComponent에서 읽는다.
 * 강화는 임시 배분(Pending)을 모았다가 ConfirmPending으로 한 번에 서버에 보낸다.
 */
UCLASS()
class DC1_API UDC1PlayerStatsViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	// 값 또는 임시 배분이 바뀐 스탯
	FDC1OnPlayerStatChanged				OnStatChanged;

	// 임시 배분을 뺀 남은 포인트
	FDC1OnRemainingStatPointsChanged	OnRemainingStatPointsChanged;

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	// 장비 등 GE 보정이 포함된 표시값
	float	GetStatValue(EDC1PlayerStat InStat) const;
	float	GetStatBaseValue(EDC1PlayerStat InStat) const;

	// 표시값이 Base보다 높으면 ▲ 표시
	bool	IsStatBoosted(EDC1PlayerStat InStat) const;

	bool	IsEnhanceableStat(EDC1PlayerStat InStat) const;
	int32	GetInvestedCount(EDC1PlayerStat InStat) const;
	float	GetEnhanceAmount(EDC1PlayerStat InStat) const;

	int32	GetRemainingStatPoints() const;
	int32	GetPendingCount(EDC1PlayerStat InStat) const { return PendingStats.GetCount(InStat); }
	float	GetPendingAmount(EDC1PlayerStat InStat) const { return GetPendingCount(InStat) * GetEnhanceAmount(InStat); }
	bool	HasPending() const { return 0 < PendingStats.GetTotal(); }
	bool	CanAddPending(EDC1PlayerStat InStat) const;
	bool	CanRemovePending(EDC1PlayerStat InStat) const { return 0 < GetPendingCount(InStat); }

	void	AddPending(EDC1PlayerStat InStat);
	void	RemovePending(EDC1PlayerStat InStat);
	void	ConfirmPending();
	void	CancelPending();

private:
	void	HandleStatAttributeChanged(const FOnAttributeChangeData& InData, EDC1PlayerStat InStat);

	// 포인트가 바뀌면 모든 행의 [+] 가능 여부도 바뀌므로 스탯 전체를 다시 알린다
	void	HandleStatPointsChanged();

	void	BroadcastAll();

	int32	GetAvailableStatPoints() const;
	FGameplayAttribute GetDisplayAttribute(EDC1PlayerStat InStat) const;

private:
	TWeakObjectPtr<UAbilitySystemComponent>	AbilitySystemComponent = { nullptr };
	TWeakObjectPtr<UDC1PlayerStatComponent>	StatComponent = { nullptr };

	// ClearSource 시점엔 StatComponent가 먼저 사라질 수 있어 구독한 어트리뷰트를 따로 기억
	TArray<FGameplayAttribute>	SubscribedAttributes;

	// 확정 전 임시 배분 (로컬 전용)
	FDC1StatInvestment PendingStats;
};
