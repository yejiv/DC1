// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/Button/DC1UserButton.h"
#include "Define/DC1Enums.h"
#include "DC1CustomizeMenuButton.generated.h"

class UImage;
class UTextBlock;

/**
 * 커스터마이징 화면 좌측 메뉴 버튼(몸/얼굴/머리카락/음성 등).
 * 선택된 메뉴는 다른 메뉴가 선택되기 전까지 Active(=Pressed) 상태를 유지한다 (TabHeaderButton과 같은 EDC1TabButtonState 재사용).
 *
 * Normal/Hover/Pressed 텍스처 3장은 UserButton의 Style(FButtonStyle)에 Blueprint에서 직접 바인딩한다.
 * 네이티브 Pressed는 마우스가 실제로 눌려있는 동안만 보이는 순간 상태라 "선택 유지"가 안 되므로,
 * Active일 때는 Style.Normal/Hovered를 Style.Pressed로 덮어써서 항상 Pressed 텍스처만 보이게 고정한다
 * (TabHeaderButton::ApplyState가 TintColor를 세 슬롯에 동일하게 미는 것과 같은 트릭, 대상만 브러시로 바뀜).
 * SelectedImage는 Pressed(Active)일 때만 위에 추가로 덧씌운다.
 */
UCLASS()
class DC1_API UDC1CustomizeMenuButton : public UDC1UserButton
{
	GENERATED_BODY()

public:
	virtual bool			SetState(EDC1TabButtonState InState);
	EDC1TabButtonState		GetState() const { return ButtonState; }

	// 부모 위젯이 배열/맵에서 이 버튼이 어떤 메뉴인지 식별하는 용도
	EDC1CustomizeMenu		GetMenuType() const { return MenuType; }

protected:
	virtual void	NativeConstruct() override;

	virtual void	HandleClicked() override;
	virtual void	HandleHovered() override;
	virtual void	HandleUnhovered() override;

	// ButtonState 기준으로 UserButton 스타일(Pressed 고정) + SelectedImage 표시를 갱신
	virtual void	ApplyState();

protected:
	EDC1TabButtonState		ButtonState = { EDC1TabButtonState::Default };

private:
	UPROPERTY(EditAnywhere, Category = "UserButton|Customize")
	EDC1CustomizeMenu		MenuType = { EDC1CustomizeMenu::Head };

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock>	TextLabel = { nullptr };

	// WBP 인스턴스별로 직접 채움 (DC1TitleButton의 Button_LabelText와 동일 패턴)
	UPROPERTY(EditAnywhere, Category = "UserButton")
	FText					Button_LabelText = { FText::GetEmpty() };

	// Pressed(Active) 상태일 때만 위에 덧씌우는 이미지
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage>		SelectedImage = { nullptr };

private:
	bool					bIsHovered = { false };

	// Active 진입 시 Normal/Hovered를 Pressed로 덮어쓰기 전에 원래 브러시를 보관 (해제 시 복원용)
	FSlateBrush				CachedNormalBrush;
	FSlateBrush				CachedHoveredBrush;
};
