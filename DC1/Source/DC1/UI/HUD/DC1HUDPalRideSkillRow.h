// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalRideSkillRow.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

/**
 * 라이딩 HUD 의 탑승 스킬 한 줄. 키 이미지 + 스킬 이름.
 * 쿨타임 중엔 이름 자리에 차오르는 막대, 키는 흐리게. 쓴 순간 민트 띠가 번쩍이고 빨리 사라진다
 */
UCLASS()
class DC1_API UDC1HUDPalRideSkillRow : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void					SetSkill(const FText& InName, UTexture2D* InKeyIcon);

	void					SetCoolingDown(bool InCoolingDown);

	// 0 = 막 씀, 1 = 다 참
	void					SetCooldownRatio(float InRatio);

	// 쓴 순간 민트 띠
	void					PlayCooldownFlash();

protected:
	virtual void			NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Key = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>	Text_Name = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UProgressBar>	ProgressBar_Cooldown = { nullptr };

	// 줄 전체 민트 띠. 평소엔 투명
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>		Image_Flash = { nullptr };

private:
	// 민트 띠가 사라지는 시간
	UPROPERTY(EditAnywhere, Category = "Ride", meta = (ClampMin = "0.01"))
	float					FlashDuration = { 0.25f };

	// 쿨타임 중 키 이미지 투명도
	UPROPERTY(EditAnywhere, Category = "Ride", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float					CoolingKeyOpacity = { 0.4f };

	float					FlashRemaining = { 0.f };
};
