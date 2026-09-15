// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDPalWidget.generated.h"

class UDC1PalViewModel;
enum class EDC1PalSlotShift : uint8;

/**
 * HUD의 팰 섹션. 선택 슬롯을 가운데 크게, 양옆을 작게 보여주고 전환 시 한 칸씩 민다.
 * ViewModel은 HUD를 거치지 않고 이 위젯이 직접 조회한다.
 */
UCLASS()
class DC1_API UDC1HUDPalWidget : public UDC1UserWidget
{
	GENERATED_BODY()

private:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	void BindViewModel();
	void UnbindViewModel();

private:
	void HandleSlotShifted(EDC1PalSlotShift InShift);

private:
	TWeakObjectPtr<UDC1PalViewModel> PalViewModel = { nullptr };
};