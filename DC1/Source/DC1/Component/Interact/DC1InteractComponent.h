// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Interface/DC1InteractableInterface.h"
#include "DC1InteractComponent.generated.h"

struct FInputActionValue;

class ADC1Player;
class APlayerController;
class ADC1InteractBuildActor;
class UDC1InteractPromptWidget;
class UDC1UIManagerSubsystem;
class UWidgetComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DC1_API UDC1InteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UDC1InteractComponent();

public:
	AActor*						GetCurrentTarget() const { return CurrentTarget.Get(); }
	void						SetInteractEnabled(bool InEnabled);
	void						SetTargetLocked(bool InLocked);

protected:
	virtual void				BeginPlay() override;

public:
	virtual void				TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void						RequestCancelBuild(ADC1InteractBuildActor* InBuildTarget);
	void						RequestStartWork(ADC1InteractBuildActor* InBuildTarget);
	void						RequestStopWork();
	

private:
	UFUNCTION(Server, Reliable)
	void						Server_CancelBuild(ADC1InteractBuildActor* InBuildTarget);

	UFUNCTION(Server, Reliable)
	void						Server_StartWork(ADC1InteractBuildActor* InBuildTarget);
	
	UFUNCTION(Server, Reliable)
	void						Server_StopWork();
	

	IDC1InteractableInterface*	GetCurrentInteractable() const;
	ADC1Player*					GetPlayer() const;
	UDC1UIManagerSubsystem*		GetUIManager() const;

	AActor*						FindBestTarget() const;
	void						SetCurrentTarget(AActor* InNewTarget);


	bool						IsInInteractRange(const ADC1Player* Player, const AActor* InTarget) const;

	// IMC / 키
	void						SetUpInteractInput();

	void						OnInteractKeyPressed(const FInputActionValue& Value, EDC1InteractKey InKey);
	void						OnInteractKeyReleased(const FInputActionValue& Value, EDC1InteractKey InKey);

	const FDC1InteractOption*	FindOption(EDC1InteractKey InKey) const;

	void						ShowPrompt();
	void						HidePrompt();
	void						RefreshOptions();
	void						ReleaseAllHeldKeys();
	void						TickHoldConfirm(float DeltaTime);

	void						UpdatePromptAndIMC();
	void						SetKeyIMCLayer(EDC1InteractKey InKey, bool InEnabled);
	
	const FGameplayTag			GetIMCTagFromKey(EDC1InteractKey InKey) const;

private:
	// 경계에서 떨림 방지용 Bonus
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float	CurrentTargetDotBonus = { 0.05f };

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float	HoldConfirmDuration = { 2.f };

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	FVector	InteractPromtOffset = { FVector(0.f, 30.f, 0.f) };

private:
	TWeakObjectPtr<AActor>				CurrentTarget = { nullptr };

	bool								IsTargetLocked = { false };

	TWeakObjectPtr<ADC1InteractBuildActor>	ServerWorkTarget = { nullptr };

	TArray<FDC1InteractOption>			CurrentOptions;

	TMap<EDC1InteractKey, FGameplayTag>	HeldAbilityTags;

	TOptional<EDC1InteractKey>			HoldConfirmKey;
	float								HoldConfirmElapsed = { 0.f };

	FDelegateHandle						OptionsChangedHandle;
	TWeakObjectPtr<UDC1InteractPromptWidget> PromptWidget = { nullptr };
	TWeakObjectPtr<UWidgetComponent>	PromptComponent = { nullptr };

	// IMC 중복 호출 방지용 캐시
	TSet<EDC1InteractKey>				EnabledKeyLayers;
};
