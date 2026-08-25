// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Enum/DC1Enums.h"
#include "DC1FadeWidget.generated.h"

struct FUILoadingEntry;
class UDC1FadeImage;
class UDC1FadeText;

UCLASS()
class DC1_API UDC1FadeWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	bool			IsFading() const { return bIsFading; }

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void			SetFadeType(EDC1FadeType InType, const FUILoadingEntry* InLoadingEntry = nullptr, const FText* InText = nullptr);
	void			FadeIn(float InDuration, FSimpleDelegate OnComplete = FSimpleDelegate());
	void			FadeOut(float InDuration, FSimpleDelegate OnComplete = FSimpleDelegate());

private:
	void			StartFade(float InTargetAlpha, float InDuration, FSimpleDelegate OnComplete);
	void			FinishFade();

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1FadeImage>	FadeImage = { nullptr };

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UDC1FadeText>	FadeText = { nullptr };

private:
	float			StartAlpha = { 0.f };

	float			TargetAlpha = { 0.f };

	float			FadeDuration = { 0.f };

	float			Elapsed = { 0.f };

	bool			bIsFading = { false };

	FSimpleDelegate	CompleteDelegate;
};
