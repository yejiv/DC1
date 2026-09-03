// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1HUDWidget.generated.h"

/**
 * 항상 떠 있는 플레이어 HUD의 루트.
 * ScreenStack에 올라가지 않고 UIManager의 오버레이로 관리된다.
 *
 * 하위 섹션(Status / Pal / Equip / Skill)은 각자 필요한 ViewModel을 직접 조회하므로
 * 이 클래스는 레이아웃 컨테이너 역할만 한다. 특정 섹션에 직접 접근할 일이 생기면
 * 그때 BindWidget 참조를 추가한다.
 */
UCLASS()
class DC1_API UDC1HUDWidget : public UDC1UserWidget
{
	GENERATED_BODY()
};