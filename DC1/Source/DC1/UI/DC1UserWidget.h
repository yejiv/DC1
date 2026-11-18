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

	TEnumAsByte<EUMGSequencePlayMode::Type> PlayMode = { EUMGSequencePlayMode::Forward };

	bool RestoreState = { false };

	// 0 = 무한 루프
	int32 NumLoops = { 1 };
};

UCLASS()
class DC1_API UDC1UserWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void	NativeOnInitialized() override;
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;

public:
	virtual void	HandleInputKey(const FKey& InKey);

public:
	// Manager에서 활성화될 때 호출
	virtual void	Activate();

	// Manager에서 비활성화될 때 호출
	virtual void	Deactivate();

protected:
	// 기본 구현이 자식에게 전파 - 오버라이드는 Super 호출 필수
	virtual void	NativeOnActivated();
	virtual void	NativeOnDeactivated();

	// 자식에게 Activate/Deactivate를 전파할지 - 기본은 전부 ( TabWidget의 탭 변환에서의 비활성화 목적 )
	virtual bool	ShouldPropagateTo(const UDC1UserWidget* InChild) const { return true; }

	void			PopScreen();

	void			PlayAnim(UWidgetAnimation* InAnim, const FDC1AnimPlayInfo& InPlayInfo);

	// 재생 중단 + 추적 해제
	void			StopAnim(UWidgetAnimation* InAnim);

	void			SnapAnimToEdge(UWidgetAnimation* InAnim, const FDC1AnimPlayInfo& InPlayInfo);

	// Anim 끝난 시점 처리 - ex) ApplyState
	virtual void	OnAnimFinished(const UWidgetAnimation* InAnim) {};

private:
	void			HandleAnimFinished(FWidgetAnimationState& InState);

	// NativeOnActivated/Deactivated override시 Super 미호출 -> 해당 자식 단계에서 전파 끊김
	void			PropagateActivation(bool InIsActivate);

protected:
	// 상태가 바뀔 때만 NativeOnActivated/Deactivated 호출
	bool			bIsActivated = { false };

private:
	// 재생 중인 Anim 진행 방향 Caching
	UPROPERTY()
	TMap<TObjectPtr<UWidgetAnimation>, FDC1AnimPlayInfo> PlayingAnims;

protected:
	// [ IntroAnim ] Animation 바인딩 (없어도 됨)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> IntroAnim = { nullptr };

	// [ OutroAnim ] Animation 바인딩 (없어도 됨)
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> OutroAnim = { nullptr };

private:
	UPROPERTY()
	TMap<TObjectPtr<UWidgetAnimation>, FDC1AnimPlayInfo> PendingSnaps;
};
