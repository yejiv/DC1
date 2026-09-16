// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalDisplayTypes.h"

#include "DC1PalStatusWidget.generated.h"

class UTexture2D;
class UTextBlock;
class UImage;
class UDC1StatusBar;

/**
 * 선택된 팰 한 마리의 상태 표시. VM을 모르고, 부모가 가공된 값을 넣어준다.
 * 입력이 표시용 값뿐이라 HUD 밖(팰 박스, 파티 화면)에서도 재사용할 수 있다
 */
UCLASS()
class DC1_API UDC1PalStatusWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;

public:
	void SetPalInfo(const FDC1PalDisplayInfo& InDisplayInfo);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Level = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	HPBar = {};

	// 속성 아이콘 두 칸. 지금 PalData는 속성이 하나뿐이라 01은 아직 채울 값이 없다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_ElementIcon00 = {};

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_ElementIcon01 = {};
};