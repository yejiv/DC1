// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalPartyComponent.generated.h"

class ADC1PalCharacter;

DECLARE_MULTICAST_DELEGATE(FOnPalSlotTranslated);
DECLARE_MULTICAST_DELEGATE(FOnPalSlotChanged);
/**
 *
 * 플레이어가 데리고 다니는 팰 목록. ADC1PlayerState에 붙는다.
 * 꺼내둔 팰은 목록에 남아있음
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1PalPartyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1PalPartyComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 스피어가 잡아온 것을 받는다. OnPalCaptured에 바인딩하는 진입점
	void					HandlePalCaptured(const FDC1PalInstance& InPal);

	// 소환/회수 토글. 입력은 클라에서 들어오므로 서버로 넘긴다
	UFUNCTION(Server, Reliable)
	void					Server_ToggleDeploy();

	// 선택 슬롯을 한 칸 옮긴다. 정원 전체를 돌고 끝에서 감싼다.
	// 입력이 클라에서 들어오므로 로그도 클라에서 찍고, 서버에는 인덱스만 보낸다
	void					CycleSlot(int32 InDelta);

public:
	int32					GetCapacity() const { return Capacity; }
	int32					GetSelectedSlot() const { return SelectedSlot; }
	const FDC1PalInstance*	GetPalInfo(int32 Index) const;

	float					GetPalMaxHealth(int32 InSlotIndex) const;

	// 칸이 찼나. 죽은 팰도 칸은 차지한다
	bool					HasPalAt(int32 InSlotIndex) const;

	// 살아 있나. 체력 0 이면 false
	bool					IsPalAlive(int32 InSlotIndex) const;

	// 지금 월드에 나와 있나. 이건 데이터가 아니라 액터와 대조해야 안다
	bool					IsDeployed(int32 InSlotIndex) const;

public:
	// 팰 슬롯 변화 시 호출
	FOnPalSlotTranslated	OnPalSlotTranslated;

	// 팰 등록 변화 시 호출
	FOnPalSlotChanged		OnPalSlotChanged;

protected:
	virtual void			BeginPlay() override;

	// Pals 복제 수신 시점. 클라에서만 불린다.
	// UI 알림은 여기서 시작하면 된다
	UFUNCTION()
	void					OnRep_Pals();

	UFUNCTION()
	void					OnRep_DeployedPal();

	void					ToggleDeploy();

	// 소환은 서버가 하므로 선택 슬롯도 서버가 알아야 한다
	UFUNCTION(Server, Reliable)
	void					Server_SetSelectedSlot(int32 InSlotIndex);

	// 슬롯 하나를 한 줄로 찍는다. UI가 붙기 전까지의 확인 수단
	void					LogSlot(int32 InSlotIndex) const;

	// 회수 한 번. 목록 반영 + 액터 파괴 + 예약된 타이머 정리.
	// E 회수와 사망 회수가 같은 일을 하므로 공유한다
	void					RetrieveDeployed();

	// 꺼내둔 팰의 OnPalDied 진입점. 래그돌을 잠깐 보여주고 회수한다
	void					HandleDeployedPalDied();

	// 회수한 상태를 목록에 반영한다. PalID로 찾아 덮어쓴다
	void					UpdatePal(const FDC1PalInstance& InPal);

	// 주인 앞 NavMesh 위. 이 컴포넌트가 위치를 정하는 건 주인이 누군지 알기 때문이다
	bool					FindDeployTransform(FTransform& OutTransform) const;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Pals)
	TArray<FDC1PalInstance>	Pals;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Party", meta = (ClampMin = "1"))
	int32					Capacity = { 5 };

	// 주인과의 거리
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Party", meta = (ClampMin = "0.0"))
	float					DeployDistance = { 300.f };

	// 죽고 나서 회수까지의 유예. 이 시간 동안 래그돌이 보인다.
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Party", meta = (ClampMin = "0.1"))
	float					DeathRetrieveDelay = { 2.f };

private:
	// 꺼내둔 팰. 회수하려면 액터를 붙들고 있어야 한다.
	UPROPERTY(ReplicatedUsing = OnRep_DeployedPal)
	TObjectPtr<ADC1PalCharacter>		DeployedPal = { nullptr };

	FTimerHandle						DeathRetrieveTimer;

	// 선택된 슬롯. 클라는 입력으로, 서버는 Server_SetSelectedSlot으로 각자 갱신한다
	int32								SelectedSlot = { 0 };
};
