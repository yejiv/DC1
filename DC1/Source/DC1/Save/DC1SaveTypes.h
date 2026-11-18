// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "DC1SaveTypes.generated.h"

/**
 * 저장 대상 하나 ( 액터 / 컴포넌트 ) 의 SaveGame 속성 묶음
 */
USTRUCT()
struct FDC1ObjectSaveRecord
{
	GENERATED_BODY()

	// 누구 데이터인가 ( 컴포넌트 이름, 액터 자신은 "Self" )
	UPROPERTY()
	FName			Key = { NAME_None };

	// SaveGame 속성을 바이트로 바꾼 내용물
	UPROPERTY()
	TArray<uint8>	Bytes;
};

/**
 * 월드 액터 하나 ( 다시 스폰할 정보 + 액터 / 컴포넌트 Record )
 */
USTRUCT()
struct FDC1ActorSaveRecord
{
	GENERATED_BODY()

	// 스폰할 클래스
	UPROPERTY()
	FSoftClassPath						ActorClass;

	// 스폰 위치
	UPROPERTY()
	FTransform							Transform = { FTransform::Identity };

	// 액터 자신 ( "Self" ) + 컴포넌트들
	UPROPERTY()
	TArray<FDC1ObjectSaveRecord>	Records;
};
