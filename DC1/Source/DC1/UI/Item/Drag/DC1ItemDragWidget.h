// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1ItemDragWidget.generated.h"

class UImage;
class UTextBlock;
class USizeBox;
struct FDC1ItemRow;

/**
 * 드래그 중 마우스를 따라다니는 아이템 미리보기 위젯
 * DragDropOperation의 DefaultDragVisual로 사용됨
 */
UCLASS()
class DC1_API UDC1ItemDragWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	UDC1ItemDragWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

public:
	/**
	 * 드래그 위젯 초기화
	 * @param InItemData 아이템 정보
	 * @param InStackCount 드래그 중인 아이템 개수
	 */
	void InitDragWidget(const FDC1ItemRow* InItemData, int32 InStackCount);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeBox_Root;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_Icon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_Count;
};
