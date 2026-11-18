// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "DC1BuildSocketComponent.generated.h"


// 스냅 소켓 데이터 Comp
// 에디터 뷰포트에서 회전 세팅 및 수정하기 위해 SceneComponent 상속
// 위치는 SnapComponent가 갱신
// Local Rotate X = 상대가 붙어오는 방향
// Local Rotate Z = X 둘레 회전
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1BuildSocketComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	UDC1BuildSocketComponent();

public:
	const FVector&					GetPositionFromDirection() const { return PositionFromDirection; }
	const FGameplayTag&				GetSnapType() const { return SnapType; }
	const FGameplayTagContainer&	GetAcceptTypes() const { return AcceptTypes; }

private:
	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	FVector					PositionFromDirection = { FVector::ZeroVector };

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	FGameplayTag			SnapType;

	UPROPERTY(EditDefaultsOnly, Category = "Build|Snap")
	FGameplayTagContainer	AcceptTypes;
};
