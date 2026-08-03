// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DC1UserWidget.generated.h"

UCLASS()
class DC1_API UDC1UserWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// Manager에서 활성화될 때 호출
	void Activate();

	// Manager에서 비활성화될 때 호출
	void Deactivate();

protected:
	void NativeOnActivated();
	void NativeOnDeactivated();

protected:
	// [ IntroAnim ] Animation 바인딩 (없어도 됨)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> IntroAnim;

	// [ OutroAnim ] Animation 바인딩 (없어도 됨)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OutroAnim;
};
