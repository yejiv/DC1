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
class USoundBase;
class UAudioComponent;

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

	// 다른 플레이어도 들어야 하는 월드 사운드 재생 ( 서버 - 전원 Multicast / 클라 - 본인 즉시 재생 + 서버 경유 )
	void PlayWorldSound(USoundBase* InSound, const FVector& InLocation);

	// 보호막 피격 / 파괴음 ( 서버 PlayerSet 데미지 처리에서 재생 )
	USoundBase* GetShieldHitSound() const { return ShieldHitSound; }
	USoundBase* GetShieldBreakSound() const { return ShieldBreakSound; }
	USoundBase* GetHitSound() const { return HitSound; }

private:
	UFUNCTION(Server, Unreliable)
	void Server_PlayWorldSound(USoundBase* InSound, FVector_NetQuantize InLocation);

	// InSkipLocal : 요청한 클라는 이미 재생했으므로 건너뜀
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayWorldSound(USoundBase* InSound, FVector_NetQuantize InLocation, bool InSkipLocal);

public:
	void ToggleMove();
	void SetSprint(bool Sprint);
	void PlayAbility(const FGameplayTag& AbilityTag);
	void CancelAbility(const FGameplayTag& AbilityTag);

	void TryJumpOrGlide();

	void Change_AttackCollider(UPrimitiveComponent* AttackCollider, bool IsEquip);

	// Stamina 잔존량 체크
	bool	CheckRemainStamina(float Stamina) const;

	// Body, HairEquip Mesh 교체, ( InAttachBoneName + InAttachOffset = HairEquip 전용, Body는 사용안함 )
	void Change_EquipMesh(USkeletalMesh* InMesh, EDC1EquipSlotType InSlotType, FName InAttachBoneName = NAME_None, const FTransform& InAttachOffset = FTransform::Identity);

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

	// 앉기 사운드 ( bIsCrouched 복제로 다른 클라에서도 호출됨 )
	virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
	virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

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

	// 이동 모드 전환 사운드 ( 착지, 활공 시작/종료, 벽 붙기 ). 시뮬레이티드 프록시도 OnMovementModeChanged를 거친다
	void	PlayMovementModeSound(EMovementMode PrevMovementMode, uint8 PreviousCustomMode, bool OnGliding, bool OnClimbing);
	void	SetGlideLoopSound(bool IsPlay);

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

	// 사망 징글 DeathSounds 를 순서대로 한 번씩 재생 ( 본인 화면만 )
	void	PlayDeathSoundAt(int32 InIndex);
	UFUNCTION()
	void	HandleDeathSoundFinished();

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

	// 변형 여러 개 중 랜덤 재생
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TArray<TObjectPtr<USoundBase>>					LandingSounds;

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							CrouchSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							UnCrouchSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							GlideStartSound = { nullptr };

	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							GlideEndSound = { nullptr };

	// 활공 중 반복 재생 ( 사운드 에셋 Looping 켜져 있어야 함 )
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							GlideLoopSound = { nullptr };

	// 벽에 붙는 순간
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TArray<TObjectPtr<USoundBase>>					ClimbStartSounds;

	// 보호막이 남아 있는 상태로 맞았을 때
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							ShieldHitSound = { nullptr };

	// 보호막이 0이 되는 순간 ( 피격음 대신 재생 )
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							ShieldBreakSound = { nullptr };

	// 보호막 없이 체력이 깎일 때
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TObjectPtr<USoundBase>							HitSound = { nullptr };

	// 사망 시 본인 화면에서 배열 순서대로 이어서 한 번 재생
	UPROPERTY(EditDefaultsOnly, Category = "Sound")
	TArray<TObjectPtr<USoundBase>>					DeathSounds;

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent>						GlideLoopAudio = { nullptr };

	UPROPERTY(Transient)
	TObjectPtr<UAudioComponent>						DeathAudio = { nullptr };

	int32											DeathSoundIndex = { INDEX_NONE };

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
