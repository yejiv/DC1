// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1PalRideViewModel.generated.h"

class ADC1PalCharacter;
class UAbilitySystemComponent;
class UDC1PalPartyComponent;
class UDC1PalRideComponent;
class UDC1PalData;

DECLARE_MULTICAST_DELEGATE(FOnPalRideInfoChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPalRideHoldChanged, bool);
DECLARE_MULTICAST_DELEGATE(FOnPalRideHoldCompleted);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPalRideSkillCooldownChanged, int32, bool);

// 탑승 스킬 하나의 쿨타임.
// 팰 ASC 는 Minimal 복제라 클라에는 쿨타임 태그만 온다. 남은 시간은 태그가 붙은 때부터 표의 쿨타임으로 센다
struct FDC1RideSkillCooldown
{
	FGameplayTag	CooldownTag;
	float			Duration = { 0.f };
	float			StartTime = { 0.f };
	bool			bActive = { false };
};

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

	// 탑승 스킬 쿨타임 중. InIndex 는 GetRideSkillNames 순서
	bool					IsRideSkillCoolingDown(int32 InIndex) const;

	// 0 = 막 씀, 1 = 다 참. 쿨타임이 아니면 1
	float					GetRideSkillCooldownRatio(int32 InIndex) const;

	// F 를 누름 / 뗌 (PlayerController)
	void					SetRideHoldPressed(bool InPressed);

	// F 를 끝까지 누름 (PlayerController)
	void					NotifyRideHoldCompleted();

public:
	FOnPalRideInfoChanged	OnRideInfoChanged;
	FOnPalRideHoldChanged	OnRideHoldChanged;
	FOnPalRideHoldCompleted	OnRideHoldCompleted;

	// 탑승 스킬 쿨타임 시작(true) / 끝(false). 스킬 순서 번호
	FOnPalRideSkillCooldownChanged	OnRideSkillCooldownChanged;

private:
	// 소환 팰이 바뀌면 탑승 컴포넌트 구독을 옮긴다
	void					HandleDeployedPalChanged();
	void					HandleRideStateChanged();

	void					RefreshRideSkillNames(const UDC1PalData* InPalData);

	// 소환 팰 ASC 의 쿨타임 태그를 구독한다. 같은 ASC 면 그대로 둔다
	void					RefreshRideSkillCooldowns(const ADC1PalCharacter* InPal, const UDC1PalData* InPalData);
	void					UnbindRideSkillCooldowns();
	void					HandleCooldownTagChanged(const FGameplayTag InTag, int32 InNewCount);

	float					GetWorldTime() const;

private:
	TWeakObjectPtr<UDC1PalPartyComponent>	PalPartyComponent = { nullptr };
	TWeakObjectPtr<UDC1PalRideComponent>	RideComponent = { nullptr };

	bool									bRideAvailable = { false };
	bool									bRiding = { false };
	TArray<FText>							RideSkillNames;

	TWeakObjectPtr<UAbilitySystemComponent>	PalAbilitySystemComponent = { nullptr };
	TArray<FDC1RideSkillCooldown>			RideSkillCooldowns;

	// 구독한 쿨타임 태그와 핸들 (같은 순서, 태그당 한 번)
	TArray<FGameplayTag>					BoundCooldownTags;
	TArray<FDelegateHandle>					CooldownTagHandles;
};
