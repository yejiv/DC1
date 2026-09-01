// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalPartyComponent.generated.h"

class ADC1PalCharacter;

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

protected:
	void					ToggleDeploy();

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
	UPROPERTY(Replicated)
	TArray<FDC1PalInstance>	Pals;

	UPROPERTY(EditDefaultsOnly, Category = "Pal|Party", meta = (ClampMin = "1"))
	int32					Capacity = { 5 };

	// 주인과의 거리
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Party", meta = (ClampMin = "0.0"))
	float					DeployDistance = { 300.f };

	// 죽고 나서 회수까지의 유예. 이 시간 동안 래그돌이 보인다.
	// SetTimer는 0 이하를 타이머 해제로 처리하므로 0을 허용하지 않는다
	UPROPERTY(EditDefaultsOnly, Category = "Pal|Party", meta = (ClampMin = "0.1"))
	float					DeathRetrieveDelay = { 2.f };

private:
	// 꺼내둔 팰. 회수하려면 액터를 붙들고 있어야 한다. 서버에서만 갱신된다
	TWeakObjectPtr<ADC1PalCharacter>	DeployedPal;

	FTimerHandle						DeathRetrieveTimer;
};
