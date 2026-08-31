// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DC1AnimInstance_Bow.generated.h"

class ADC1Player;
class ADC1ProjectileWeapon;

/**
 * 활 메시 전용. 시위(string1)가 캐릭터의 당기는 손을 따라가게 한다.
 * UDC1AnimInstance는 소유자를 Pawn으로 가정하므로 여기서는 쓰지 않는다.
 */
UCLASS()
class DC1_API UDC1AnimInstance_Bow : public UAnimInstance
{
	GENERATED_BODY()

public:
	UDC1AnimInstance_Bow(const FObjectInitializer& ObjectInitializer);

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

public:
	// 당기는 손 위치. 활 메시 컴포넌트 공간
	UPROPERTY(BlueprintReadOnly)
	FVector			StringLocation = {};

	// 0이면 기본 자세, 1이면 손을 그대로 따라간다
	UPROPERTY(BlueprintReadOnly)
	float				StringAlpha = {};

public:
	// 시위를 당기는 손 소켓. 캐릭터 메시 기준
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow")
	FName			DrawHandSocketName = { "weapon_r" };

	// 캐릭터 애님에서 평가되는 게이트 커브. 이 값이 시위 추종 알파가 된다
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow")
	FName			DrawCurveName = { "StringPull" };

private:
	void				Compute_StringLocation();

private:
	UPROPERTY()
	TObjectPtr<ADC1ProjectileWeapon>	Weapon = { nullptr };
	UPROPERTY()
	TObjectPtr<ADC1Player>					Player = { nullptr };
};