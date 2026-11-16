// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Slider.h"
#include "DC1Slider.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

/*
* 바를 채운 부분 / 빈 부분 2색으로 그리는 슬라이더
* Bar Brush에 머테리얼 넣고 Percent 갱신
* 코드 상으로 SetValue/SetMin/SetMax 호출 시 -> RefreshFill 직접 호출
*/
UCLASS()
class DC1_API UDC1Slider : public USlider
{
	GENERATED_BODY()

public:
	void			RefreshFill();

protected:
	virtual void	SynchronizeProperties() override;

private:
	void			ApplyBarMaterial();

	UFUNCTION()
	void			HandleFillValueChanged(float InValue);

private:
	// Bar Brush 머테리얼
	UPROPERTY(EditAnywhere, Category = "Fill")
	TObjectPtr<UMaterialInterface>			BarMaterial = { nullptr };

	// BarMaterial - Percent Param Name
	UPROPERTY(EditAnywhere, Category = "Fill")
	FName									PercentParameterName = { TEXT("Percent") };

	// Bar MID
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic>	BarMID = { nullptr };
};
