// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "DC1NotificationComponent.generated.h"

class UDC1NotificationViewModel;
class USoundBase;

/**
 * 서버에서 정해진 일(아이템 획득, 야생 쓰러뜨림)을 그 플레이어 화면의 알림으로 보낸다. ADC1PlayerState 에 붙는다.
 * 서버가 Client_ 함수를 부르면 주인 클라에서 알림 ViewModel 로 넘긴다.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DC1_API UDC1NotificationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDC1NotificationComponent();

public:
	// 아이템 획득
	UFUNCTION(Client, Reliable)
	void							Client_NotifyItemAcquired(FName InItemID, int32 InCount);

	// 야생 쓰러뜨림. 플레이어가 직접 쓰러뜨렸으면 InKillerSpecies 가 비어 있다
	UFUNCTION(Client, Reliable)
	void							Client_NotifyPalDefeated(FGameplayTag InKillerSpecies, const FString& InKillerNickname, FGameplayTag InVictimSpecies);

private:
	// 주인 클라의 알림 ViewModel. 로컬 플레이어가 아니면 nullptr
	UDC1NotificationViewModel*		FindNotificationViewModel() const;

private:
	// 야생 쓰러뜨림 알림과 함께 재생
	UPROPERTY(EditDefaultsOnly, Category = "Notification|Sound")
	TObjectPtr<USoundBase>			DefeatSound = { nullptr };
};
