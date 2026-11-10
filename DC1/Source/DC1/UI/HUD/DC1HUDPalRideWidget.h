// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalRideWidget.generated.h"

class UDC1HUDPalRideSkillRow;
class UDC1PalRideViewModel;
class UDynamicEntryBox;
class UTextBlock;
class UTexture2D;
class UWidgetAnimation;

/**
 * HUD 의 라이딩 섹션. 탈 수 있는 팰을 소환하면 [F] 탑승, 타면 [F] 내리기 + 탑승 스킬 목록.
 * F 를 누르는 동안 PressAnim, 끝까지 누르면 SuccessAnim
 */
UCLASS()
class DC1_API UDC1HUDPalRideWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleRideInfoChanged();
	void HandleRideHoldChanged(bool InPressed);
	void HandleRideHoldCompleted();
	void HandleRideSkillCooldownChanged(int32 InIndex, bool InStarted);

private:
	TWeakObjectPtr<UDC1PalRideViewModel> RideViewModel = { nullptr };

	// 스킬 순서대로의 줄. 쿨타임 막대를 매 틱 채운다
	UPROPERTY(Transient)
	TArray<TObjectPtr<UDC1HUDPalRideSkillRow>>	SkillRows;

private:
	// "탑승" / "내리기"
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>				Text_Action = { nullptr };

	// 탑승 스킬 목록. 탑승 중에만 보인다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>					Panel_Skill = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>		SkillContainer = { nullptr };

	// F 누르는 동안 (커짐 + 민트 상자). 떼면 거꾸로 돌린다
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>		PressAnim = { nullptr };

	// 끝까지 눌렀을 때 (주황 링)
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation>		SuccessAnim = { nullptr };

private:
	UPROPERTY(EditAnywhere, Category = "Ride")
	FText								RideText;

	UPROPERTY(EditAnywhere, Category = "Ride")
	FText								DismountText;

	// 탑승 스킬 입력 순서대로의 키 이미지. 이 개수만큼만 스킬을 보여 준다
	UPROPERTY(EditAnywhere, Category = "Ride")
	TArray<TObjectPtr<UTexture2D>>		SkillKeyIcons;
};
