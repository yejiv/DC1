// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Fade/DC1FadeImage.h"
#include "Data/DA/UI/DC1UILoadingData.h"
#include "Components/Image.h"

void UDC1FadeImage::Play(const FUILoadingEntry& InEntry)
{
	SelectFrames = InEntry.Frames;
	FPS = InEntry.FPS;
	LastFrameIndex = INDEX_NONE;
	Elapsed = 0.f;
	bIsPlaying = true;

	if (false == SelectFrames.IsEmpty())
	{
		LastFrameIndex = 0;
		SpriteImage->SetBrushFromTexture(SelectFrames[LastFrameIndex]);
	}
}

void UDC1FadeImage::Stop()
{
	bIsPlaying = false;
}

void UDC1FadeImage::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (false == bIsPlaying || true == SelectFrames.IsEmpty())
	{
		return;
	}

	Elapsed += InDeltaTime;
	const int32 FrameIndex = FMath::FloorToInt(Elapsed * FPS) % SelectFrames.Num();
	if (FrameIndex == LastFrameIndex)
	{
		return;
	}
	else
	{
		LastFrameIndex = FrameIndex;
		SpriteImage->SetBrushFromTexture(SelectFrames[LastFrameIndex]);
	}
}
