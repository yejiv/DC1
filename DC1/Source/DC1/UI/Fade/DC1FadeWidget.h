// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1FadeWidget.generated.h"

class UMaterialInterface;
class UTextBlock;

UENUM(BlueprintType)
enum class EDC1FadeType : uint8
{
	None = 0 UMETA(DisplayName = "None"), // 검정 화면
	Spinner = 1 UMETA(DisplayName = "Spinner"), // 검정 화면 + 스프라이트 애니메이션
	Loading = 2 UMETA(DisplayName = "Loading"), // 검정 화면 + 스프라이트 애니메이션 + Tip
};

UCLASS()
class DC1_API UDC1FadeWidget : public UDC1UserWidget
{
	GENERATED_BODY()
	
public:
	bool			IsFading() const { return bIsFading; }

public:
	void			FadeIn(float InDuration, EDC1FadeType InType, FSimpleDelegate OnComplete = FSimpleDelegate());
	void			FadeOut(float InDuration, FSimpleDelegate OnComplete = FSimpleDelegate());

	void			SetTipText(const FText& InTip);

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeTick(const FGeometry& MyGeomtry, float InDeltaTime) override;

private:
	void			StartFade(float InTargetAlpha, float InDuration, FSimpleDelegate OnComplete);
	void			FinishFade();
	void			ApplyContent(EDC1FadeType InType);

private:
	UPROPERTY(meta = (BindWidget))
	TArray<TObjectPtr<UMaterialInterface>>		SpinnerMaterials;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock>	TipText;

	float			StartAlpha = 0.f;
	float			TargetAlpha = 0.f;
	float			FadeDuration = 0.f;
	float			Elapsed = 0.f;
	bool			bIsFading;

	FSimpleDelegate	CompleteDelegate;
};
