// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DC1GameState.generated.h"

class ADC1PalBoxActor;
class UDC1PalPartyComponent;

UCLASS()
class DC1_API ADC1GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	static ADC1GameState*	Get(const UObject* InWorldContext);

	bool					HasBase() const;
	ADC1PalBoxActor*		GetBasePalBox() const { return BasePalBox; }
	// 위치에 따른 거점 return ( 배치 판정, 건축물들 등록 사용 )
	ADC1PalBoxActor*		FindBaseAt(const FVector& InLocation) const;
	// 거점 세팅 ( 서버당 1개 )
	void					SetBase(ADC1PalBoxActor* InPalBox);
	// 거점 변경( 설치, 해제 ) 시 플레이어들 PalParty에 화면 갱신 전달 ( 서버 직접 / 클라 OnRep )
	void					NotifyBaseChanged();
	// PlayerArray 순회하며 찾기, ( 거점 팰 스폰 및 반환 사용 )
	UDC1PalPartyComponent*	FindPalPartyBySlotName(const FString& InSlotName) const;

private:
	UFUNCTION()
	void					OnRep_BasePalBox();
	// BasePalBox OnEndPlay 호출
	UFUNCTION()
	void					HandleBaseEndPlay(AActor* InActor, EEndPlayReason::Type InEndPlayReason);

private:
	// GS는 현재 거점이 있는지 여부만 확인 ( 배열 + 최대 개수로 변경 시 거점 늘릴 수 있음 )
	// 각 클라 복제
	UPROPERTY(ReplicatedUsing = OnRep_BasePalBox)
	TObjectPtr<ADC1PalBoxActor>	BasePalBox = { nullptr };
};
