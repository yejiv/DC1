// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Types/SlateEnums.h"
#include "DC1CraftSelectWidget.generated.h"

class UButton;
class UDynamicEntryBox;
class UEditableTextBox;
class UImage;
class UDC1Slider;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_TwoParams(FDC1OnCraftRequested, FName, int32);

UCLASS()
class DC1_API UDC1CraftSelectWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	FDC1OnCraftRequested	OnCraftRequested;

public:
	void			SetRecipe(const FDC1ItemRecipeRow& InRecipe, int32 InMaxCraftCount);
	void			ClearRecipe();

protected:
	virtual void	NativeOnInitialized() override;

private:
	void			SetCraftCount(int32 InCount);
	void			ApplyCountWidgets();
	void			RefreshMaterials();

	UFUNCTION()
	void			HandleSliderChanged(float InValue);

	UFUNCTION()
	void			HandleCountCommitted(const FText& InText, ETextCommit::Type InCommitType);

	UFUNCTION()
	void			HandleMinusTenClicked();

	UFUNCTION()
	void			HandleMinusOneClicked();

	UFUNCTION()
	void			HandlePlusOneClicked();

	UFUNCTION()
	void			HandlePlusTenClicked();

	UFUNCTION()
	void			HandleCraftClicked();

private:
	// 완성품 이미지
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				Image_ProductIcon = { nullptr };
	// 완성품 이름 x 갯수 ex) 야구방망이 x 1
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_ProductName = { nullptr };
	// 사용 재료 목록
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	MaterialListBox = { nullptr };
	// 개수 선택(슬라이드)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1Slider>			Slider_Count = { nullptr };
	// 작업량 표시
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	RequiredWork_Count = { nullptr };
	// 개수 설정 및 표시
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox>	Text_Count = { nullptr };	
	// 현재 생산 개수 표시
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_CurrentCount = { nullptr };
	// 생산 최대 개수
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>			Text_MaxCount = { nullptr };

	// 개수 -10 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				Button_MinusTen = { nullptr };
	// 개수 -1 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				Button_MinusOne = { nullptr };
	// 개수 +1 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				Button_PlusOne = { nullptr };
	// 개수 +10 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				Button_PlusTen = { nullptr };
	// 생산 버튼
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton>				Button_Craft = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				ButtonMinusTen_Image = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				ButtonMinusOne_Image = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				ButtonPlusOne_Image = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>				ButtonPlusTen_Image = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Craft|Count|Color")
	FLinearColor					ActiveCountButtonColor = { FLinearColor::White };

	UPROPERTY(EditAnywhere, Category = "Craft|Count|Color")
	FLinearColor					InactiveCountButtonColor = { FLinearColor::White };

private:
	FDC1ItemRecipeRow				CurrentRecipe;
	int32							CraftCount = { 0 };
	int32							MaxCraftCount = { 0 };
};
