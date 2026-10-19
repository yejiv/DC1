// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Teleport/DC1TeleportViewModel.h"
#include "DC1TeleportRowWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTeleportRowClicked, int32 /*RowIndex*/);

/**
 * 텔레포트 / 부활 지점 한 줄. 클릭만 알리고 처리는 화면이 한다
 */
UCLASS()
class DC1_API UDC1TeleportRowWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetEntry(int32 InRowIndex, const FDC1TeleportEntry& InEntry);

public:
	FOnTeleportRowClicked	OnRowClicked;

protected:
	virtual void	NativeOnInitialized() override;

private:
	UFUNCTION()
	void			HandleClicked();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UButton>			Button_Select = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	// 지점 종류 아이콘 ( 거점 / 텔레포트 )
	UPROPERTY(Transient, meta = (BindWidgetOptional))
	TObjectPtr<UImage>			Image_Type = { nullptr };

private:
	int32						RowIndex = { INDEX_NONE };
};
