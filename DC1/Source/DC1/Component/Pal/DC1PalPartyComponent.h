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

private:
	// 꺼내둔 팰. 회수하려면 액터를 붙들고 있어야 한다. 서버에서만 갱신된다
	TWeakObjectPtr<ADC1PalCharacter>	DeployedPal;
};
