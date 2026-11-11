// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "DC1SaveInterface.generated.h"

UINTERFACE(MinimalAPI)
class UDC1SaveInterface : public UInterface
{
	GENERATED_BODY()
};

// 세이브 대상 표시 ( 액터 / 컴포넌트 ). 저장할 멤버는 UPROPERTY(SaveGame)
class DC1_API IDC1SaveInterface
{
	GENERATED_BODY()

public:
	// 저장 직전
	virtual void		OnPreSave() {}

	// 전체 로드 완료 후 처리
	virtual void		OnPostLoad() {}
};
