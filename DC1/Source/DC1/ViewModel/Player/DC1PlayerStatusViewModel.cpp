// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewModel/Player/DC1PlayerStatusViewModel.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Character/Player/DC1PlayerState.h"

void UDC1PlayerStatusViewModel::BindSource(ADC1PlayerState* InPlayerState)
{
	// TODO : 기존 구독이 있으면 ClearSource로 먼저 해제
	// TODO : InPlayerState에서 ASC를 꺼내 AbilitySystemComponent에 캐싱
	// TODO : Health / MaxHealth / Stamina / MaxStamina 변경 델리게이트 구독
	// TODO : BroadcastAll로 현재 값 초기 전달
}

void UDC1PlayerStatusViewModel::ClearSource()
{
	// TODO : 구독한 어트리뷰트 델리게이트들을 RemoveAll(this)로 해제
	// TODO : AbilitySystemComponent 무효화
}

float UDC1PlayerStatusViewModel::GetHealth() const
{
	return 0.f;
}

float UDC1PlayerStatusViewModel::GetMaxHealth() const
{
	return 0.f;
}

float UDC1PlayerStatusViewModel::GetHealthPercent() const
{
	// TODO : MaxHealth가 0 이하일 때 0 반환 (나눗셈 방어는 위젯이 아니라 여기서)
	return 0.f;
}

float UDC1PlayerStatusViewModel::GetStamina() const
{
	return 0.f;
}

float UDC1PlayerStatusViewModel::GetMaxStamina() const
{
	return 0.f;
}

float UDC1PlayerStatusViewModel::GetStaminaPercent() const
{
	return 0.f;
}

void UDC1PlayerStatusViewModel::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	// TODO : OnHealthChanged 브로드캐스트
}

void UDC1PlayerStatusViewModel::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	// TODO : OnHealthChanged 브로드캐스트
}

void UDC1PlayerStatusViewModel::HandleStaminaChanged(const FOnAttributeChangeData& Data)
{
	// TODO : OnStaminaChanged 브로드캐스트
}

void UDC1PlayerStatusViewModel::HandleMaxStaminaChanged(const FOnAttributeChangeData& Data)
{
	// TODO : OnStaminaChanged 브로드캐스트
}

void UDC1PlayerStatusViewModel::BroadcastAll()
{
	// TODO : OnHealthChanged / OnStaminaChanged 현재 값으로 브로드캐스트
}

float UDC1PlayerStatusViewModel::GetAttributeValue(const FGameplayAttribute& InAttribute) const
{
	// TODO : ASC 유효성 확인 후 GetNumericAttribute 반환, 아니면 0
	return 0.f;
}