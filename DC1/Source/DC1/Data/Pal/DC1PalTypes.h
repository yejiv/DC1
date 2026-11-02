// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "DC1PalTypes.generated.h"

// 팰이 있는 곳
UENUM()
enum class EDC1PalContainer : uint8
{
	Party,
	Box,
	Base,
};

/**
 *
 *
 */
USTRUCT(BlueprintType)
struct FDC1PalInstance
{
	GENERATED_BODY()

	// 어떤 종인가. UDC1PalRegistryData 조회 키
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	FGameplayTag			SpeciesTag;

	// 개체 식별자. 같은 종 여러 마리를 위치와 무관하게 지목한다.
	// 인덱스로 참조하면 한 마리 방출할 때 뒤가 전부 밀린다
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	FGuid					PalID;

	// 비어 있으면 UI가 종 이름을 대신 표시
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	FString					Nickname;

	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	EDC1PalGender			Gender = { EDC1PalGender::None };

	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	int32					Level = { 1 };

	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	float					Experience = { 0.f };

	// 개체값
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	int32					TalentHealth = { -1 };

	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	int32					TalentAttack = { -1 };

	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	int32					TalentDefense = { -1 };

	// 다쳐서 잡힌 팰은 그 상태가 유지
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	float					CurrentHealth = { 0.f };

	// 현재 SAN. -1 = 아직 안 정함(가득)
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	float					CurrentSanity = { -1.f };

	// 개체가 태어날 때 굴려서 붙는다. 같은 종도 개체마다 다르게 만드는 값
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	TArray<FGameplayTag>	PassiveTags;

	// 추첨을 이미 했는가
	UPROPERTY(BlueprintReadOnly, Category = "Pal")
	bool					bPassivesRolled = { false };

	bool IsValidPal() const { return SpeciesTag.IsValid() && PalID.IsValid(); }

	bool IsDeployable() const { return IsValidPal() && (CurrentHealth > 0.f); }
};
