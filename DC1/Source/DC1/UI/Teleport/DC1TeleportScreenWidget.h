// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/DC1UserWidget.h"
#include "DC1TeleportScreenWidget.generated.h"

class UScrollBox;
class UDC1TeleportViewModel;
class UDC1TeleportRowWidget;

/**
 * 텔레포트 화면. 이동 가능한 지점 목록
 */
UCLASS()
class DC1_API UDC1TeleportScreenWidget : public UDC1UserWidget
{
	GENERATED_BODY()

public:
	virtual void	HandleInputKey(const FKey& InKey) override;

protected:
	virtual void	NativeConstruct() override;
	virtual void	NativeDestruct() override;
	virtual void	NativeOnInitialized() override;

private:
	void			BindViewModel();
	void			UnbindViewModel();

	void			RefreshRows();

	void			HandleEntriesChanged();
	void			HandleRowClicked(int32 InRowIndex);

	void			CreateRowWidget();

private:
	UPROPERTY(Transient, meta = (BindWidget))
	TObjectPtr<UScrollBox>				ScrollBox_Points = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Teleport")
	TSubclassOf<UDC1TeleportRowWidget>	RowWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Teleport")
	FKey								CloseKey = EKeys::Escape;

private:
	TWeakObjectPtr<UDC1TeleportViewModel>	TeleportViewModel = { nullptr };

	UPROPERTY(Transient)
	TArray<TObjectPtr<UDC1TeleportRowWidget>>	RowWidgets;
};
