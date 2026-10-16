// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DC1TeleportPointInterface.generated.h"

class APlayerState;

// 지점 종류 ( UI 표시 / 정렬용, 사용 가능 판정은 CanUse )
UENUM(BlueprintType)
enum class EDC1TeleportPointType : uint8
{
	Teleport = 0		UMETA(DisplayName = "Teleport"),	// 텔레포트 오브젝트 - 해금 필요
	Base = 1			UMETA(DisplayName = "Base"),			// 거점 - 소유자면 바로 사용
};

UINTERFACE(MinimalAPI)
class UDC1TeleportPointInterface : public UInterface
{
	GENERATED_BODY()
};

// 텔레포트 / 부활 지점 ( 텔레포트 오브젝트, 거점 )
class DC1_API IDC1TeleportPointInterface
{
	GENERATED_BODY()

public:
	// 지점 종류
	virtual EDC1TeleportPointType		GetPointType() const = 0;

	// UI 리스트에 보여질 이름
	virtual FText								GetPointName() const = 0;

	// 도착 위치
	virtual FTransform						GetArrivalTransform() const = 0;

	// 해당 플레이어가 사용 가능한 지점인가 ( 해금 여부, 거점 소유자 )
	virtual bool								CanUse(const APlayerState* InPlayerState) const = 0;
};
