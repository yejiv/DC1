// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/DC1UIData.h"

const TSubclassOf<UDC1UserWidget> UDC1UIData::FindUserWidgetClassByTag(const FGameplayTag& UITag)
{
	if (const TSubclassOf<UDC1UserWidget>* Found = UserWidgetEntires.Find(UITag))
	{
		return *Found;
	}

	UE_LOG(LogWindows, Error, TEXT("Can't find UserWidgetClass for UITag [%s]"), *UITag.ToString());

	return nullptr;
}
