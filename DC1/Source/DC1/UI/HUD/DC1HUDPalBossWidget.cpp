// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/HUD/DC1HUDPalBossWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "UI/Bar/DC1StatusBar.h"
#include "ViewModel/Pal/DC1PalBossViewModel.h"

void UDC1HUDPalBossWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindViewModel();
}

void UDC1HUDPalBossWidget::NativeDestruct()
{
	UnbindViewModel();

	Super::NativeDestruct();
}

void UDC1HUDPalBossWidget::BindViewModel()
{
	if (UDC1ViewModelSubsystem* VMSubsystem = GetOwningLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		BossViewModel = VMSubsystem->GetViewModel<UDC1PalBossViewModel>();
		if (false == BossViewModel.IsValid())
			return;

		BossViewModel->OnBossChanged.AddUObject(this, &ThisClass::HandleBossChanged);
		BossViewModel->OnBossStatChanged.AddUObject(this, &ThisClass::HandleBossStatChanged);

		HandleBossChanged();
	}
}

void UDC1HUDPalBossWidget::UnbindViewModel()
{
	if (false == BossViewModel.IsValid())
		return;

	BossViewModel->OnBossChanged.RemoveAll(this);
	BossViewModel->OnBossStatChanged.RemoveAll(this);
	BossViewModel.Reset();
}

void UDC1HUDPalBossWidget::HandleBossChanged()
{
	const bool bHasBoss = BossViewModel.IsValid() && BossViewModel->HasBoss();
	SetVisibility(bHasBoss ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	if (false == bHasBoss)
		return;

	Text_Title->SetText(BossViewModel->GetTitle());
	Text_Name->SetText(BossViewModel->GetName());

	SetElementIcon(Image_Element0, BossViewModel->GetElementIcon(0));
	SetElementIcon(Image_Element1, BossViewModel->GetElementIcon(1));

	HandleBossStatChanged();

	// 새 보스는 지연 감소 연출 없이 바로 맞춘다
	const float MaxHealth = BossViewModel->GetMaxHealth();
	HealthBar->SnapToPercent((MaxHealth > 0.f) ? (BossViewModel->GetHealth() / MaxHealth) : 0.f);
}

void UDC1HUDPalBossWidget::HandleBossStatChanged()
{
	if (false == BossViewModel.IsValid() || false == BossViewModel->HasBoss())
		return;

	const float Health = BossViewModel->GetHealth();
	const float MaxHealth = BossViewModel->GetMaxHealth();

	Text_Level->SetText(FText::AsNumber(BossViewModel->GetLevel()));
	HealthBar->SetPercent((MaxHealth > 0.f) ? (Health / MaxHealth) : 0.f);

	Text_Health->SetText(FText::Format(FText::FromString(TEXT("{0} / {1}")), FText::AsNumber(FMath::CeilToInt(Health)), FText::AsNumber(FMath::CeilToInt(MaxHealth))));
}

void UDC1HUDPalBossWidget::SetElementIcon(UImage* InImage, UTexture2D* InIcon)
{
	if (nullptr == InIcon)
	{
		InImage->SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	InImage->SetBrushFromTexture(InIcon);
	InImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}
