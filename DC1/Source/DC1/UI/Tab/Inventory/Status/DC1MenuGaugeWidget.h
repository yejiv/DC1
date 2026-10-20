#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1MenuGaugeWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class UTexture2D;

// 메뉴용 수치 게이지 (실드, HP, 배고픔)
UCLASS()
class DC1_API UDC1MenuGaugeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void SetValues(float InCurrent, float InMax);

protected:
	virtual void NativePreConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar_Gauge;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Current;

	// "/최대" 부분 (현재값보다 흐리게 표시)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Max;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextBlock_Title;

	// 바 아래 1px 선 (채움 색의 반투명)
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> Image_Line;

	// 패널에 배치된 인스턴스마다 지정
	UPROPERTY(EditAnywhere, Category = "Gauge")
	TObjectPtr<UTexture2D> GaugeIcon = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Gauge")
	FText GaugeTitle;

	UPROPERTY(EditAnywhere, Category = "Gauge")
	FLinearColor FillColor = { FLinearColor::White };
};
