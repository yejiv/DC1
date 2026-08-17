// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DC1Character.generated.h"

struct FOnAttributeChangeData;

UCLASS()
class DC1_API ADC1Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADC1Character(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

protected:
	// ASC가 준비된 뒤 자식 클래스가 호출 (펠 = 스폰 시, 플레이어 = PlayerState 확보 시)
	void BindHealthWatcher();

	void HandleHealthChanged(const FOnAttributeChangeData& Data);

	// 사망 반응은 자식이 정하기
	virtual void OnDeath() {}

protected:
	bool bDead = { false };

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
