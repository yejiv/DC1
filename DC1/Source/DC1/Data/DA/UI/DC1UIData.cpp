// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/UI/DC1UIData.h"
#include "UI/DC1UserWidget.h"

TSubclassOf<UDC1UserWidget> UDC1UIData::FindUserWidgetClassByTag(const FGameplayTag& UITag) const
{
	TSubclassOf<UDC1UserWidget> WidgetClass = UserWidgetEntries.FindRef(UITag);
	if (true == ::IsValid(WidgetClass))
	{
		return WidgetClass;
	}

	UE_LOG(LogTemp, Error, TEXT("UDC1UIData :: Can't find UserWidgetClass for UITag [%s]"), *UITag.ToString());

	return nullptr;
}
