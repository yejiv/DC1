// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Enum/DC1Enums.h"
#include "UI/Fade/Dc1FadeWidget.h"
#include "DC1UIManagerSubsystem.generated.h"

struct FUILoadingEntry;

class UDC1UIData;
class UDC1UserWidget;
class APlayerController;
class UDC1FadeWidget;

UCLASS()
class DC1_API UDC1UIManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	FSimpleMulticastDelegate	OnScreenStackChanged;
	
	bool						HasActiveScreen() { return 0 < ScreenStack.Num(); };

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// IsPop == 최상위 UI를 Pop하고 Push 할 것인가
	UDC1UserWidget*				PushScreen(const FGameplayTag& UITag, bool IsPop = false);
	void						PopScreen();

	void						FadeIn(float InDuration, EDC1FadeType FadeType = EDC1FadeType::Default, FSimpleDelegate OnComplete = FSimpleDelegate());
	void						FadeOut(float InDuration, FSimpleDelegate OnComplete = FSimpleDelegate());


	void						RouteInputKey(const FKey& Key);
	void						BeginLoadingTransition(float InDuration, FSimpleDelegate OnReadyToTravel);

private:
	UDC1UserWidget*				CreateOverlay(const FGameplayTag& UITag, int32 ZOrder);
	UDC1UserWidget*				PushScreenInternal(UDC1UserWidget* CreatedWidget);
	UDC1UserWidget*				PushScreenInternal(const FGameplayTag& UITag, TSubclassOf<UDC1UserWidget> WidgetClass);


	TSubclassOf<UDC1UserWidget> FindWidgetClass(const FGameplayTag& UITag);
	void						UpdateInputMode();
	
	void						RollLoadingType(EDC1FadeType InType);
	void						OnPostLoadMap(UWorld* LoadedWorld);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UDC1UserWidget>>	ScreenStack;
	
	UPROPERTY()
	TObjectPtr<const UDC1UIData>		CachedUIData;

	UPROPERTY()
	TObjectPtr<UDC1FadeWidget>			FadeWidget;

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1UserWidget>> CreatedWidget;

	EDC1FadeType						PendingFadeType = EDC1FadeType::Default;
	const FUILoadingEntry*				PendingLoadingEntry = nullptr;
	const FText*						PendingTip = nullptr;
	bool								bIsTraveling = false;

	static constexpr int32				FadeZOrder = 100;
};
