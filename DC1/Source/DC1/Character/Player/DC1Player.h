// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/DC1Character.h"
#include "Interface/DC1InteractableInterface.h"
#include "GameplayTagContainer.h"
#include "Define/DC1Enums.h"
#include "ActiveGameplayEffectHandle.h"
#include "DC1Player.generated.h"

class USkeletalMesh;
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
class DC1_API ADC1Player : public ADC1Character, public IDC1InteractableInterface
{
	GENERATED_BODY()

public:
	ADC1Player(const FObjectInitializer& ObjectInitializer);

public:
	FDC1OnInteractOptionsChanged	OnInteractOptionsChanged;

public:
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	virtual void SetAttackCollision(bool OnCollision) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	USphereComponent* GetInteractDetectCollider() const { return InteractDetectCollider; }
	UDC1ScreenOffsetWidgetComponent* GetInteractPromptWidgetComponent() const { return InteractPromptWidgetComponent; }
	UDC1EquipComponent* GetEquipComponent() const;

	// 상호작용 컴포넌트의 현재 대상 ( 로컬 - Work GA가 IsLocallyControlled에서 호출 )
	virtual IDC1WorkTargetInterface* GetCurrentWorkTarget() const override;

// IDC1InteractableInterface override
public:
	// 시선 판정 기준점을 위한 InteractCollider 가져오기
	virtual UPrimitiveComponent*	GetInteractCollider() const override;

	// 치료 상호작용 옵션
	virtual void							GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const override;

	// 옵션 목록이 바뀔 때 Broadcast - 포커스 중 UI/키 판정 갱신용
	virtual FDC1OnInteractOptionsChanged& GetOnInteractOptionsChanged() override { return OnInteractOptionsChanged; }

public:
	UFUNCTION()
	void OnAttackColliderOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

public:
	bool GetAimingState() const { return IsAiming; }
	bool GetDownState() const { return IsDown; }

	// 부활 가능 시각 ( 서버 월드 시간 기준 ). 클라는 GetServerWorldTimeSeconds와 비교
	float GetRespawnAvailableTime() const { return RespawnAvailableTime; }

	bool CanClimb() const;
	bool CanGlide() const;

	// 치료 Interact
	bool	CanBeRevived() const;
	void	StartBeingRevived();
	void	StopBeingRevived();
	void	CompleteRevived();

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

	// Body, HairEquip Mesh 교체
	void Change_EquipMesh(USkeletalMesh* InMesh, EDC1EquipSlotType InSlotType);

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
	// 탄 팰. 안 타면 null
	APawn*				GetRiddenPawn() const { return RiddenPawn; }
	// 서버 전용. 파티 컴포넌트가 탈 때·다 내릴 때 부른다
	void				SetRiddenPawn(APawn* InRiddenPawn) { RiddenPawn = InRiddenPawn; }

	// 탑승 중엔 팰 컨트롤러 시점
	virtual FRotator GetViewRotation() const override;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HeadMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairMesh = { nullptr };
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMeshComponent>	HairEquipMesh = { nullptr };
	// Body 방어구 해제(nullptr) 시 되돌릴 기본 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USkeletalMesh>			DefaultBodyMesh = { nullptr };
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

	// 이 기계의 로컬 플레이어가 조종했던 캐릭터인가. 탑승으로 조종이 팰로 넘어가도 스태미나 위젯을 유지한다
	bool									IsLocalPlayerCharacter = { false };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UDC1ScreenOffsetWidgetComponent>		InteractPromptWidgetComponent = { nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USphereComponent>		InteractCollider = { nullptr };


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

	// 기절 게이지 감시 ( 서버 / 본인 / 다른 클라 모두 ). 0이 되면 사망
	void	BindDownedGaugeWatcher();
	void	HandleDownedGaugeChanged(const struct FOnAttributeChangeData& Data);

private:
	void	ShowDeathScreen();

private:
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent>	ActiveCollider = { nullptr };

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
	// 치료 상호작용 옵션
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	TArray<FDC1InteractOption>			InteractOptions;

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

	FTimerHandle									DeathScreenTimerHandle;

private:
	bool	IsAiming = {};
	bool	IsDown = {};
	bool	IsIncapacitated = {};

	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float	DeathScreenDelay = { 2.f };
	UPROPERTY(EditDefaultsOnly, Category = "Death")
	float	RespawnDelay = { 5.f };

	float	FixedTargetArmLength = {};
	float	CurrentTargetArmLength = {};
	float	ZoomInLength = { 70.f };
	float	ZoomInSpeed = {5.f};

	// 스프링 암 기본 높이
	float	FixedSpringArmHeight = {};
	float	CurrentSpringArmHeight = {};

	// 팰 탑승 중 스프링 암 높이
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	float	RideSpringArmHeight = { 50.f };

private:
	UPROPERTY(Replicated)
	float	RespawnAvailableTime = { };

	// 탄 팰. 붙기 전에 넣고 뗀 뒤에 비운다
	UPROPERTY(Replicated)
	TObjectPtr<APawn>	RiddenPawn = { nullptr };
};
