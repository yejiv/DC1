// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1HUDBuildWidget.generated.h"

class ADC1BuildActor;
class UDC1BuildViewModel;
class UDC1HUDBuildGuide;
class UDC1HUDBuildMaterialList;
class UDC1HUDBuildModeBanner;
class UPanelWidget;
class UTextBlock;
class UImage;

UCLASS()
class DC1_API UDC1HUDBuildWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	void HandleBuildModeChanged(EDC1BuildMode InMode, const FGameplayTag& InBuildTag);
	void HandleBlockReasonChanged(EDC1BuildBlockReason InReason);
	void HandleDismantleTargetChanged(ADC1BuildActor* InTarget);

	void SetPlacementInfo(const FGameplayTag& InBuildTag);
	void SetDismantleInfo(const ADC1BuildActor* InTarget);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1HUDBuildModeBanner>	ModeBanner = { nullptr };
	// 건설 불가 박스
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget>			BlockReasonBox = { nullptr };
	// 건설 불가 이유
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_BlockReason = { nullptr };
	// 건축물 이름
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_BuildName = { nullptr };
	// 사용/반환 재료 목록
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1HUDBuildMaterialList>	MaterialList = { nullptr };
	// 가이드
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1HUDBuildGuide>		GuideBox = { nullptr };
	// 센터 망치/파괴 표시
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>					Image_Center = { nullptr };

private:
	// 이유별 텍스트
	UPROPERTY(EditAnywhere, Category = "HUD|Build", meta = (MultiLine = true))
	TMap<EDC1BuildBlockReason, FText>	BlockReasonTexts;

	UPROPERTY(EditAnywhere, Category = "HUD|Build")
	TMap<EDC1BuildMode, TObjectPtr<UTexture2D>>	ModeCenterTextures;

private:
	TWeakObjectPtr<UDC1BuildViewModel>	BuildViewModel = { nullptr };
};
