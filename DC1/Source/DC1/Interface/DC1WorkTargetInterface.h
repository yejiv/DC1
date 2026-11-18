// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "DC1WorkTargetInterface.generated.h"

class ADC1Character;
class ADC1PalCharacter;

// 선언만
DECLARE_MULTICAST_DELEGATE(FDC1OnPalWorkersChanged);

UINTERFACE(MinimalAPI)
class UDC1WorkTargetInterface : public UInterface
{
	GENERATED_BODY()
};

/*
 * Work GA 대상 인터페이스( 건축물 = 진행도, 채집 노드 = 데미지 )
AssignPalWorker → Work GA가 IsPalWorkerAssigned 확인 / OnPalWorkersChanged로 해제 감지
 */

class DC1_API IDC1WorkTargetInterface
{
	GENERATED_BODY()

public:
	// 실행 작업 타입 ( Work.* ), 작업할 수 없으면 EmptyTag
	virtual FGameplayTag				GetWorkTypeTag() const = 0;

	// Work GA 시작 / 종료 시점 호출 ( 서버 )
	virtual void						AddWorker(const ADC1Character* InWorker) = 0;
	virtual void						RemoveWorker(const ADC1Character* InWorker) = 0;

	// 팰 배정 헬퍼 함수
	virtual bool						AssignPalWorker(ADC1PalCharacter* InPal) = 0;
	virtual void						UnassignPalWorker(ADC1PalCharacter* InPal) = 0;
	virtual bool						IsPalWorkerAssigned(const ADC1PalCharacter* InPal) const = 0;
	// 지금 배정하면 받아줄지 ( 배정은 하지 않음 ) - 팰 AI 가 쉬는 중에 할 일이 생겼는지 볼 때 쓴다
	virtual bool						CanAcceptPalWorker(const ADC1PalCharacter* InPal) const = 0;

	// 작업 배정 목록 변경 ( 해제 포함 ) - 팰 Work GA가 구독
	virtual FDC1OnPalWorkersChanged&	GetOnPalWorkersChanged() = 0;

	// 서버 - 작업 애니메이션 타격 1회 / InMultiplier = 노티파이 배율
	virtual void						ApplyWorkHit(const ADC1Character* InWorker, float InMultiplier) {}
};

namespace DC1WorkTarget
{
	// 작업자 1명의 작업 속도 = WorkSpeed * isPal ? * 적성 레벨 / State.Dead == 0
	DC1_API float	GetWorkerSpeed(const ADC1Character* InWorker, const FGameplayTag& InWorkType);

	// 팰이 작업에 배정될 수 있는지 - Param( 플레이어 팀 / 적성 / 거점 범위 )
	DC1_API bool	CanPalWorkAt(const ADC1PalCharacter* InPal, const FGameplayTag& InWorkType, const FVector& InLocation);
}
