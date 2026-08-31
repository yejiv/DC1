// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "GameplayTagContainer.h"
#include "DC1Player.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UPrimitiveComponent;
class USphereComponent;

class UDC1AbilitySystemComponent;
class UDC1EquipComponent;
/**
 * 
 */
UCLASS()
class DC1_API ADC1Player : public ADC1Character
{
	GENERATED_BODY()

public:
	ADC1Player(const FObjectInitializer& ObjectInitializer);

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void SetAttackCollision(bool OnCollision) override;

	USphereComponent* GetInteractDetectCollider() const { return InteractDetectCollider; }

public:
	UFUNCTION()
	void OnAttackColliderOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	bool GetAimingState() const { return IsAiming; }

public:
	void ToggleMove();
	void SetSprint(bool Sprint);
	void PlayAbility(const FGameplayTag& AbilityTag);

	void Change_AttackCollider(UPrimitiveComponent* AttackCollider, bool IsEquip);

private:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HeadMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairEquipMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USpringArmComponent>		SpringArm = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UCameraComponent>			Camera = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent>			HandAttackCollider = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent>			InteractDetectCollider = { nullptr };

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDC1EquipComponent>		Equipment = { nullptr };

private:
	void	InitializeAbilitySystem();
	void	Clear_Resource();

private:
	void	OnWeaponStateChanged(const FGameplayTag Tag, int32 NewCount);
	void	OnAimStateChanged(const FGameplayTag Tag, int32 NewCount);
	void	OnZoomStateChanged(const FGameplayTag Tag, int32 NewCount);

	void	RegisterStateHandle(const FGameplayTag& Tag, void (ADC1Player::*InFunc)(FGameplayTag, int32), UDC1AbilitySystemComponent* ASC);

private:
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent>		ActiveCollider = { nullptr };

	// 판정 창 하나에서 이미 때린 대상. 창이 열릴 때마다 비운다
	TSet<TWeakObjectPtr<AActor>>		HitTargetsInWindow;

private:
	TMap<FGameplayTag, FDelegateHandle> WeaponStateTagHandles;
	FDelegateHandle								AimStateTagHandle;
	FDelegateHandle								ZoomInStateTagHandle;
	TMap<FGameplayTag, FDelegateHandle>	PlayerStateTagHandles;

private:
	bool	IsAiming = {};

	float	FixedTargetArmLength = {};
	float	CurrentTargetArmLength = {};
	float	ZoomInLength = { 70.f };
	float	ZoomInSpeed = {5.f};
};
