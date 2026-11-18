// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item/Equip/DC1Equip.h"
#include "DC1Glider.generated.h"

UCLASS()
class DC1_API ADC1Glider : public ADC1Equip
{
	GENERATED_BODY()

public:
	ADC1Glider();

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	float	GetGlideSpeed() const { return GlideSpeed; }
	void	SetGlideSpeed(float InSpeed) { GlideSpeed = InSpeed; }

public:
	// 캐릭터 메시에 부착할 소켓
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Glider")
	FName				AttachSocketName;

private:
	// EquipItemRow::Speed. 서버가 장착 시 넣고 클라로 복제
	UPROPERTY(Replicated)
	float	GlideSpeed = { 0.f };
};
