// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalDetailWidget.generated.h"

class UDC1PalBoxViewModel;

/**
 * 팰 상세 정보 (보유 팰 목록에서 선택한 팰)
 */
UCLASS()
class DC1_API UDC1PalDetailWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;

private:
	void BindViewModel();
	void UnBindViewModel();

private:
	UPROPERTY()
	TWeakObjectPtr<UDC1PalBoxViewModel>	CachedViewModel = { nullptr };
};
