// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Tab/Build/DC1BuildTabWidget.h"

#include "UI/Button/AnimButton/Tab/DC1TabHeaderButton.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1TechTreeComponent.h"

void UDC1BuildTabWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	

	UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		return;
	}

	TechComp->OnTechStateChange.AddUObject(this, &ThisClass::OnTechStateChanged);
}

void UDC1BuildTabWidget::ButtonInitialize()
{

}

void UDC1BuildTabWidget::CloseWidget(const FKey& InputKey)
{
}

UDC1TechTreeComponent* UDC1BuildTabWidget::GetTechTreeComponent()
{
	ADC1PlayerState* PS = GetOwningPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PS))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Can't Find PS"));
		return nullptr;
	}

	UDC1TechTreeComponent* TechComp = PS->GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		UE_LOG(LogTemp, Warning, TEXT("UDC1BuildTabWidget :: Can't Find TechComp"));
		return nullptr;
	}

	return TechComp;
}

void UDC1BuildTabWidget::OnTechStateChanged()
{
	UDC1TechTreeComponent* TechComp = GetTechTreeComponent();
	if (false == ::IsValid(TechComp))
	{
		return;
	}

	for (const TPair<FGameplayTag, int32>& Pair : TabToSwitchIndex)
	{
		//UDC1BuildCategoryWidget* CategoryWidget = GetTabWidget<UDC1BuildCategoryWidget>(Pair.Key);
		//if (false == ::IsValid(CategoryWidget))
		//{
		//	continue;
		//}

		//CategoryWidget->RefreshUnlockState(TechComp);

		//if (CategoryWidget->HasUnlockedSlot())
		//{
		//	UDC1TabHeaderButton* TabButton = nullptr;
		//	TObjectPtr<UDC1TabHeaderButton>* TabButtonPtr = TabButtons.Find(Pair.Key);
		//	if (nullptr != TabButtonPtr)
		//	{
		//		TabButton = TabButtonPtr->Get();
		//	}

		//	if (false == TabButton->CanSelect())
		//	{
		//		TabButton->SetState(EDC1TabButtonState::Inactive);
		//	}
		//}
	}
}
