// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Notification/DC1NotificationComponent.h"

#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Notification/DC1NotificationViewModel.h"

#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

UDC1NotificationComponent::UDC1NotificationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1NotificationComponent::Client_NotifyItemAcquired_Implementation(FName InItemID, int32 InCount)
{
	if (UDC1NotificationViewModel* NotificationViewModel = FindNotificationViewModel())
	{
		NotificationViewModel->PushItemAcquiredNotice(InItemID, InCount);
	}
}

void UDC1NotificationComponent::Client_NotifyPalDefeated_Implementation(FGameplayTag InKillerSpecies, const FString& InKillerNickname, FGameplayTag InVictimSpecies)
{
	if (UDC1NotificationViewModel* NotificationViewModel = FindNotificationViewModel())
	{
		NotificationViewModel->PushPalDefeatNotice(InKillerSpecies, InKillerNickname, InVictimSpecies);

		// 처치음 - 쓰러뜨린 사람 화면에서만
		UGameplayStatics::PlaySound2D(this, DefeatSound);
	}
}

UDC1NotificationViewModel* UDC1NotificationComponent::FindNotificationViewModel() const
{
	const APlayerState* OwnerState = GetOwner<APlayerState>();
	const APlayerController* PC = (nullptr != OwnerState) ? OwnerState->GetPlayerController() : nullptr;
	const ULocalPlayer* LocalPlayer = (nullptr != PC) ? PC->GetLocalPlayer() : nullptr;
	if (nullptr == LocalPlayer)
	{
		return nullptr;
	}

	UDC1ViewModelSubsystem* VMSubsystem = LocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>();
	return (nullptr != VMSubsystem) ? VMSubsystem->GetViewModel<UDC1NotificationViewModel>() : nullptr;
}
