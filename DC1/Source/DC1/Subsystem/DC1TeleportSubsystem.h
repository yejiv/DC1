// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DC1TeleportSubsystem.generated.h"

class APlayerState;
class ADC1TeleportActor;

DECLARE_MULTICAST_DELEGATE(FDC1OnPointsChanged);

UCLASS()
class DC1_API UDC1TeleportSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 지점 등록 / 해제 ( 지점 액터의 BeginPlay / EndPlay )
	void					RegisterPoint(AActor* InPoint);
	void					UnregisterPoint(AActor* InPoint);

	// 플레이어가 사용 가능한 지점 목록 ( 클라 UI, 서버 검증 )
	void					GetUsablePoints(const APlayerState* InPlayerState, TArray<AActor*>& OutPoints) const;

	// Teleport Actor 개수 반환
	int32					GetTeleportActorNum() const;

	// MaxBaseNum 반환
	int32					GetMaxBaseNum() const { return MaxBaseNum; }

	// 해금 ID로 텔레포트 오브젝트 찾기
	ADC1TeleportActor*		FindTeleportActorById(FName InPointId) const;

public:
	void							NotifyPointsChanged();

public:
	FDC1OnPointsChanged	OnPointsChanged;

private:
	TArray<TWeakObjectPtr<AActor>>	Points;

private:
	// 거점 생성 가능 개수
	int32							MaxBaseNum = { 1 };
};
