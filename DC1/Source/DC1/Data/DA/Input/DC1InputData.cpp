// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/DA/Input/DC1InputData.h"
#include "InputAction.h"
#include "InputMappingContext.h"

const UInputMappingContext* UDC1InputData::FindInputMappingContextbyTag(const FGameplayTag& Tag) const
{
	for (const FDC1InputMappingContext& IMC : InputMappingContexts)
	{
		if (IMC.InputMappingContext && IMC.IMCTag == Tag)
		{
			return IMC.InputMappingContext;
		}
	}

	UE_LOG(LogWindows, Error, TEXT("Can't find IMC for InputTag [%s]"), *Tag.ToString());

	return nullptr;
}

const UInputAction* UDC1InputData::FindInputActionbyTag(const FGameplayTag& Tag) const
{
	for (const FDC1InputAction& IA : InputActions)
	{
		if (IA.InputAction && IA.IATag == Tag)
		{
			return IA.InputAction;
		}
	}

	UE_LOG(LogWindows, Error, TEXT("Can't find IMC for InputTag [%s]"), *Tag.ToString());

	return nullptr;
}
