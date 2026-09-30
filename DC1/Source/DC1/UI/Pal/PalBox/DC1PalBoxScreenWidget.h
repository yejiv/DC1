// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "Data/Pal/DC1PalTypes.h"
#include "DC1PalBoxScreenWidget.generated.h"

class UPanelWidget;
class UDC1PalBoxViewModel;
class UDC1PalBoxSlotWidget;
class UDC1PalBoxDetailWidget;
class UDC1PalBoxStorageWidget;
class UDC1PalBoxCursorWidget;

/**
 * 팰박스 화면. 보유 팰 + 상자 + 상세 정보
 */
UCLASS()
class DC1_API UDC1PalBoxScreenWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	virtual void	HandleInputKey(const FKey& InKey) override;

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;
	virtual void	NativeOnActivated() override;
	virtual void	NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 버튼보다 먼저
	virtual FReply	NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply	NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	void			BindViewModel();
	void			UnbindViewModel();
	void			CreateSlots();
	void			RefreshSlots();
	void			RefreshStorage(UDC1PalBoxStorageWidget* InStorage, EDC1PalContainer InContainer);
	void			RefreshDetail();

	// 마우스 아래 칸
	bool			FindSlotAt(const FVector2D& InScreenPosition, EDC1PalContainer& OutContainer, int32& OutSlotIndex) const;

	void			HandlePalsChanged();
	void			HandlePickChanged();
	void			HandleSlotHovered(int32 InSlotIndex);
	void			HandleStorageSlotHovered(int32 InSlotIndex);
	void			HandleBaseSlotHovered(int32 InSlotIndex);

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UPanelWidget>			SlotContainer = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalBoxStorageWidget>	Storage = { nullptr };

	// ③ 거점에 있는 팰. 상자 패널을 그대로 쓴다
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalBoxStorageWidget>	BaseStorage = { nullptr };

	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UDC1PalBoxDetailWidget>	Detail = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "PalBox")
	TSubclassOf<UDC1PalBoxSlotWidget>	SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "PalBox")
	TSubclassOf<UDC1PalBoxCursorWidget>	CursorWidgetClass;

	UPROPERTY(EditAnywhere, Category = "PalBox")
	FKey								CloseKey = EKeys::Zero;

	// 이 값 이상 끌고 떼면 드래그
	UPROPERTY(EditDefaultsOnly, Category = "PalBox", meta = (ClampMin = "0.0"))
	float								DragThreshold = { 10.f };

	// 커서 아이콘 위치 (마우스 기준)
	UPROPERTY(EditDefaultsOnly, Category = "PalBox")
	FVector2D							CursorOffset = { 12.f, 12.f };

private:
	TWeakObjectPtr<UDC1PalBoxViewModel>	PalBoxViewModel = { nullptr };

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDC1PalBoxSlotWidget>>	SlotWidgets;

	UPROPERTY(Transient)
	TObjectPtr<UDC1PalBoxCursorWidget>	CursorWidget = { nullptr };

	EDC1PalContainer					HoveredContainer = { EDC1PalContainer::Party };
	int32								HoveredSlot = { INDEX_NONE };

	// 누른 채 떼기 전
	bool								bPressPending = { false };
	FVector2D							PressPosition = { FVector2D::ZeroVector };
};
