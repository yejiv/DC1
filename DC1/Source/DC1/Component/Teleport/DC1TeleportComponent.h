// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DC1TeleportComponent.generated.h"

/**
 *
 * 텔레포트 지점 해금 목록. ADC1PlayerState에 붙는다.
 *
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1TeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1TeleportComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	const TArray<FName>&	GetUnlockedPointIds() const { return UnlockedPointIds; }
	bool					IsPointUnlocked(FName InPointId) const;

	// 텔레포트 오브젝트 해금
	UFUNCTION(Server, Reliable)
	void					Server_UnlockPoint(AActor* InPoint);

	// ID로 해금. 서버 전용 ( 검증은 호출하는 쪽 책임 - Server_UnlockPoint, 거점 완성 등 )
	void					UnlockPointById(FName InPointId);

	// InSource 지점에서 InDest 지점으로 순간이동
	UFUNCTION(Server, Reliable)
	void					Server_RequestTeleport(AActor* InSource, AActor* InDest);

	// Teleport 종료 후 처리 ( 성공 / 실패 모두 호출 -> FadeOut 보장 )
	UFUNCTION(Client, Reliable)
	void					Client_OnTeleportFinished(bool bInSuccess);

private:
	UFUNCTION()
	void					OnRep_UnlockedPointIds();

private:
	// 텔레포트 검증 + 이동 ( 서버 )
	bool					TryTeleport(AActor* InSource, AActor* InDest);

	// 상호작용 허용 거리 내인지 판단
	bool					IsInnerInteractRange(AActor* InPoint) const;

private:
	// 해금된 텔레포트 지점 ID ( 세이브 키 )
	UPROPERTY(ReplicatedUsing = OnRep_UnlockedPointIds)
	TArray<FName>			UnlockedPointIds;

	// 서버 검증용 지점 상호작용 허용 거리 ( 네트워크 지연을 감안해 상호작용 범위보다 넉넉하게 )
	UPROPERTY(EditDefaultsOnly, Category = "Teleport", meta = (ClampMin = "0.0"))
	float					InteractAllowDistance = { 500.f };
};
