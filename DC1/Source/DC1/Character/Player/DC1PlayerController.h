// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "DC1PlayerController.generated.h"

class ADC1Player;

UCLASS()
class DC1_API ADC1PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADC1PlayerController(const FObjectInitializer& ObjectInitializer);

public:
	void SetCursorMode(bool OnTurn);

public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupInputComponent() override;
	// 서버 Possess/UnPossess, 클라이언트 OnRep_Pawn 모두 이 함수를 거친다
	virtual void SetPawn(APawn* InPawn) override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnTurn(const FInputActionValue& Value);
	void OnSprint(const FInputActionValue& Value);
	void RestoreSprint(const FInputActionValue& Value);
	void OnToggleMove(const FInputActionValue& Value);

private:
	void OnUIChange();

#pragma region Debug
	// TODO : 무기 상태 전환 테스트용. 정식 장착 로직 나오면 이 region 통째로 제거
	//        (.cpp 의 동명 region, SetupInputComponent 의 SetUp_DebugInput() 호출 1줄도 같이)
private:
	void SetUp_DebugInput();

	void OnDebugWeaponNone();
	void OnDebugWeaponMelee();
	void OnDebugWeaponBow();

	UFUNCTION(Server, Reliable)
	void ServerSetWeaponState(FGameplayTag NewStateTag);
#pragma endregion

private:
	// InputMappingContext SetUp
	void SetUp_IMC(const FGameplayTag& IMC_Tag);

private:
	TObjectPtr<ADC1Player>	ControlObject = { nullptr };

	FGameplayTag			CurrentIMCTag = FGameplayTag();

private:
	bool	IsTurn = { true };
	bool	IsActiveScreen = { false };

	
#pragma region UI_TEST
private:
	void OpenTech();
private:
	bool	IsOpen = { false };
#pragma endregion
};
