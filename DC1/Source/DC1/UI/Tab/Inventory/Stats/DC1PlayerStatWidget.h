#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1PlayerStatWidget.generated.h"

class UButton;
class UImage;
class UTextBlock;
class UTexture2D;
class UDC1PlayerStatsViewModel;

// 스탯 패널의 한 줄 (아이콘, 이름, 값, ▲, 강화 모드의 [-]/[+])
UCLASS()
class DC1_API UDC1PlayerStatWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetViewModel(UDC1PlayerStatsViewModel* InViewModel);
	void SetEnhanceMode(bool bInEnhanceMode);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void Refresh();
	void HandleStatChanged(EDC1PlayerStat InStat);

	UFUNCTION()
	void HandleIncreaseClicked();

	UFUNCTION()
	void HandleDecreaseClicked();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Name;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Value;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Boosted;

	// 임시 배분 미리보기 (예: +100)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_Pending;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Decrease;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Increase;

	// 패널에 배치된 인스턴스마다 지정
	UPROPERTY(EditAnywhere, Category = "Stat")
	EDC1PlayerStat Stat = { EDC1PlayerStat::Health };

	UPROPERTY(EditAnywhere, Category = "Stat")
	TObjectPtr<UTexture2D> StatIcon = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Stat")
	FText StatName;

	UPROPERTY()
	TObjectPtr<UDC1PlayerStatsViewModel> CachedViewModel = { nullptr };

	bool bEnhanceMode = { false };
};
