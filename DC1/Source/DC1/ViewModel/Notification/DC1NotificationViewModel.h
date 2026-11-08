// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "ViewModel/Pal/DC1PalBoxDisplayTypes.h"
#include "GameplayTagContainer.h"
#include "DC1NotificationViewModel.generated.h"

struct FDC1PalInstance;
class UTexture2D;

// 포획 성공 알림 표시값
USTRUCT()
struct FDC1PalCaptureNoticeInfo
{
	GENERATED_BODY()

	UPROPERTY()
	FText							Name;

	UPROPERTY()
	int32							Level = { 1 };

	UPROPERTY()
	TObjectPtr<UTexture2D>			Icon = { nullptr };

	// 비어 있으면 위젯이 패시브 칸을 숨긴다
	UPROPERTY()
	TArray<FDC1PalBoxPassiveInfo>	Passives;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPalCaptureNotice, const FDC1PalCaptureNoticeInfo&);

// 왼쪽 한 줄 알림 표시값 (아이템 획득, 야생 쓰러뜨림)
USTRUCT()
struct FDC1FeedNoticeInfo
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UTexture2D>			Icon = { nullptr };

	UPROPERTY()
	FText							Message;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnFeedNotice, const FDC1FeedNoticeInfo&);

/**
 * 화면 알림용 ViewModel. 게임 코드가 알림을 요청하면 표시값으로 바꿔 알림 위젯에 넘긴다
 */
UCLASS()
class DC1_API UDC1NotificationViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	// 구독할 소스가 없다. 요청은 게임 코드가 직접 한다
	virtual void BindSource(ADC1PlayerState* InPlayerState) override {}
	virtual void ClearSource() override {}

public:
	// 포획 성공 (던진 사람 클라)
	void					PushPalCaptureNotice(const FDC1PalInstance& InPal);

	// 아이템 획득 (얻은 사람 클라)
	void					PushItemAcquiredNotice(FName InItemID, int32 InCount);

	// 야생 쓰러뜨림 (쓰러뜨린 사람 클라). 플레이어가 직접 쓰러뜨렸으면 InKillerSpecies 가 비어 있다
	void					PushPalDefeatNotice(const FGameplayTag& InKillerSpecies, const FString& InKillerNickname, const FGameplayTag& InVictimSpecies);

public:
	FOnPalCaptureNotice		OnPalCaptureNotice;
	FOnFeedNotice			OnFeedNotice;
};
