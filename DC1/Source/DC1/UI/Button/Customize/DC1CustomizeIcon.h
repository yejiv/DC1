// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/Button/DC1UserButton.h"
#include "DC1CustomizeIcon.generated.h"

class UImage;
class UTexture2D;

/**
 * Head/Hair 공용 파츠 아이콘 버튼. Panel이 레지스트리를 순회하며 스폰하고 Init으로 채워준다.
 */
UCLASS()
class DC1_API UDC1CustomizeIcon : public UDC1UserButton
{
	GENERATED_BODY()

public:
	void Init(const FGameplayTag& InPartTag, UTexture2D* InIcon);

	const FGameplayTag& GetPartTag() const { return PartTag; }

	void SetSelected(bool bInSelected);

private:
	UPROPERTY()
	FGameplayTag				PartTag = { };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>		IconImage = { nullptr };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>		SelectedImage = { nullptr };
};
