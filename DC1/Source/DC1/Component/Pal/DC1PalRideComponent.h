// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "DC1PalRideComponent.generated.h"

class ADC1PalCharacter;
class UDC1PalData;

DECLARE_MULTICAST_DELEGATE(FOnRideStateChanged);

/**
 *
 * 팰 탑승 상태. ADC1PalCharacter 에 붙는다.
 * 탑승 단계·방향 복제, 탄 사람 몽타주와 위치(옆 ↔ 등), 탑승 중 스킬 회전
 * 단계 진행(언제 타고 내리나)은 파티 컴포넌트가 정한다
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1PalRideComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1PalRideComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 탄 사람 애니(ABP)가 읽는다. 탄 사람은 GetRiddenPawn 으로 이 팰을 얻는다
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pal|Ride")
	EDC1RideState					GetRideState() const { return RideState; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pal|Ride")
	EDC1RideDirection				GetRideDirection() const { return RideDirection; }

	// 서버. 파티가 탑승 단계를 바꿀 때 부른다
	void							SetRideState(EDC1RideState InState, EDC1RideDirection InDirection);

	// 탄 사람 = 이 팰에 붙어 있고 GetRiddenPawn 이 이 팰인 플레이어. 둘 다 복제되므로 클라에서도 찾을 수 있다
	APawn*							GetRider() const;

	// 팰 정면 기준 InLocation 이 있는 쪽
	EDC1RideDirection				FindRideDirection(const FVector& InLocation) const;

	// 올라타기 시작 / 내리기 끝 자리. 탑승 소켓에서 RideDirection 쪽으로 띄운 곳
	FVector							GetRideSideLocation() const;

	// DC1 Ride Move 노티파이 구간. 이 동안 탄 사람을 옆 ↔ 소켓으로 옮긴다 (각 기계)
	void							BeginRideMove(float InDuration);
	void							EndRideMove();

	// 탑승 중 조작 막음. 올라타기·내리기 중, 스킬 전 회전 중, 스킬 중
	bool							IsRideActionLocked() const;

	// 탑승 중 스킬 (탄 사람 클라). 카메라 방향으로 짧게 돈 뒤 서버에 발동을 요청. InSkillIndex 는 PalData.SkillTags 순서
	void							RequestRideSkill(int32 InSkillIndex);

public:
	// 탑승 단계가 바뀜 (각 기계). 라이딩 HUD 가 구독한다
	FOnRideStateChanged				OnRideStateChanged;

protected:
	UFUNCTION()
	void							OnRep_RideState();

	void							TickRideMove();

	// 0 이면 출발 자리, 1 이면 도착 자리. 올라타기는 옆 → 소켓, 내리기는 소켓 → 옆
	void							PlaceRider(float InAlpha);

	void							TickRideSkillTurn();

	UFUNCTION(Server, Reliable)
	void							Server_UseRideSkill(const FGameplayTag& InSkillTag);

	ADC1PalCharacter*				GetPal() const;
	const UDC1PalData*				GetPalData() const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_RideState)
	EDC1RideState					RideState = { EDC1RideState::None };

	UPROPERTY(Replicated)
	EDC1RideDirection				RideDirection = { EDC1RideDirection::Left };

	// 노티파이 구간. 각 기계가 자기 애니 기준으로 센다
	float							RideMoveStartTime = { 0.f };
	float							RideMoveDuration = { 0.f };

	FTimerHandle					RideMoveTimer;

	// 탑승 중 스킬을 쓰기 전 카메라 방향으로 도는 시간
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Ride", meta = (ClampMin = "0.0"))
	float							RideSkillTurnTime = { 0.15f };

	FTimerHandle					RideSkillTurnTimer;
	FRotator						RideSkillTurnFrom = { FRotator::ZeroRotator };
	FRotator						RideSkillTurnTo = { FRotator::ZeroRotator };
	float							RideSkillTurnStartTime = { 0.f };
	FGameplayTag					PendingRideSkillTag;
};
