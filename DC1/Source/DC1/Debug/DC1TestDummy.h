// Fill out your copyright notice in the Description page of Project Settings.
// TODO : 공격 판정 테스트용 더미. 정식 타겟 나오면 이 파일 통째로 삭제

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystemInterface.h"
#include "DC1TestDummy.generated.h"

class UCapsuleComponent;
class UStaticMeshComponent;
class UDC1AbilitySystemComponent;
class UDC1PalAttributeSet;

UCLASS()
class DC1_API ADC1TestDummy : public AActor, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADC1TestDummy(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

private:
	virtual void PostInitializeComponents() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "Dummy")
	TObjectPtr<UCapsuleComponent>			Capsule = { nullptr };
	UPROPERTY(VisibleAnywhere, Category = "Dummy")
	TObjectPtr<UStaticMeshComponent>		Mesh = { nullptr };

	UPROPERTY(VisibleAnywhere, Category = "Ability")
	TObjectPtr<UDC1AbilitySystemComponent>	AbilitySystemComponent = { nullptr };
	UPROPERTY()
	TObjectPtr<UDC1PalAttributeSet>			AttributeSet = { nullptr };
};