// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/BuildInfo/Production/DC1ProductionInfoWidget.h"
#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Animation/WidgetAnimation.h"
#include "Materials/MaterialInstanceDynamic.h"

void UDC1ProductionInfoWidget::OnAnimFinished(const UWidgetAnimation* InAnim)
{
	if (false == ::IsValid(InAnim) || InAnim != ExpandAnim)
	{
		return;
	}

	if (CurrentInfoState != EDC1BuildInfoState::ProductionDetail)
	{
		SetDetailPanelVisible(false);
	}
}

void UDC1ProductionInfoWidget::Subscribe(ADC1InteractBuildActor* InBuildActor)
{
	ADC1ProductionBuildActor* ProductionActor = Cast<ADC1ProductionBuildActor>(InBuildActor);
	if (true == ::IsValid(ProductionActor))
	{
		ProductionChangedHandle = ProductionActor->OnProductionChanged.AddUObject(this, &ThisClass::RefreshValues);
	}
}

void UDC1ProductionInfoWidget::Unsubscribe(ADC1InteractBuildActor* InBuildActor)
{
	ADC1ProductionBuildActor* ProductionActor = Cast<ADC1ProductionBuildActor>(InBuildActor);
	if (true == ::IsValid(ProductionActor))
	{
		ProductionActor->OnProductionChanged.Remove(ProductionChangedHandle);
	}

	ProductionChangedHandle.Reset();
}

void UDC1ProductionInfoWidget::RefreshValues()
{
	ADC1ProductionBuildActor* ProductionActor = Cast<ADC1ProductionBuildActor>(BuildActor.Get());
	if (false == ::IsValid(ProductionActor))
	{
		return;
	}

	FName ItemID = ProductionActor->GetProductionID();
	if (ItemID != CachedItemID)
	{
		SetItemInfo(ItemID);
		CachedItemID = ItemID;
	}

	if (true == ::IsValid(ItemProgressImage))
	{
		UMaterialInstanceDynamic* MID = ItemProgressImage->GetDynamicMaterial();
		if (true == ::IsValid(MID))
		{
			MID->SetScalarParameterValue(ItemProgressParamName, ProductionActor->GetProductionProgress());
		}
	}

	if (true == ::IsValid(StoredCountText))
	{
		FText StoredCount = FText::FromString(FString::FromInt(ProductionActor->GetStoredCount()));
		StoredCountText->SetText(StoredCount);
	}

	if (true == ::IsValid(MaxCountText))
	{
		FText MaxCount = FText::FromString(FString::FromInt(ProductionActor->GetMaxStoredCount()));
		MaxCountText->SetText(MaxCount);
	}
}

void UDC1ProductionInfoWidget::OnInfoStateChanged(EDC1BuildInfoState InPrevState, EDC1BuildInfoState InNewState)
{
	if (CurrentInfoState == EDC1BuildInfoState::Hidden)
	{
		return;
	}

	SetActivePanel(InNewState);

	FDC1AnimPlayInfo Forward;
	FDC1AnimPlayInfo Reverse;
	Reverse.PlayMode = EUMGSequencePlayMode::Reverse;

	switch (InNewState)
	{
	case EDC1BuildInfoState::ProductionDetail:
	{
		(InPrevState == EDC1BuildInfoState::ProductionCompact) ? PlayAnim(ExpandAnim, Forward) : SnapAnimToEdge(ExpandAnim, Forward);
		SetDetailPanelVisible(true);
		break;
	}
	case EDC1BuildInfoState::ProductionCompact:
	{
		if (InPrevState == EDC1BuildInfoState::ProductionDetail)
		{
			PlayAnim(ExpandAnim, Reverse);
		}
		else
		{
			SnapAnimToEdge(ExpandAnim, Reverse);
			SetDetailPanelVisible(false);
		}
		break;
	}
	case EDC1BuildInfoState::ProductionNoItem :
	{
		SnapAnimToEdge(ExpandAnim, Reverse);
		SetDetailPanelVisible(false);
	}
	}
}

void UDC1ProductionInfoWidget::SetActivePanel(EDC1BuildInfoState InState)
{
	if (false == ::IsValid(ProductionStateSwitcher))
	{
		return;
	}

	int32* Index = StateToSwitchIndex.Find(InState);
	if (nullptr == Index || nullptr == ProductionStateSwitcher->GetWidgetAtIndex(*Index))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1ProductionInfoWidget :: Don't Find PanelToSwitchIndex - [%s]"), *UEnum::GetValueAsString(InState));
		return;
	}

	ProductionStateSwitcher->SetActiveWidgetIndex(*Index);
}

void UDC1ProductionInfoWidget::SetItemInfo(FName InItemID)
{
	if (false == ::IsValid(ItemIcon) || false == ::IsValid(ItemNameText))
	{
		return;
	}

	if (true == InItemID.IsNone())
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		ItemNameText->SetText(FText::GetEmpty());

		return;
	}

	const FDC1ItemRow* Row = DC1ItemHelper::FindItemRow(InItemID);

	if (nullptr == Row)
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1ProductionInfoWidget :: Can't Find ItemRow - [%s]"), *InItemID.ToString());

		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		ItemNameText->SetText(FText::GetEmpty());
		return;
	}

	if (true == Row->ItemIcon.IsNull())
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		ItemIcon->SetBrushFromSoftTexture(Row->ItemIcon);
		ItemIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	ItemNameText->SetText(Row->ItemName);
}

void UDC1ProductionInfoWidget::SetDetailPanelVisible(bool InIsVisible)
{
	if (true == ::IsValid(DetailPanel))
	{
		DetailPanel->SetVisibility(InIsVisible ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}
