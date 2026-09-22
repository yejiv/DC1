// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/Appearance/DC1AppearanceRegistryData.h"

const FDC1AppearancePartDef* UDC1AppearanceRegistryData::GetHairData(const FGameplayTag& Tag) const
{
	if (false == Tag.IsValid())
	{
		return Hairs.Find(DefaultHairTag);
	}

	return Hairs.Find(Tag);
}

const FDC1AppearancePartDef* UDC1AppearanceRegistryData::GetHeadData(const FGameplayTag& Tag) const
{
	if (false == Tag.IsValid())
	{
		return Heads.Find(DefaultHeadTag);
	}

	return Heads.Find(Tag);
}
