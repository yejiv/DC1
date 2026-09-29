// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Pal/PalBox/DC1PalBoxDetailWidget.h"

#include "UI/Bar/DC1StatusBar.h"
#include "UI/Pal/PalBox/DC1PalBoxPassiveRowWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxSkillRowWidget.h"
#include "UI/Pal/PalBox/DC1PalBoxWorkEntryWidget.h"

#include "Components/DynamicEntryBox.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

namespace DC1PalBoxDetail
{
	static FText ToText(float InValue)
	{
		const FNumberFormattingOptions NoGrouping = FNumberFormattingOptions::DefaultNoGrouping();
		return FText::AsNumber(FMath::RoundToInt(InValue), &NoGrouping);
	}
}

void UDC1PalBoxDetailWidget::SetDetailInfo(const FDC1PalBoxDetailInfo& InDetailInfo)
{
	const FDC1PalBoxSlotInfo& Summary = InDetailInfo.Summary;

	Panel_NoData->SetVisibility(ESlateVisibility::Collapsed);
	Panel_Content->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	Text_Level->SetText(DC1PalBoxDetail::ToText(static_cast<float>(Summary.Level)));
	Text_Name->SetText(Summary.Name);
	Image_Icon->SetBrushFromTexture(Summary.Icon);

	Image_Gender->SetBrushFromTexture(InDetailInfo.GenderIcon);
	Image_Gender->SetVisibility((nullptr != InDetailInfo.GenderIcon) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	// 속성
	Image_ElementIcon00->SetBrushFromTexture(InDetailInfo.ElementIcon);
	Image_ElementIcon00->SetVisibility((nullptr != InDetailInfo.ElementIcon) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	Image_ElementIcon01->SetBrushFromTexture(InDetailInfo.ElementIcon2);
	Image_ElementIcon01->SetVisibility((nullptr != InDetailInfo.ElementIcon2) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	// 경험치
	const bool bMaxLevel = (InDetailInfo.ExpToNextLevel <= 0);
	ExpBar->SnapToPercent(bMaxLevel ? 1.f : (InDetailInfo.Experience / static_cast<float>(InDetailInfo.ExpToNextLevel)));
	Text_Next->SetText(DC1PalBoxDetail::ToText(bMaxLevel ? 0.f : (InDetailInfo.ExpToNextLevel - InDetailInfo.Experience)));

	// 스탯
	HealthBar->SnapToPercent((Summary.MaxHealth > 0.f) ? (Summary.Health / Summary.MaxHealth) : 0.f);
	Text_Health->SetText(DC1PalBoxDetail::ToText(Summary.Health));
	Text_MaxHealth->SetText(DC1PalBoxDetail::ToText(Summary.MaxHealth));
	Text_Attack->SetText(DC1PalBoxDetail::ToText(InDetailInfo.Attack));
	Text_Defense->SetText(DC1PalBoxDetail::ToText(InDetailInfo.Defense));
	Text_WorkSpeed->SetText(DC1PalBoxDetail::ToText(InDetailInfo.WorkSpeed));

	RefreshWorks(InDetailInfo.Works);
	RefreshSkills(InDetailInfo.Skills);
	RefreshPassives(InDetailInfo.Passives);
}

void UDC1PalBoxDetailWidget::ShowNoData()
{
	Panel_NoData->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	Panel_Content->SetVisibility(ESlateVisibility::Collapsed);
}

void UDC1PalBoxDetailWidget::RefreshWorks(const TArray<FDC1PalBoxWorkInfo>& InWorks)
{
	// 풀에 돌려놓고 다시 꺼낸다
	WorkContainer->Reset();

	for (const FDC1PalBoxWorkInfo& Work : InWorks)
	{
		UDC1PalBoxWorkEntryWidget* Entry = WorkContainer->CreateEntry<UDC1PalBoxWorkEntryWidget>();
		if (nullptr == Entry)
		{
			return;
		}

		Entry->SetWorkInfo(Work);
	}
}

void UDC1PalBoxDetailWidget::RefreshSkills(const TArray<FDC1PalBoxSkillInfo>& InSkills)
{
	SkillContainer->Reset();

	for (int32 Index = 0; Index < SkillRowCount; ++Index)
	{
		UDC1PalBoxSkillRowWidget* Row = SkillContainer->CreateEntry<UDC1PalBoxSkillRowWidget>();
		if (nullptr == Row)
		{
			return;
		}

		if (InSkills.IsValidIndex(Index))
		{
			Row->SetSkillInfo(InSkills[Index]);
		}
		else
		{
			Row->SetEmpty();
		}
	}
}

void UDC1PalBoxDetailWidget::RefreshPassives(const TArray<FDC1PalBoxPassiveInfo>& InPassives)
{
	PassiveContainer->Reset();

	for (const FDC1PalBoxPassiveInfo& Passive : InPassives)
	{
		UDC1PalBoxPassiveRowWidget* Row = PassiveContainer->CreateEntry<UDC1PalBoxPassiveRowWidget>();
		if (nullptr == Row)
		{
			return;
		}

		Row->SetPassiveInfo(Passive);
	}
}
