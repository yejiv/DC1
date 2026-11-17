// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "DC1ExperienceSubsystem.generated.h"

/**
 * 경험치 분배. 서버 전용
 * 경험치가 발생한 곳에서 Grant 호출 → 받을 대상을 정해 그 ASC 로 GameplayEvent 를 보낸다
 *  - 플레이어          → 그 PlayerState
 *  - 거점 팰           → 팰 자신
 *  - 파트너 팰         → 주인 PlayerState ( Party 가 분배 )
 *  - 야생 팰 / 그 외   → 무시
 * 누가 반응할지는 SourceTag 로 각 구독자가 거른다
 */
UCLASS()
class DC1_API UDC1ExperienceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	// 경험치 발생 지점에서 호출 ( 서버 )
	void					Grant(AActor* InEarner, const FGameplayTag& InSourceTag, int32 InAmount);

private:
	// 이벤트를 받을 액터. 없으면 nullptr
	AActor*					FindRecipient(AActor* InEarner) const;
};
