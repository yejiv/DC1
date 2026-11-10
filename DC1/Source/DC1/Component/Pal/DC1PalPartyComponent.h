// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalPartyComponent.generated.h"

class ADC1PalCharacter;
class ADC1PalBoxActor;
class ADC1PalAIController;
class UDC1PalBaseComponent;
class ACharacter;
class AController;
class APlayerController;

DECLARE_MULTICAST_DELEGATE(FOnPalSlotTranslated);
DECLARE_MULTICAST_DELEGATE(FOnPalSlotChanged);
DECLARE_MULTICAST_DELEGATE(FOnPalBoxChanged);
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

	// 처치·포획 경험치. 파티 전원 전액. 서버 전용. InExpRate = 알파 배율 (일반 1)
	void					GrantExperienceFor(int32 InDefeatedLevel, float InExpRate);

	// 소환/회수 토글. 입력은 클라에서 들어오므로 서버로 넘긴다. 소환할 칸은 클라가 고른 번호를 같이 보낸다
	UFUNCTION(Server, Reliable)
	void					Server_ToggleDeploy(int32 InSlotIndex);

	// 탑승/하차. 달려오는 중이면 취소
	UFUNCTION(Server, Reliable)
	void					Server_ToggleRide();

	// 달려오기 취소. 팰이 맞거나 죽거나 접힐 때. (InResumeBehavior : 트리 활성화 여부)
	void					StopRideApproach(bool InResumeBehavior);

	// 선택 슬롯을 한 칸 옮긴다. 정원 전체를 돌고 끝에서 감싼다.
	// 클라 전용 값이다. 서버는 소환 요청(Server_ToggleDeploy)에 실려 온 번호를 쓴다
	void					CycleSlot(int32 InDelta);

	// 두 칸 교환. 소환 중인 팰이 상자로 가면 먼저 회수
	UFUNCTION(Server, Reliable)
	void					Server_MovePal(EDC1PalContainer InFrom, int32 InFromIndex, EDC1PalContainer InTo, int32 InToIndex);

	// 파티·상자에 빈칸이 있나
	bool					CanAcceptPal() const;

	// 주인 기절. 하차만 ( 달려오기 취소 포함 ), 팰은 그대로 둔다. 서버 전용
	void					HandleOwnerDowned();

	// 주인 사망. 탑승 해제 + 소환 팰 회수. 서버 전용
	void					HandleOwnerDied();

public:
	// 위치 지정 조회
	const FDC1PalInstance*	GetPalInfo(EDC1PalContainer InContainer, int32 InSlotIndex) const;
	float					GetPalHealth(EDC1PalContainer InContainer, int32 InSlotIndex) const;
	float					GetPalMaxHealth(EDC1PalContainer InContainer, int32 InSlotIndex) const;
	int32					GetBoxCapacity() const { return BoxCapacity; }
	// 거점 칸은 서버 공용 ( GS )
	int32					GetBaseCapacity() const;

	// 최대 체력. PalBaseComponent도 사용
	static float			GetMaxHealthOf(const FDC1PalInstance& InPal);

public:
	int32					GetCapacity() const { return Capacity; }
	int32					GetSelectedSlot() const { return SelectedSlot; }
	const FDC1PalInstance*	GetPalInfo(int32 Index) const;

	float					GetPalMaxHealth(int32 InSlotIndex) const;

	// 현재 체력. 소환 중이면 액터 값
	float					GetPalHealth(int32 InSlotIndex) const;

	// 칸이 찼나. 죽은 팰도 칸은 차지한다
	bool					HasPalAt(int32 InSlotIndex) const;

	// 살아 있나. 체력 0 이면 false
	bool					IsPalAlive(int32 InSlotIndex) const;

	// 지금 월드에 나와 있나. 이건 데이터가 아니라 액터와 대조해야 안다
	bool					IsDeployed(int32 InSlotIndex) const;

	// 소환 중인 팰. 없으면 nullptr
	ADC1PalCharacter*		GetDeployedPal() const { return DeployedPal; }

	// 거점 조회 ( GS )
	bool					HasBase() const;

	ADC1PalBoxActor*		GetBasePalBox() const;

	// 거점 칸이 바뀐 시점 ( 서버 직접 / 클라 OnRep )
	void					HandleBaseChanged();

	// 거점에서 돌아온 팰 받기 - [ 우선순위 : 팰 상자 -> 파티 빈칸 ] ( 서버 ), 둘 다 가득이면 false
	bool					ReceivePal(const FDC1PalInstance& InPal);

public:
	void ExportSaveData(TArray<FDC1PalInstance>& Out) const;
	void ImportSaveData(const TArray<FDC1PalInstance>& In);

public:
	// 팰 슬롯 변화 시 호출
	FOnPalSlotTranslated		OnPalSlotTranslated;

	// 팰 등록 변화 시 호출
	FOnPalSlotChanged		OnPalSlotChanged;

	// 상자·거점 변화
	FOnPalBoxChanged		OnPalBoxChanged;

protected:
	virtual void			BeginPlay() override;

	// Pals 복제 수신 시점. 클라에서만 불린다.
	// UI 알림은 여기서 시작하면 된다
	UFUNCTION()
	void					OnRep_Pals();

	UFUNCTION()
	void					OnRep_DeployedPal();

	UFUNCTION()
	void					OnRep_BoxPals();

	// 상자 회복 타이머 ( 거점 회복은 UDC1PalBaseComponent )
	void					HealStoredPals();

	// Base는 거점 PalBox의 공용 배열
	TArray<FDC1PalInstance>&	GetContainer(EDC1PalContainer InContainer);
	const TArray<FDC1PalInstance>&	GetContainer(EDC1PalContainer InContainer) const;

	UDC1PalBaseComponent*	GetBasePalComponent() const;

	void					BroadcastChanged(EDC1PalContainer InContainer);

	void					ToggleDeploy(int32 InSlotIndex);

	// 회수 한 번. 목록 반영 + 액터 파괴 + 예약된 타이머 정리.
	// E 회수와 사망 회수가 같은 일을 하므로 공유한다
	void					RetrieveDeployed();

	// 꺼내둔 팰의 OnPalDied 진입점. 래그돌을 잠깐 보여주고 회수한다
	void					HandleDeployedPalDied();

	void					BeginRideApproach();

	void					HandleRideApproachFinished(bool InArrived);

	// 플레이어 이동 제한
	UFUNCTION(Client, Reliable)
	void					Client_SetRideApproachInputBlocked(bool InBlocked);

	// 옆에서 올라타기 시작. 다 오르면 FinishMount
	void					Ride();
	void					FinishMount();

	// F 로 내리기. 다 내리면 EndRide
	void					BeginDismount();

	// 즉시 하차. (InReturnPalToAI false 면 AI 컨트롤러 파괴)
	void					EndRide(bool InReturnPalToAI);

	APlayerController*		GetOwnerController() const;
	ADC1PalAIController*	GetDeployedPalController() const;

	// 회수한 상태를 목록에 반영한다. PalID로 파티·상자·거점에서 찾아 덮어쓴다
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

	// 캡슐 표면 간 거리
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Ride", meta = (ClampMin = "0.0"))
	float					RideAcceptanceRadius = { 30.f };

	// 서버 전용
	UPROPERTY()
	TObjectPtr<ACharacter>	Rider = { nullptr };

	// 탑승 중 비켜 둔 AI. 서버 전용
	UPROPERTY()
	TObjectPtr<AController>	RiddenPalController = { nullptr };

	// 상자
	UPROPERTY(ReplicatedUsing = OnRep_BoxPals)
	TArray<FDC1PalInstance>	BoxPals;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Box", meta = (ClampMin = "1"))
	int32					BoxCapacity = { 30 };

	// 회복 주기(초)
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Box", meta = (ClampMin = "0.1"))
	float					BoxHealInterval = { 1.f };

	FTimerHandle			BoxHealTimer;

private:
	// 꺼내둔 팰. 회수하려면 액터를 붙들고 있어야 한다.
	UPROPERTY(ReplicatedUsing = OnRep_DeployedPal)
	TObjectPtr<ADC1PalCharacter>		DeployedPal = { nullptr };

	FTimerHandle						DeathRetrieveTimer;

	// 올라타기·내리기가 끝나는 시점
	FTimerHandle						RideStepTimer;

	// 선택된 슬롯. 클라 입력(CycleSlot)으로만 바뀐다. 복제·서버 동기화 없음
	int32								SelectedSlot = { 0 };
};
