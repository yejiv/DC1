// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "DC1Player.generated.h"

class USkeletalMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UPrimitiveComponent;
class USphereComponent;
class UWidgetComponent;
class UDC1ScreenOffsetWidgetComponent;

class UGameplayEffect;

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
	UDC1ScreenOffsetWidgetComponent* GetInteractPromptWidgetComponent() const { return InteractPromptWidgetComponent; }
	UDC1EquipComponent* GetEquipComponent() const;

	// 상호작용 컴포넌트의 현재 대상 ( 로컬 - Work GA가 IsLocallyControlled에서 호출 )
	virtual IDC1WorkTargetInterface* GetCurrentWorkTarget() const override;

public:
	UFUNCTION()
	void OnAttackColliderOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	bool GetAimingState() const { return IsAiming; }
	bool GetDownState() const { return IsDown; }

	// 사망 시각 ( 서버 ). 부활 대기 시간 검증용
	float GetDeathTime() const { return DeathTime; }

	bool CanClimb() const;
	bool CanGlide() const;

public:
	UFUNCTION(Server, Reliable)
	void Server_ReleaseClimb();

public:
	void ToggleMove();
	void SetSprint(bool Sprint);
	void PlayAbility(const FGameplayTag& AbilityTag);
	void CancelAbility(const FGameplayTag& AbilityTag);

	void TryJumpOrGlide();

	void Change_AttackCollider(UPrimitiveComponent* AttackCollider, bool IsEquip);

	// Stamina 잔존량 체크
	bool	CheckRemainStamina(float Stamina) const;

private:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void OnRep_PlayerState() override;
	virtual void NotifyControllerChanged() override;

	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode = 0) override;

	virtual void OnHealthDepleted() override;
	virtual void OnDeath() override;

public:
	// 탑승 = 폰에 붙어 있음. 아니면 null
	APawn*				GetRiddenPawn() const;

	// 탑승 중엔 팰 컨트롤러 시점
	virtual FRotator GetViewRotation() const override;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent>			StaminaWidgetComponent = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDC1ScreenOffsetWidgetComponent>		InteractPromptWidgetComponent = { nullptr };

private:
	void	InitializeAbilitySystem();
	// 장착 컴포넌트는 PlayerState 소유다. 폰이 준비됐음을 알려 스폰/부착을 트리거한다
	void	InitializeEquip();
	// AppearanceComponent도 PlayerState 소유. 폰의 3개 메쉬를 등록(SetParts)한다
	void	InitializeAppearance();
	void	Clear_Resource();

private:
	void	OnWeaponStateChanged(const FGameplayTag Tag, int32 NewCount);
	void	OnAimStateChanged(const FGameplayTag Tag, int32 NewCount);
	void	OnZoomStateChanged(const FGameplayTag Tag, int32 NewCount);
	void	OnDownStateChanged(const FGameplayTag Tag, int32 NewCount);
	void	OnDeadStateChanged(const FGameplayTag Tag, int32 NewCount);

	// 기절 + 사망 공통 처리 - 현재 태그 상태로 판단 ( Down → Dead 전환 시 태그 도착 순서 무관 )
	void	RefreshIncapacitated();

	void	RegisterStateHandle(const FGameplayTag& Tag, void (ADC1Player::*InFunc)(FGameplayTag, int32), UDC1AbilitySystemComponent* ASC);

	// 벽타기 진입/이탈 시 State.Climbing 태그 반영. 이동 모드가 바뀌는 클라(예측)·서버 양쪽에서 호출된다
	void	ApplyClimbState(bool IsApply);
	// 서버 전용. 소모 GE 제거 + 핸들 무효화
	void	RemoveGameplayEffect(UDC1AbilitySystemComponent* ASC, FActiveGameplayEffectHandle& Handle);
	// 서버 전용. 벽 타는 도중 점프 시 스태미나 소모
	void	ApplyClimbJumpStamina();

	void	ApplyGlideState(bool IsApply);

private:
	FGameplayTag ConvertWeaponTagToAnimTag(const FGameplayTag& Tag);

private:
	UFUNCTION()
	void	HandleOnCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity);

	void	HandleDamaged(const struct FOnAttributeChangeData& Data);

private:
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent>		ActiveCollider = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Glide")
	TSubclassOf<UGameplayEffect>			GlideStaminaCostClass = { nullptr };
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	TSubclassOf<UGameplayEffect>			ClimbStaminaCostClass = { nullptr };
	// 벽 점프 1회 비용(Instant, SetByCaller.StaminaCost). GE_Stamina_Cost 지정
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	TSubclassOf<UGameplayEffect>			ClimbJumpCostClass = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float											ClimbMovingSpeedThreshold = { 1.f };
	UPROPERTY(EditDefaultsOnly, Category = "Climb")
	float											ClimbJumpStaminaCost = { 10.f };

	// 판정 창 하나에서 이미 때린 대상. 창이 열릴 때마다 비운다
	TSet<TWeakObjectPtr<AActor>>		HitTargetsInWindow;

private:
	FGameplayTag								OldWeaponTag = {};

private:
	TMap<FGameplayTag, FDelegateHandle> WeaponStateTagHandles;
	FDelegateHandle								AimStateTagHandle;
	FDelegateHandle								ZoomInStateTagHandle;
	TMap<FGameplayTag, FDelegateHandle>	PlayerStateTagHandles;
	FDelegateHandle								DamagedHandle;

	FActiveGameplayEffectHandle				ClimbStaminaCostHandle;
	FActiveGameplayEffectHandle				GlideStaminaCostHandle;

private:
	bool	IsAiming = {};
	bool	IsDown = {};

	float	DeathTime = { 0.f };

	float	FixedTargetArmLength = {};
	float	CurrentTargetArmLength = {};
	float	ZoomInLength = { 70.f };
	float	ZoomInSpeed = {5.f};
};
