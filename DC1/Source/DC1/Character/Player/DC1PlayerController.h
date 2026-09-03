// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "GameplayTagContainer.h"
#include "DC1PlayerController.generated.h"

class ADC1Player;

class UDC1InputData;

class UDC1TabWidget;
class UDC1InventoryViewModel;

class UDC1BuildPlacementComponent;
class UDC1InteractComponent;

UCLASS()
class DC1_API ADC1PlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ADC1PlayerController(const FObjectInitializer& ObjectInitializer);

public:
	void SetCursorMode(bool OnTurn);
	// Priority 0 초과인 IMC들을 추가/삭제 - Priority 0은 Add / Remove 예외처리 후 return
	void SetIMCEnabled(const FGameplayTag& IMCTag, bool Enabled);

	// State Build 관리, InteractComp Tick 관리
	void SetBuildMode(bool Enabled);

	UDC1InventoryViewModel* GetInventoryViewModel() const { return InventoryViewModel; }
	UDC1BuildPlacementComponent* GetBuildPlacementComponent() const { return BuildPlacementComponent; }
	UDC1InteractComponent* GetInteractComponent() const { return InteractComponent; }
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void SetupInputComponent() override;

	// 서버 Possess/UnPossess, 클라이언트 OnRep_Pawn 모두 이 함수를 거친다
	virtual void SetPawn(APawn* InPawn) override;

	// UI가 열려있는 상태에서 InputKey 전달
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

	virtual void OnRep_PlayerState() override;

private:
	void OnMove(const FInputActionValue& Value);
	void OnTurn(const FInputActionValue& Value);
	void OnSprint(const FInputActionValue& Value);
	void RestoreSprint(const FInputActionValue& Value);
	void OnToggleMove(const FInputActionValue& Value);
	void OnJump(const FInputActionValue& Value);
	void StopJumping(const FInputActionValue& Value);
	void OnCrouch(const FInputActionValue& Value);
	void OnDodge(const FInputActionValue& Value);

	void OnAttack(const FInputActionValue& Value);
	void OnAttackPressed(const FInputActionValue& Value);
	void OnAttackReleased(const FInputActionValue& Value);

	void OnInteractionPressed(const FInputActionValue& Value);
	void OnInteractionReleased(const FInputActionValue& Value);

	void ChangeWeapon(const FInputActionValue& Value);

	void OnThrowPressed(const FInputActionValue& Value);
	void OnThrowReleased(const FInputActionValue& Value);

	// 팰 소환/회수 토글
	void OnCallPalPressed(const FInputActionValue& Value);

private:
	void InitPlayerStateBindings();

// UI 
private:
	void OpenTabWidget(FGameplayTag InUITag, FGameplayTag InTab);
	void OnUIChange();

#pragma region Debug
	// TODO : HP 감소 / HUD 확인용. 테스트 끝나면 이 region 통째로 제거
	//        (.cpp 의 동명 region, SetupInputComponent 의 SetUp_DebugInput() 호출 1줄도 같이)
	void SetUp_DebugInput();

	void OnDebugDamageSelf();

	// 어트리뷰트 변경은 서버 권한이므로 RPC로 넘긴다
	UFUNCTION(Server, Reliable)
	void ServerDebugDamageSelf(float InDamageAmount);
#pragma endregion

	void SetUpDefaultInput(UEnhancedInputComponent* EnhancedInputComponent, const UDC1InputData* InputData);
	void SetUpUIInput(UEnhancedInputComponent* EnhancedInputComponent, const UDC1InputData* InputData);

private:
	// InputMappingContext Set MappingContext Priority 0
	void SetBaseIMC(const FGameplayTag& IMCTag);

private:
	UPROPERTY()
	TObjectPtr<ADC1Player>	ControlObject = { nullptr };

	// 인벤토리는 기존 방식 유지 — PC가 VM을 만들어 위젯에 주입한다
	UPROPERTY()
	TObjectPtr<UDC1TabWidget>			CachedMainTabWidget = { nullptr };

	UPROPERTY()
	TObjectPtr<UDC1InventoryViewModel>	InventoryViewModel = { nullptr };

	FGameplayTag			CurrentIMCTag = FGameplayTag();

	UPROPERTY(EditAnywhere, Category = "Build")
	TObjectPtr<UDC1BuildPlacementComponent> BuildPlacementComponent = { nullptr };

	UPROPERTY(EditAnywhere, Category = "Build|Interact")
	TObjectPtr<UDC1InteractComponent> InteractComponent = { nullptr };

private:
	bool			IsTurn = { true };
	bool			IsActiveScreen = { false };
	FGameplayTag	PressedInteractAbilityTag = { FGameplayTag::EmptyTag };
};
