// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "DC1BuildInfoWidget.generated.h"


UCLASS(Abstract)
class DC1_API UDC1BuildInfoWidget : public UDC1UserWidget
{
	GENERATED_BODY()

protected:
	virtual void			NativeDestruct() override;

public:
	void					BindBuildActor(ADC1InteractBuildActor* InBuildActor);
	void					UnbindBuildActor();

	void					SetInfoState(EDC1BuildInfoState InPrevState, EDC1BuildInfoState InNewState);

protected:
	// 자식 구현 Actor Delegate 구독 / 해제 ( 해제는 InBuildActor 유효성 여부 상관없이 Reset )
	virtual void			Subscribe(ADC1InteractBuildActor* InBuildActor) {}
	virtual void			Unsubscribe(ADC1InteractBuildActor* InBuildActor){}

	// 자식 구현 현재 값으로 전부 다시 그리기
	virtual void			RefreshValues() {}

	// 자식 구현 상태별 정적 모습 / 애니메이션
	virtual void			OnInfoStateChanged(EDC1BuildInfoState InPrevState, EDC1BuildInfoState InNewState) {}

private:
	void					UnsubscribeInternal();

protected:
	TWeakObjectPtr<ADC1InteractBuildActor>	BuildActor = { nullptr };
	EDC1BuildInfoState						CurrentInfoState = { EDC1BuildInfoState::Hidden };

private:
	bool									IsSubscribed = { false };
};
