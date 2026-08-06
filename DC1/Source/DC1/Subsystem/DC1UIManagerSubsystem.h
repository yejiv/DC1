// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DC1UIManagerSubsystem.generated.h"

class UDC1UIData;
class UDC1UserWidget;
class APlayerController;

UCLASS()
class DC1_API UDC1UIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// IsPop == 최상위 UI를 Pop하고 Push 할 것인가
	UDC1UserWidget*				PushScreen(const FGameplayTag& UITag, bool IsPop = false);
	void						PopScreen();

private:
	UDC1UserWidget*				PushScreenInternal(TSubclassOf<UDC1UserWidget> WidgetClass);
	TSubclassOf<UDC1UserWidget> FindWidgetClass(const FGameplayTag& UITag);
	void						UpdateInputMode();

private:
	UPROPERTY()
	TArray<TObjectPtr<UDC1UserWidget>> ScreenStack;
	
	UPROPERTY()
	TObjectPtr<const UDC1UIData> CachedUIData;
};
