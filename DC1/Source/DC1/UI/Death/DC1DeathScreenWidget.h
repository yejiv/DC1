// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1DeathScreenWidget.generated.h"

class UTextBlock;
class UDC1BracketButton;

class ADC1Player;
class UDC1TeleportViewModel;

/**
 * 사망 화면. 부활 대기 카운트다운 후 부활 버튼
 */
UCLASS()
class DC1_API UDC1DeathScreenWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void	NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

	void HandleOnClickedRespawnButton();

private:
	// 사망 로고
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1UserWidget>	DeathLogoImage = { nullptr };

	// 사망 문구
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		DeathText = { nullptr };

	// 부활 가능까지 남은 시간
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		RespawnTimeText = { nullptr };

	// 부활 ( 대기 시간 끝나면 활성화 )
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1BracketButton>	RespawnButton = { nullptr };

private:
	UPROPERTY()
	TWeakObjectPtr<ADC1Player>	OwnerLocalPlayer = { nullptr };

	// 부활 지점 목록 ( Respawn 모드 설정 / 지점 유무 확인 )
	TWeakObjectPtr<UDC1TeleportViewModel>	TeleportViewModel = { nullptr };

private:
	FNumberFormattingOptions		RespawnTextFormatOption = {};
};
