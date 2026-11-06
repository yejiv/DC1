// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "DC1BuildDestroySubsystem.generated.h"

class ADC1BuildActor;
class APlayerState;

UCLASS()
class DC1_API UDC1BuildDestroySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

private:
	struct FDC1DestroyCascade
	{
		// 현재 파괴 연출 중인 건축물 수
		int32		RemainingCount = { 0 };

		// 파괴 전에 모아둔 인접한 건축물들
		TArray<TWeakObjectPtr<ADC1BuildActor>>	NextCandidates;

		// 이 파괴 연쇄가 맡은 건축물들
		TArray<TWeakObjectPtr<ADC1BuildActor>> Claimed;

		// 이 연쇄를 시작한 플레이어
		TWeakObjectPtr<APlayerState>	Instigator;
	};

public:
	// 파괴 진입점 ( 서버 Only ), InInstigator -> 연쇄 모든 건축물에 파괴자로 보관
	void		RequestDestroy(ADC1BuildActor* InTarget, APlayerState* InInstigator = nullptr);

private:
	void		StartDestroyWave(const TArray<ADC1BuildActor*>& InActors, int32 InCascadeID);
	void		HandleDestroyFinished(ADC1BuildActor* InActor, int32 InCascadeID);
	void		FinishCascade(int32 InCascadeID);

private:
	TMap<int32, FDC1DestroyCascade>			Cascades;
	
	int32									NextCascadeID = { 0 };
	
	// 중복 파괴 방지
	TSet<TWeakObjectPtr<ADC1BuildActor>>	ClaimedActors;
};
