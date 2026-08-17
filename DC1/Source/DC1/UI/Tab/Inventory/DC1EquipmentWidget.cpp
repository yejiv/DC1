// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Tab/Inventory/DC1EquipmentWidget.h"

#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UDC1EquipmentWidget::NativeConstruct()
{
	Super::NativeConstruct();
	CreateUI();
}

void UDC1EquipmentWidget::SetViewModel(UDC1InventoryViewModel* NewViewModel)
{
	CachedViewModel = NewViewModel;
}

void UDC1EquipmentWidget::CreateUI()
{
	// ========== 장비 슬롯 생성 ==========

	// 무기 슬롯 4개
	for (int32 i = 0; i < 4; ++i)
	{
		UBorder* WeaponSlot = NewObject<UBorder>(this);
		WeaponSlot->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
		WeaponSlots.Add(WeaponSlot);
	}

	// 머리
	HeadSlot = NewObject<UBorder>(this);
	HeadSlot->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

	// 몸
	ChestSlot = NewObject<UBorder>(this);
	ChestSlot->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

	// 방패
	ShieldSlot = NewObject<UBorder>(this);
	ShieldSlot->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

	// 글라이더
	GliderSlot = NewObject<UBorder>(this);
	GliderSlot->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));

	// 장신구 2개
	for (int32 i = 0; i < 2; ++i)
	{
		UBorder* AccessorySlot = NewObject<UBorder>(this);
		AccessorySlot->SetBrushColor(FLinearColor(0.3f, 0.3f, 0.3f, 1.0f));
		AccessorySlots.Add(AccessorySlot);
	}

	// ========== 플레이어 정보 생성 ==========

	// 이름
	PlayerNameText = NewObject<UTextBlock>(this);
	PlayerNameText->SetText(FText::FromString(TEXT("플레이어")));
	PlayerNameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));

	// 레벨
	PlayerLevelText = NewObject<UTextBlock>(this);
	PlayerLevelText->SetText(FText::FromString(TEXT("Lv.1")));
	PlayerLevelText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
}
