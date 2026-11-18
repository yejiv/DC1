// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1PalDetailWidget.generated.h"

class UDC1PalBoxViewModel;
class UDC1PalBoxDetailWidget;
class UDC1UserButton;

/**
 * 팰 상세 정보 (보유 팰 목록에서 선택한 팰)
 * 팰 정보 화면 우측에 팝업처럼 띄움, 뒤로가기 버튼으로 닫음
 */
UCLASS()
class DC1_API UDC1PalDetailWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	// 뒤로가기 버튼 클릭 시 호출
	FSimpleDelegate	OnBackClicked;

public:
	// 파티 슬롯 번호의 팰 상세 정보 표시 (빈 칸이면 NoData)
	void SetPalInfo(int32 InSlotIndex);

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;

private:
	void BindViewModel();
	void UnBindViewModel();

	void HandleBackClicked();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalBoxDetailWidget>	PalBoxDetail = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1UserButton>			Button_Back = { nullptr };

private:
	UPROPERTY()
	TWeakObjectPtr<UDC1PalBoxViewModel>	CachedViewModel = { nullptr };
};
