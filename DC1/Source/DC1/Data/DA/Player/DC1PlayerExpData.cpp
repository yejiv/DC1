// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/DA/Player/DC1PlayerExpData.h"

namespace DC1PlayerExp
{
	static const FDC1PlayerExpRow* FindRow(const UDataTable* InTable, int32 InLevel)
	{
		if (nullptr == InTable)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Player] ExpTable is not set. Check DA_PlayerExp."));
			return nullptr;
		}

		return InTable->FindRow<FDC1PlayerExpRow>(FName(*FString::FromInt(InLevel + 1)), TEXT("PlayerExp"), false);
	}
}

int32 UDC1PlayerExpData::GetExpToNextLevel(int32 InLevel) const
{
	if (InLevel >= MaxLevel)
	{
		return 0;
	}

	const FDC1PlayerExpRow* Row = DC1PlayerExp::FindRow(ExpTable, InLevel);

	return (nullptr != Row) ? Row->PlayerNextExp : 0;
}
