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
class ADC1ProductionBuildActor;
class ADC1CraftBuildActor;
class ADC1PickupItem;
class UDC1InteractPromptWidget;
class UDC1UIManagerSubsystem;
class UWidgetComponent;
class USoundBase;

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
	void						RequestCollectProduct(ADC1ProductionBuildActor* InProductionTarget);
	void						RequestPickupItem(ADC1PickupItem* InPickup);
	void						RequestRegisterRecipe(ADC1CraftBuildActor* InCraftTarget, FName InRecipeID, int32 InCraftCount);
	void						RequestCancelRecipe(ADC1CraftBuildActor* InCraftTarget);

	// 서버 검증용 - 작업 GA도 사용 ( 클라가 보낸 대상 거리 확인 )
	bool						IsInInteractRange(const ADC1Player* Player, const AActor* InTarget) const;

private:
	UFUNCTION(Server, Reliable)
	void						Server_CancelBuild(ADC1InteractBuildActor* InBuildTarget);

	UFUNCTION(Server, Reliable)
	void						Server_CollectProduct(ADC1ProductionBuildActor* InProductionTarget);

	UFUNCTION(Server, Reliable)
	void						Server_PickupItem(ADC1PickupItem* InPickup);

	UFUNCTION(Server, Reliable)
	void						Server_RegisterRecipe(ADC1CraftBuildActor* InCraftTarget, FName InRecipeID, int32 InCraftCount);

	UFUNCTION(Server, Reliable)
	void						Server_CancelRecipe(ADC1CraftBuildActor* InCraftTarget);

	void						HandleScreenStackChanged();

	IDC1InteractableInterface*	GetCurrentInteractable() const;
	ADC1Player*					GetPlayer() const;
	UDC1UIManagerSubsystem*		GetUIManager() const;

	AActor*						FindBestTarget() const;
	void						SetCurrentTarget(AActor* InNewTarget);

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

	// 거리 가중 점수 = 0이면 시선 점수만
	UPROPERTY(EditDefaultsOnly, Category = "Interact", meta = (ClampMin = "0"))
	float	DistanceScoreWeight = { 0.5f };

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float	HoldConfirmDuration = { 2.f };


	UPROPERTY(EditDefaultsOnly, Category = "Interact|Widget")
	FVector2D								InteractPromptScreenOffset = { FVector2D::ZeroVector };

	// 옵션 활동 태그( Interact.* ) → Character Ability 태그
	UPROPERTY(EditDefaultsOnly, Category = "Interact|Ability")
	TMap<FGameplayTag, FGameplayTag>		InteractionToAbility;

	// 아이템 줍기 요청 시 본인 화면에서 재생
	UPROPERTY(EditDefaultsOnly, Category = "Interact|Sound")
	TObjectPtr<USoundBase>					PickupSound = { nullptr };

private:
	TWeakObjectPtr<AActor>				CurrentTarget = { nullptr };

	bool								IsTargetLocked = { false };

	TArray<FDC1InteractOption>			CurrentOptions;

	TMap<EDC1InteractKey, FGameplayTag>	HeldAbilityTags;

	TOptional<EDC1InteractKey>			HoldConfirmKey;
	float								HoldConfirmElapsed = { 0.f };

	FDelegateHandle						OptionsChangedHandle;
	TWeakObjectPtr<UDC1InteractPromptWidget> PromptWidget = { nullptr };
	TWeakObjectPtr<UWidgetComponent>	PromptComponent = { nullptr };

	// IMC 중복 호출 방지용 캐시
	TSet<EDC1InteractKey>				EnabledKeyLayers;

	bool								IsRefreshPending = { false };
};
