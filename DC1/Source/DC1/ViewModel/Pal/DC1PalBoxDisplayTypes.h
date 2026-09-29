// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DC1PalBoxDisplayTypes.generated.h"

class UTexture2D;

/**
 * 팰박스 위젯 표시값. VM 이 채우고 위젯은 VM 을 모른다
 */
USTRUCT()
struct FDC1PalBoxSlotInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FText					Name;

	UPROPERTY()
	int32					Level = { 1 };

	UPROPERTY()
	float					Health = { 0.f };

	UPROPERTY()
	float					MaxHealth = { 0.f };

	UPROPERTY()
	TObjectPtr<UTexture2D>	Icon = { nullptr };
};

USTRUCT()
struct FDC1PalBoxWorkInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UTexture2D>	Icon = { nullptr };

	// 0 = 적성 없음
	UPROPERTY()
	int32					Level = { 0 };
};

USTRUCT()
struct FDC1PalBoxSkillInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FText					Name;

	UPROPERTY()
	TObjectPtr<UTexture2D>	ElementIcon = { nullptr };

	// 원작 표기
	UPROPERTY()
	int32					Power = { 0 };
};

USTRUCT()
struct FDC1PalBoxPassiveInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FText					Name;

	// Rank 아이콘이 없으면 nullptr
	UPROPERTY()
	TObjectPtr<UTexture2D>	RankIcon = { nullptr };
};

USTRUCT()
struct FDC1PalBoxDetailInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FDC1PalBoxSlotInfo				Summary;

	UPROPERTY()
	TObjectPtr<UTexture2D>			GenderIcon = { nullptr };

	UPROPERTY()
	TObjectPtr<UTexture2D>			ElementIcon = { nullptr };

	// 두 번째 속성이 없으면 nullptr
	UPROPERTY()
	TObjectPtr<UTexture2D>			ElementIcon2 = { nullptr };

	UPROPERTY()
	float							Experience = { 0.f };

	// 최대 레벨이면 0
	UPROPERTY()
	int32							ExpToNextLevel = { 0 };

	// 패시브 포함 최종값
	UPROPERTY()
	float							Attack = { 0.f };

	UPROPERTY()
	float							Defense = { 0.f };

	UPROPERTY()
	float							WorkSpeed = { 0.f };

	// WorkChart 순서
	UPROPERTY()
	TArray<FDC1PalBoxWorkInfo>		Works;

	UPROPERTY()
	TArray<FDC1PalBoxSkillInfo>		Skills;

	UPROPERTY()
	TArray<FDC1PalBoxPassiveInfo>	Passives;
};
