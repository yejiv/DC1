// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/UI/DC1UIData.h"

TSubclassOf<UDC1UserWidget> UDC1UIData::FindUserWidgetClassByTag(const FGameplayTag& UITag) const
{
	if (const TSubclassOf<UDC1UserWidget>* Found = UserWidgetEntries.Find(UITag))
	{
		return *Found;
	}

	UE_LOG(LogTemp, Error, TEXT("Can't find UserWidgetClass for UITag [%s]"), *UITag.ToString());

	return nullptr;
}
