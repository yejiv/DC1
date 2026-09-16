// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/DA/UI/DC1UIResourceData.h"

#include "Engine/Texture2D.h"

UTexture2D* UDC1UIResourceData::FindTextureByTag(const FGameplayTag& ResourceTag) const
{
	const TObjectPtr<UTexture2D>* Found = TextureEntries.Find(ResourceTag);
	if (nullptr == Found)
	{
		UE_LOG(LogTemp, Warning, TEXT("Can't Find Texture from Resource Tag [%s]."), *ResourceTag.ToString());
		return nullptr;
	}

	return *Found;
}