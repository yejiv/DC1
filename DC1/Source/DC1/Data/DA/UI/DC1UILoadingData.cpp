// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/UI/DC1UILoadingData.h"

const FUILoadingEntry* UDC1UILoadingData::GetRandomSprites() const
{
	if (true == LoadingEntries.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1UILoadingData :: Empty UI Loading Data"));
        
		return nullptr;
	}

	int32 RandomNum = FMath::RandRange(0, LoadingEntries.Num() - 1);
    
	return &LoadingEntries[RandomNum];
}
