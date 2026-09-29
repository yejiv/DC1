// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Define/DC1Enums.h"
#include "GameplayTagContainer.h"
#include "DC1CustomizePanel.generated.h"

class UUniformGridPanel;
class UDC1CustomizeIcon;
class UDC1CustomizeViewModel;
class UDC1ColorWheel;
class UDC1CustomizeColorButton;
struct FDC1AppearanceData;

/**
 * ContentSwitcher의 한 페이지(Head 또는 Hair). 레지스트리를 순회해서 IconContainer에
 * DC1CustomizeIcon들을 스폰하고, 클릭된 아이콘을 MenuType에 맞는 ViewModel Set 함수로 이어준다.
 */
UCLASS()
class DC1_API UDC1CustomizePanel : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void			NativeConstruct() override;
	virtual void			NativeDestruct() override;

protected:
	void					BindViewModel();
	void					UnbindViewModel();

	void					PopulateIcons();

	void					HandleIconClicked(UDC1CustomizeIcon* ClickedIcon);

	UFUNCTION()
	void					HandleColorChanged(FLinearColor InColor);

	UFUNCTION()
	void					HandleColorPickClosed(bool bConfirmed);

	void					HandleColorButtonClicked();

private:
	FGameplayTag		MakePartTag(int32 Index);

	// MenuType(Head/Hair/Eye)에 맞는 값을 Data에서 읽거나 VM에 반영 - 색상 관련 함수들이 공유하는 3-way 분기
	FLinearColor		GetColorFromData(const FDC1AppearanceData& Data) const;
	void					ApplyColorToViewModel(const FLinearColor& InColor);

private:
	// 이 패널이 Head 페이지인지 Hair 페이지인지
	UPROPERTY(EditAnywhere, Category = "Customize|Panel")
	EDC1CustomizeMenu									MenuType = { EDC1CustomizeMenu::Head };

	UPROPERTY(EditDefaultsOnly, Category = "Customize|Panel")
	TSubclassOf<UDC1CustomizeIcon>					IconClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel>					IconContainer = { nullptr };

	UPROPERTY()
	TArray<TObjectPtr<UDC1CustomizeIcon>>		Icons;

	TWeakObjectPtr<UDC1CustomizeViewModel>	CustomizeViewModel = { nullptr };

	// 이 패널의 피부색(Head)/머리색(Hair) 선택용 팝업. 평소엔 숨겨져 있다가 ColorButton 클릭 시에만 보임. WBP마다 없을 수 있어서 Optional
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDC1ColorWheel>						ColorWheel = { nullptr };

	// 현재 색을 스와치로 보여주면서 ColorWheel 팝업을 여는 버튼
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UDC1CustomizeColorButton>			ColorButton = { nullptr };

	// ColorWheel 열기 직전 색상 스냅샷 - 취소 시 이 값으로 되돌림
	FLinearColor												ColorBeforeOpen = { FLinearColor::White };

private:
	UPROPERTY()
	TObjectPtr<UDC1CustomizeIcon>					SelectedIcon = { nullptr };

private:
	// Icon 개수
	UPROPERTY(EditAnywhere, Category = "Customize|Panel")
	int32													IconNum = {};

	// Icon 열 개수
	UPROPERTY(EditAnywhere, Category = "Customize|Grid")
	int32													FixedCol = { 6 };

	// Icon 간격
	UPROPERTY(EditAnywhere, Category = "Customize|Grid")
	int32													Interval = { 20 };
};
