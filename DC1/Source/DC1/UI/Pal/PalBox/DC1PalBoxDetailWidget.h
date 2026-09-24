// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "DC1PalBoxDetailWidget.generated.h"

class UImage;
class UTextBlock;
class UDynamicEntryBox;
class UDC1StatusBar;

/**
 * 팰 상세 정보
 */
UCLASS()
class DC1_API UDC1PalBoxDetailWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	void			SetDetailInfo(const FDC1PalBoxDetailInfo& InDetailInfo);
	void			ShowNoData();

private:
	void			RefreshWorks(const TArray<FDC1PalBoxWorkInfo>& InWorks);
	void			RefreshSkills(const TArray<FDC1PalBoxSkillInfo>& InSkills);
	void			RefreshPassives(const TArray<FDC1PalBoxPassiveInfo>& InPassives);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_NoData = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UWidget>			Panel_Content = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Level = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Name = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Gender = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	ExpBar = { nullptr };

	// 다음 레벨까지 남은 양
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Next = { nullptr };

	// 속성 아이콘
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_ElementIcon00 = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_ElementIcon01 = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UImage>			Image_Icon = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1StatusBar>	HealthBar = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Health = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_MaxHealth = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Attack = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_Defense = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UTextBlock>		Text_WorkSpeed = { nullptr };

	// Entry Widget Class 는 디자이너에서 지정한다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	WorkContainer = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	SkillContainer = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDynamicEntryBox>	PassiveContainer = { nullptr };

private:
	// 빈 칸은 "스킬 없음"
	UPROPERTY(EditDefaultsOnly, Category = "PalBox", meta = (ClampMin = "1"))
	int32							SkillRowCount = { 3 };
};
