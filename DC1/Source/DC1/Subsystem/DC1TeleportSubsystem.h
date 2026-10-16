// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DC1TeleportSubsystem.generated.h"

class APlayerState;
class ADC1TeleportActor;

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

	// 해금 ID로 텔레포트 오브젝트 찾기
	ADC1TeleportActor*		FindTeleportActorById(FName InPointId) const;

private:
	TArray<TWeakObjectPtr<AActor>>	Points;
};
