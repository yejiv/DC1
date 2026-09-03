// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1PlayerStatusViewModel.generated.h"

class UAbilitySystemComponent;

struct FGameplayAttribute;
struct FOnAttributeChangeData;

// 현재값과 최대값은 항상 같이 움직이므로 하나의 델리게이트로 묶는다
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float /*Current*/, float /*Max*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnStaminaChanged, float /*Current*/, float /*Max*/);

/**
 * 플레이어 HUD용 ViewModel.
 * PlayerState의 ASC 어트리뷰트 변경을 구독해 표시용 값으로 가공한 뒤 위젯에 전달한다.
 */
UCLASS()
class DC1_API UDC1PlayerStatusViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	FOnHealthChanged	OnHealthChanged;
	FOnStaminaChanged	OnStaminaChanged;

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	// 위젯이 BindSource보다 늦게 붙는 경우가 있으므로, 구독과 별개로 현재 값을 읽어갈 수 있어야 한다
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
	float GetHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
	float GetMaxHealth() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
	float GetStamina() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
	float GetMaxStamina() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Status")
	float GetStaminaPercent() const;

private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);
	void HandleStaminaChanged(const FOnAttributeChangeData& Data);
	void HandleMaxStaminaChanged(const FOnAttributeChangeData& Data);

	// 어트리뷰트 델리게이트는 '값이 변할 때만' 오므로, 바인딩 직후 현재 값을 한 번 직접 밀어준다
	void BroadcastAll();

	float GetAttributeValue(const FGameplayAttribute& InAttribute) const;

private:
	TWeakObjectPtr<UAbilitySystemComponent>	AbilitySystemComponent = { nullptr };
};