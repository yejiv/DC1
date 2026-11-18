// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ViewModel/DC1ViewModelBase.h"
#include "DC1PalBossViewModel.generated.h"

class ADC1PlayerState;
class ADC1PalCharacter;
class UTexture2D;
struct FOnAttributeChangeData;

DECLARE_MULTICAST_DELEGATE(FOnPalBossChanged);
DECLARE_MULTICAST_DELEGATE(FOnPalBossStatChanged);

/**
 * 보스 HP바 용 ViewModel.
 */
UCLASS()
class DC1_API UDC1PalBossViewModel : public UDC1ViewModelBase
{
	GENERATED_BODY()

public:
	virtual void BindSource(ADC1PlayerState* InPlayerState) override;
	virtual void ClearSource() override;

public:
	bool					HasBoss() const { return Boss.IsValid(); }
	const FText&			GetTitle() const { return Title; }
	const FText&			GetName() const { return Name; }
	UTexture2D*				GetElementIcon(int32 InIndex) const;

	int32					GetLevel() const;
	float					GetHealth() const;
	float					GetMaxHealth() const;

public:
	// 보스가 바뀌거나 사라졌다
	FOnPalBossChanged		OnBossChanged;

	// 지금 보스의 HP, 레벨이 바뀌었다
	FOnPalBossStatChanged	OnBossStatChanged;

private:
	void					CheckBoss();
	void					SetBoss(ADC1PalCharacter* InBoss);
	void					HandleStatChanged(const FOnAttributeChangeData& Data);

private:
	TWeakObjectPtr<ADC1PlayerState>		PlayerState = { nullptr };
	TWeakObjectPtr<ADC1PalCharacter>	Boss = { nullptr };

	FTimerHandle						BossCheckTimer;

	FText								Title;
	FText								Name;

	UPROPERTY()
	TObjectPtr<UTexture2D>				ElementIcon0 = { nullptr };

	UPROPERTY()
	TObjectPtr<UTexture2D>				ElementIcon1 = { nullptr };
};
