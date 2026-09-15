// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalDisplayTypes.h"

#include "DC1PalStatusWidget.generated.h"

class UTexture2D;

/**
 * 선택된 팰 한 마리의 상태 표시. VM을 모르고, 부모가 가공된 값을 넣어준다.
 * 입력이 표시용 값뿐이라 HUD 밖(팰 박스, 파티 화면)에서도 재사용할 수 있다
 */
UCLASS()
class DC1_API UDC1PalStatusWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetPalInfo(const FDC1PalDisplayInfo& InDisplayInfo);

	// 빈 슬롯이 선택됐을 때
	void SetEmpty();
};