// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalPartyWidget.generated.h"

class UImage;

/**
 * 팰 파티 현황 (파티에 등록한 팰 목록 + 선택한 팰 미리보기)
 */
UCLASS()
class DC1_API UDC1PalPartyWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	// PreviewSubsystem의 Pal 렌더 타깃을 표시
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>	Image_PalPreview = { nullptr };
};
