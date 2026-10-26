// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "DC1Character.generated.h"

struct FOnAttributeChangeData;
class IDC1WorkTargetInterface;

UCLASS()
class DC1_API ADC1Character : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ADC1Character(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

public:
	virtual void SetAttackCollision(bool OnCollision) {}

	// Work GA 대상 ( 플레이어 = 상호작용 대상, 펠 = 배정된 대상 ) - 일하지 않는 캐릭터는 nullptr
	virtual IDC1WorkTargetInterface* GetCurrentWorkTarget() const { return nullptr; }

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
