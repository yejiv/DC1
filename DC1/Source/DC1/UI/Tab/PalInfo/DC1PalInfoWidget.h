// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "GameplayTagContainer.h"
#include "DC1PalInfoWidget.generated.h"

class UImage;
class UWidgetSwitcher;
class UMaterialInstanceDynamic;

class UDC1PalPartyWidget;
class UDC1PalDetailWidget;

class UDC1PreviewSubsystem;

/**
 * 팰 정보 화면
 * 팰 파티 현황(파티 팰 + 미리보기) / 팰 상세 정보 두 패널을 스위칭
 */
UCLASS()
class DC1_API UDC1PalInfoWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;

public:
	void ShowPartyPanel();
	void ShowDetailPanel();

private:
	virtual void	NativeOnActivated() override;
	virtual void	NativeOnDeactivated() override;

private:
	void HandleChangedFocusingPal(const FGameplayTag& SpeciesTag);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher>			WidgetSwitcher_Panel = { nullptr };

	// 팰 파티 현황
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalPartyWidget>		PalPartyWidget = { nullptr };

	// 팰 상세 정보
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalDetailWidget>		PalDetailWidget = { nullptr };

	// PreviewSubsystem의 Pal 렌더 타깃을 표시
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>						Image_PalPreview = { nullptr };

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic>		PalPreviewMID = { nullptr };

	FName												TextureParamName = { TEXT("RenderTarget") };

private:
	TWeakObjectPtr<UDC1PreviewSubsystem>	PreviewSubsystem = { nullptr };
};
