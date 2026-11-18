// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Static.h"
#include "Define/DC1Enums.h"
#include "DC1GameplayCueNotify_Hit.generated.h"

class ADC1ResourceActor;
class UNiagaraSystem;
struct FGameplayEffectModCallbackData;
struct FGameplayEffectSpec;

/**
 * 피격 이펙트 Cue ( BP GCN_Hit, GameplayCueTag = GameplayCue.Hit )
 * 서버 NotifyHit / NotifyShieldHit -> 대상 ASC Cue ( 멀티캐스트 ) -> 각 머신 OnExecute 에서 스폰
 * - Element : 맞은 지점에 속성별 NS ( 맞은 대상이 플레이어면 Player 쪽 - Ring 없고 작은 버전 ) ( 속성은 RawMagnitude 에 실음, 클라는 SkillDef 가 없는 CDO 만 받으므로 서버에서 구함 )
 * - Shield  : 플레이어 실드가 맞으면 플레이어 중심에 실드 NS ( 따라다니게 부착, 파편 방향 User.ShardDir 랜덤 )
 * - Wood / Stone : 리소스 ( 나무 Work.Lumbering / 돌 Work.Mining ) - 플레이어 타격 + 팰 작업 둘 다
 */
UCLASS()
class DC1_API UDC1GameplayCueNotify_Hit : public UGameplayCueNotify_Static
{
	GENERATED_BODY()

public:
	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

	// 서버: 체력이 깎인 피격 ( UDC1AttributeSet ) - 캐릭터는 속성별, 리소스는 나무/돌
	static void NotifyHit(const FGameplayEffectModCallbackData& InData);

	// 서버: 실드가 깎인 피격 ( UDC1PlayerSet )
	static void NotifyShieldHit(const FGameplayEffectModCallbackData& InData);

private:
	static void				NotifyCharacterHit(const FGameplayEffectModCallbackData& InData, const AActor* InTarget);
	static void				NotifyResourceHit(const FGameplayEffectModCallbackData& InData, const ADC1ResourceActor* InResource);

	// 팰 스킬이면 스킬 속성, 아니면 ( 플레이어 무기 등 ) Neutral
	static EDC1PalElement	GetHitElement(const FGameplayEffectSpec& InSpec);

	UNiagaraSystem*			GetElementFX(EDC1PalElement InElement, bool InIsPlayer) const;

private:
	// 표에 없는 속성 ( 무속성 등 ) - NS_Hit01Max
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UNiagaraSystem>							DefaultFX = { nullptr };

	// 속성별 NS ( NS_Hit01Max_Fire ... )
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TMap<EDC1PalElement, TObjectPtr<UNiagaraSystem>>	ElementFX;

	// 맞은 대상이 플레이어일 때 ( NS_Hit01Max_PlayerHit, NS_Hit01Max_Fire_PlayerHit ... )
	UPROPERTY(EditDefaultsOnly, Category = "Hit|Player")
	TObjectPtr<UNiagaraSystem>							PlayerDefaultFX = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Hit|Player")
	TMap<EDC1PalElement, TObjectPtr<UNiagaraSystem>>	PlayerElementFX;

	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	TObjectPtr<UNiagaraSystem>							ShieldFX = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Hit|Resource")
	TObjectPtr<UNiagaraSystem>							WoodFX = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Hit|Resource")
	TObjectPtr<UNiagaraSystem>							StoneFX = { nullptr };
};
