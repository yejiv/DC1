// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "DC1AttributeSet.generated.h"

/**
 * 피격 가능한 대상 공통 ( HP · 방어력 · 데미지 처리 ) - 채집 오브젝트처럼 캐릭터가 아닌 대상도 이것만 가진다
 * 기존 데미지 GE가 Damage 메타를, 팰 데미지 계산이 대상의 DefensePower를 쓰므로 둘 다 여기 있어야 한다
 * 캐릭터 전용 스탯은 UDC1CharacterAttributeSet
 */
UCLASS()
class DC1_API UDC1AttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Health)

		UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, MaxHealth)

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_DefensePower, Category = "Attributes")
	FGameplayAttributeData DefensePower;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, DefensePower)

	// 메타 어트리뷰트
	UPROPERTY(BlueprintReadOnly, Category = "Attributes|Meta")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS_BASIC(UDC1AttributeSet, Damage)

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldHealth);
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldMaxHealth);
	UFUNCTION()
	void OnRep_DefensePower(const FGameplayAttributeData& OldDefensePower);
};
