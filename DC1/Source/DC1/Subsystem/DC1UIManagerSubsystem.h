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
	
	bool						HasActiveScreen() { return ScreenStack.Num() > 0; };

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	// InIsPop == 최상위 UI를 Pop하고 Push 할 것인가
	UDC1UserWidget*				PushScreen(const FGameplayTag& InUITag, bool InIsPop = false);
	void						PopScreen();
	UDC1UserWidget*				CreateWidgetWithTag(const FGameplayTag& InUITag);

	void						FadeIn(float InDuration, EDC1FadeType InFadeType = EDC1FadeType::Default, FSimpleDelegate InOnComplete = FSimpleDelegate());
	void						FadeOut(float InDuration, FSimpleDelegate InOnComplete = FSimpleDelegate());


	void						RouteInputKey(const FKey& InKey);
	void						BeginLoadingTransition(float InDuration, FSimpleDelegate InOnReadyToTravel);

	UDC1UserWidget*				FindWidget(const FGameplayTag& InUITag);

private:
	UDC1UserWidget*				CreateOverlay(const FGameplayTag& InUITag, int32 InZOrder);
	UDC1UserWidget*				PushScreenInternal(UDC1UserWidget* InWidget);
	UDC1UserWidget*				PushScreenInternal(const FGameplayTag& InUITag, TSubclassOf<UDC1UserWidget> InWidgetClass);


	TSubclassOf<UDC1UserWidget> FindWidgetClass(const FGameplayTag& InUITag);
	void						UpdateInputMode();
	
	void						RollLoadingType(EDC1FadeType InType);
	void						OnPostLoadMap(UWorld* InLoadedWorld);
	
private:
	UPROPERTY()
	TArray<TObjectPtr<UDC1UserWidget>>	ScreenStack;
	
	UPROPERTY()
	TObjectPtr<const UDC1UIData>		CachedUIData = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1FadeWidget>			FadeWidget = { nullptr };

	UPROPERTY()
	TMap<FGameplayTag, TObjectPtr<UDC1UserWidget>> CreatedWidget;

private:
	EDC1FadeType						PendingFadeType = { EDC1FadeType::Default };
	const FUILoadingEntry*				PendingLoadingEntry = { nullptr };
	const FText*						PendingTip = { nullptr };
	bool								bIsTraveling = { false };

	static constexpr int32				FadeZOrder = { 100 };
};
