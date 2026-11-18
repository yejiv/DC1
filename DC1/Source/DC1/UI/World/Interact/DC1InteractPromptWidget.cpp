// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/World/Interact/DC1InteractPromptWidget.h"


#include "Components/DynamicEntryBox.h"
#include "UI/World/Interact/DC1InteractPromptRowWidget.h"

void UDC1InteractPromptWidget::SetOptions(const TArray<FDC1InteractOption>& InOptions)
{
	if (false == ::IsValid(PromptEntryBox))
	{
		return;
	}

	// 풀 반납 - 다음 CreateEntry에서 재사용 ( 행 상태는 아래 SetOption / SetFillRatio로 전부 덮어씀 )
	PromptEntryBox->Reset();
	RowWidgetByKey.Empty();

	for (const FDC1InteractOption& Option : InOptions)
	{
		UDC1InteractPromptRowWidget* RowWidget = PromptEntryBox->CreateEntry<UDC1InteractPromptRowWidget>();
		if (false == ::IsValid(RowWidget))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1InteractPromptWidget :: Failed Create Row - Check Entry Widget Class"));
			continue;
		}

		RowWidget->SetOption(Option);
		RowWidget->SetFillRatio(0.f);

		RowWidgetByKey.Add(Option.Key, RowWidget);
	}
}

void UDC1InteractPromptWidget::SetKeyHighlighted(EDC1InteractKey InKey, bool IsInHighlighted)
{
	SetHoldRatio(InKey, IsInHighlighted ? 1.f : 0.f);
}

void UDC1InteractPromptWidget::SetHoldRatio(EDC1InteractKey InKey, float InRatio)
{
	UDC1InteractPromptRowWidget* RowWidget = RowWidgetByKey.FindRef(InKey);
	if (false == ::IsValid(RowWidget))
	{
		return;
	}

	RowWidget->SetFillRatio(InRatio);
}
