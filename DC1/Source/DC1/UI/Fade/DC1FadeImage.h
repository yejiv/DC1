// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "UI/DC1UserWidget.h"
#include "DC1FadeImage.generated.h"

struct	FUILoadingEntry;
class	UImage;

UCLASS()
class DC1_API UDC1FadeImage : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	void	Play(const FUILoadingEntry& InEntry);
	void	Stop();

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage>	SpriteImage;

private:
	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> SelectFrames;

	float	FPS = 10.f;
	float	Elapsed = 0.f;
	int32	LastFrameIndex = INDEX_NONE;
	bool	bIsPlaying = false;
};
