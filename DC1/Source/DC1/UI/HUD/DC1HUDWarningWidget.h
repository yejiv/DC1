#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "DC1HUDWarningWidget.generated.h"

class UDynamicEntryBox;
class UDC1HUDWarningRow;
class UDC1PlayerStatusViewModel;
class USoundBase;

UCLASS()
class DC1_API UDC1HUDWarningWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void	SetWarning(EDC1HUDWarningType InType, const FText& InText, const FLinearColor& InColor);
	void	ClearWarning(EDC1HUDWarningType InType);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void	BindViewModel();
	void	UnbindViewModel();

	void	HandleSatietyChanged(float Current, float Max);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	WarningBox = { nullptr };

	UPROPERTY(Transient)
	TMap<EDC1HUDWarningType, TObjectPtr<UDC1HUDWarningRow>>	ActiveRows;

	UPROPERTY(EditAnywhere, Category = "Warning|Hunger")
	FText			HungryText = INVTEXT("배고픔");

	UPROPERTY(EditAnywhere, Category = "Warning|Hunger")
	FText			StarvingText = INVTEXT("굶주림");

	// 주의 표시 색
	UPROPERTY(EditAnywhere, Category = "Warning|Hunger")
	FLinearColor	WarningColor = FLinearColor(1.f, 0.6f, 0.1f, 1.f);
	// 경고 표시 색
	UPROPERTY(EditAnywhere, Category = "Warning|Hunger")
	FLinearColor	DangeColor = FLinearColor(1.f, 0.15f, 0.15f, 1.f);

	// 경고가 새로 뜰 때 재생
	UPROPERTY(EditAnywhere, Category = "Warning|Sound")
	TObjectPtr<USoundBase>	HungerSound = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Warning|Sound")
	TObjectPtr<USoundBase>	StarveSound = { nullptr };

private:
	TWeakObjectPtr<UDC1PlayerStatusViewModel> StatusViewModel = { nullptr };
};
