#include "UI/Tab/Inventory/Stats/DC1PlayerStatWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"

#include "ViewModel/Player/DC1PlayerStatsViewModel.h"

namespace
{
	FText FormatStatNumber(float InValue)
	{
		FNumberFormattingOptions Options;
		Options.UseGrouping = false;
		Options.MaximumFractionalDigits = 0;
		return FText::AsNumber(InValue, &Options);
	}
}

void UDC1PlayerStatWidget::SetViewModel(UDC1PlayerStatsViewModel* InViewModel)
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnStatChanged.RemoveAll(this);
	}

	CachedViewModel = InViewModel;

	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnStatChanged.AddUObject(this, &ThisClass::HandleStatChanged);
	}

	Refresh();
}

void UDC1PlayerStatWidget::SetEnhanceMode(bool bInEnhanceMode)
{
	bEnhanceMode = bInEnhanceMode;
	Refresh();
}

void UDC1PlayerStatWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너에서도 인스턴스별 아이콘/이름이 보이도록 여기서 적용
	if (::IsValid(Image_Icon))
	{
		Image_Icon->SetBrushFromTexture(StatIcon);
	}

	if (::IsValid(TextBlock_Name))
	{
		TextBlock_Name->SetText(StatName);
	}
}

void UDC1PlayerStatWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (::IsValid(Button_Increase))
	{
		Button_Increase->OnClicked.AddDynamic(this, &ThisClass::HandleIncreaseClicked);
	}

	if (::IsValid(Button_Decrease))
	{
		Button_Decrease->OnClicked.AddDynamic(this, &ThisClass::HandleDecreaseClicked);
	}
}

void UDC1PlayerStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (::IsValid(CachedViewModel))
	{
		// SetViewModel에서 이미 구독했을 수 있으므로 중복 구독 방지
		CachedViewModel->OnStatChanged.RemoveAll(this);
		CachedViewModel->OnStatChanged.AddUObject(this, &ThisClass::HandleStatChanged);
	}

	Refresh();
}

void UDC1PlayerStatWidget::NativeDestruct()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->OnStatChanged.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UDC1PlayerStatWidget::Refresh()
{
	const bool bHasViewModel = ::IsValid(CachedViewModel);

	if (::IsValid(TextBlock_Value))
	{
		TextBlock_Value->SetText(FormatStatNumber(bHasViewModel ? CachedViewModel->GetStatValue(Stat) : 0.f));
	}

	if (::IsValid(Image_Boosted))
	{
		const bool bBoosted = bHasViewModel && CachedViewModel->IsStatBoosted(Stat);
		Image_Boosted->SetVisibility(bBoosted ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}

	if (::IsValid(TextBlock_Pending))
	{
		const float PendingAmount = bHasViewModel ? CachedViewModel->GetPendingAmount(Stat) : 0.f;
		TextBlock_Pending->SetText(FText::Format(INVTEXT("+{0}"), FormatStatNumber(PendingAmount)));
		TextBlock_Pending->SetVisibility(0.f < PendingAmount ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}

	// 방어력처럼 강화할 수 없는 스탯은 강화 모드에서도 버튼을 숨김, 평소엔 자리도 차지하지 않음
	const bool bShowButtons = bEnhanceMode && bHasViewModel && CachedViewModel->IsEnhanceableStat(Stat);
	const ESlateVisibility ButtonVisibility = bShowButtons ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;

	if (::IsValid(Button_Increase))
	{
		Button_Increase->SetVisibility(ButtonVisibility);
		Button_Increase->SetIsEnabled(bHasViewModel && CachedViewModel->CanAddPending(Stat));
	}

	if (::IsValid(Button_Decrease))
	{
		Button_Decrease->SetVisibility(ButtonVisibility);
		Button_Decrease->SetIsEnabled(bHasViewModel && CachedViewModel->CanRemovePending(Stat));
	}
}

void UDC1PlayerStatWidget::HandleStatChanged(EDC1PlayerStat InStat)
{
	if (Stat == InStat)
	{
		Refresh();
	}
}

void UDC1PlayerStatWidget::HandleIncreaseClicked()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->AddPending(Stat);
	}
}

void UDC1PlayerStatWidget::HandleDecreaseClicked()
{
	if (::IsValid(CachedViewModel))
	{
		CachedViewModel->RemovePending(Stat);
	}
}
