// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DC1UserWidget.generated.h"

class UButton;

struct FWidgetAnimationState;

USTRUCT()
struct FDC1AnimPlayInfo
{
	GENERATED_BODY()

	TEnumAsByte<EUMGSequencePlayMode::Type> PlayMode = EUMGSequencePlayMode::Forward;

	bool bRestoreState = false;

	// 0 = 무한 루프
	int32 NumLoops = 1;
};

UCLASS()
class DC1_API UDC1UserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeDestruct() override;

public:
	virtual void HandleInputKey(const FKey& InKey);

public:
	// Manager에서 활성화될 때 호출
	void Activate();

	// Manager에서 비활성화될 때 호출
	void Deactivate();

protected:
	virtual void	NativeOnActivated();
	virtual void	NativeOnDeactivated();

	void			PlayAnim(UWidgetAnimation* InAnim, const FDC1AnimPlayInfo& InPlayInfo);

	// 재생 중단 + 추적 해제
	void			StopAnim(UWidgetAnimation* InAnim);

	void			SnapAnimToEdge(UWidgetAnimation* InAnim, const FDC1AnimPlayInfo& InPlayInfo);

	// Anim 끝난 시점 처리 - ex) ApplyState
	virtual void	OnAnimFinished(const UWidgetAnimation* InAnim) {};

private:
	void			HandleAnimFinished(FWidgetAnimationState& InState);

private:
	// 재생 중인 Anim 진행 방향 Caching
	UPROPERTY()
	TMap<TObjectPtr<UWidgetAnimation>, FDC1AnimPlayInfo> PlayingAnims;

protected:
	// [ IntroAnim ] Animation 바인딩 (없어도 됨)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> IntroAnim;

	// [ OutroAnim ] Animation 바인딩 (없어도 됨)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OutroAnim;
};
