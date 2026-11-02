// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/Death/DC1DeathScreenWidget.h"

#include "GameFramework/GameStateBase.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "Tags/DC1GamePlayTags.h"

#include "ViewModel/Teleport/DC1TeleportViewModel.h"

#include "Character/Player/DC1Player.h"

#include "UI/Button/AnimButton/DC1BracketButton.h"

#include "Components/TextBlock.h"

void UDC1DeathScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerLocalPlayer = GetOwningPlayerPawn<ADC1Player>();

	BindViewModel();

	if (nullptr != RespawnButton)
	{
		RespawnButton->SetIsEnabled(false);
		RespawnButton->OnClickedDelegate.AddUObject(this, &ThisClass::HandleOnClickedRespawnButton);
	}

	RespawnTextFormatOption.SetMinimumFractionalDigits(1).SetMaximumFractionalDigits(1).SetRoundingMode(ERoundingMode::FromZero);
}

void UDC1DeathScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (true == OwnerLocalPlayer.IsValid())
	{
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		if (nullptr == GameState)
			return;

		const float Remaining = OwnerLocalPlayer->GetRespawnAvailableTime() - GameState->GetServerWorldTimeSeconds();
		if (nullptr != RespawnTimeText)
		{
			RespawnTimeText->SetText(FText::AsNumber(FMath::Max(0.f, Remaining), &RespawnTextFormatOption));
		}

		if (Remaining < 0.f)
		{
			if (nullptr != RespawnButton)
			{
				RespawnButton->SetIsEnabled(true);
			}
		}
	}
}

void UDC1DeathScreenWidget::NativeDestruct()
{
	OwnerLocalPlayer.Reset();

	UnbindViewModel();

	if (nullptr != RespawnButton)
	{
		RespawnButton->OnClickedDelegate.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1DeathScreenWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		TeleportViewModel = VMSubsystem->GetViewModel<UDC1TeleportViewModel>();
	}
}

void UDC1DeathScreenWidget::UnbindViewModel()
{
	// 목록 모드는 텔레포트 화면이 닫힐 때 정리하므로 여기선 참조만 해제
	TeleportViewModel.Reset();
}

void UDC1DeathScreenWidget::HandleOnClickedRespawnButton()
{
	UDC1UIManagerSubsystem* UIManager = GetWorld()->GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (nullptr == UIManager)
		return;

	if (nullptr == TeleportViewModel)
		return;

	TeleportViewModel->SetListMode(EDC1TeleportListMode::Respawn);
	// 부활 포인트 X -> 시작 지점 바로 부활
	if (0 == TeleportViewModel->GetEntries().Num())
	{
		TeleportViewModel->RequestRespawnAtStart();
	}
	else
	{
		UIManager->PushScreen(DC1UITags::UI_Main_Teleport);
	}
}
