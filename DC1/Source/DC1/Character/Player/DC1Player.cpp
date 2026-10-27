// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1Player.h"

#include "Define/DC1Macro.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraComponent.h"

#include "Character/Player/DC1PlayerState.h"
#include "Character/Player/DC1PlayerController.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "Component/Build/DC1BuildPlacementComponent.h"

#include "Interface/DC1WorkTargetInterface.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1CharacterMovementComponent.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Appearance/DC1AppearanceComponent.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Subsystem/DC1UIManagerSubsystem.h"
#include "Component/Widget/DC1ScreenOffsetWidgetComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"

#include "Tags/DC1GamePlayTags.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Anim/DC1AnimData.h"

ADC1Player::ADC1Player(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDC1CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	USkeletalMeshComponent* BodyMesh = GetMesh();
	BodyMesh->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -90.f), FRotator(0.f, -90.f, 0.f));
	BodyMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// HeadMesh
	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HeadMesh");
	HeadMesh->SetupAttachment(BodyMesh);
	// HairMesh & HairEquipMesh
	HairMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HairMesh");
	HairMesh->SetupAttachment(BodyMesh);
	HairEquipMesh = CreateDefaultSubobject<USkeletalMeshComponent>("HairEquipMesh");
	HairEquipMesh->SetupAttachment(BodyMesh);

	// SpringArm
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("SpringArm");
	SpringArm->SetupAttachment(BodyMesh);
	SpringArm->SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	SpringArm->TargetArmLength = 200.f;
	SpringArm->bUsePawnControlRotation = true;

	FixedTargetArmLength = SpringArm->TargetArmLength;
	CurrentTargetArmLength = FixedTargetArmLength;
	// Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm);
	Camera->SetRelativeLocation(FVector(0.f, 50.f, 0.f));
	Camera->bUsePawnControlRotation = false;

	// Attack Collider
	HandAttackCollider = CreateDefaultSubobject<USphereComponent>("HandAttackCollider");
	HandAttackCollider->SetupAttachment(RootComponent);
	HandAttackCollider->SetRelativeLocation(FVector(60.f, 0.f, 0.f));
	HandAttackCollider->SetSphereRadius(50.f);
	HandAttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Interact Collider
	InteractDetectCollider = CreateDefaultSubobject<USphereComponent>("InteractDetectCollider ");
	InteractDetectCollider->SetupAttachment(RootComponent);
	InteractDetectCollider->SetGenerateOverlapEvents(true);

	// Stamina Widget
	StaminaWidgetComponent = CreateDefaultSubobject<UWidgetComponent>("StaminaWidget");
	StaminaWidgetComponent->SetupAttachment(RootComponent);
	StaminaWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	StaminaWidgetComponent->SetVisibility(false);

	// InteractPrompWidget
	InteractPromptWidgetComponent = CreateDefaultSubobject<UDC1ScreenOffsetWidgetComponent>("InteractPrompWidget");
	InteractPromptWidgetComponent->SetupAttachment(RootComponent);
	InteractPromptWidgetComponent->SetUsingAbsoluteLocation(true);
	InteractPromptWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	InteractPromptWidgetComponent->SetDrawAtDesiredSize(true);
	InteractPromptWidgetComponent->SetPivot({ 0.5f, 0.5f });
	InteractPromptWidgetComponent->SetVisibility(false);
	InteractPromptWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* ADC1Player::GetAbilitySystemComponent() const
{
	const ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();

	// 탑승 중엔 PlayerState 가 탄 팰에 있다
	const APawn* RiddenPawn = GetRiddenPawn();
	if (nullptr == PS && nullptr != RiddenPawn)
	{
		PS = RiddenPawn->GetPlayerState<ADC1PlayerState>();
	}

	return (nullptr != PS) ? PS->GetAbilitySystemComponent() : nullptr;
}

void ADC1Player::SetAttackCollision(bool OnCollision)
{
	if (nullptr == ActiveCollider) return;

	// 한 몽타주에 주먹이 여러 번 들어있으면 창마다 새로 판정해야 한다
	if (true == OnCollision)
	{
		HitTargetsInWindow.Reset();
	}

	ECollisionEnabled::Type CollisionType = OnCollision ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision;
	ActiveCollider->SetCollisionEnabled(CollisionType);
}

UDC1EquipComponent* ADC1Player::GetEquipComponent() const
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS))
		return nullptr;
	
	return PS->GetEquipComponent();
}

IDC1WorkTargetInterface* ADC1Player::GetCurrentWorkTarget() const
{
	const ADC1PlayerController* PC = GetController<ADC1PlayerController>();
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	const UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
	if (false == ::IsValid(InteractComp))
	{
		return nullptr;
	}

	return Cast<IDC1WorkTargetInterface>(InteractComp->GetCurrentTarget());
}

void ADC1Player::OnAttackColliderOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 물리적으로 말이 안 되는 것과 창 단위 중복만 여기서 거른다. 팀 판정은 GA가 한다.
	if (false == ::IsValid(OtherActor) || this == OtherActor) return;

	if (true == HitTargetsInWindow.Contains(OtherActor)) return;
	HitTargetsInWindow.Add(OtherActor);

	FGameplayEventData Payload;
	Payload.Instigator = this;
	Payload.Target = OtherActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, DC1PlayerTags::Event_Player_Hit, Payload);
}

bool ADC1Player::CanClimb() const
{
	return CheckRemainStamina(20.f);
}

bool ADC1Player::CanGlide() const
{
	if (UDC1EquipComponent* EquipComponent = GetEquipComponent())
	{
		ADC1Glider* Glider = EquipComponent->GetCurrentGlider();
		if (nullptr == Glider)
			return false;
	}

	return CheckRemainStamina(0.f);
}

void ADC1Player::Server_ReleaseClimb_Implementation()
{
	UDC1CharacterMovementComponent* CMC = CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement());
	if (false == IsValid(CMC))
		return;

	CMC->ReleaseClimb();
}

void ADC1Player::ToggleMove()
{
	CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement())->ToggleMove();
}

void ADC1Player::SetSprint(bool Sprint)
{
	// 조준 중에는 스프린트 진입만 막는다. 해제(false)는 통과시켜야 한다
	if (true == Sprint && true == IsAiming) return;

	CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement())->SetSprint(Sprint);
}

void ADC1Player::PlayAbility(const FGameplayTag& AbilityTag)
{
	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->ActivateAbilityByTag(AbilityTag);
	}
}

void ADC1Player::CancelAbility(const FGameplayTag& AbilityTag)
{
	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->CancelAbilityByTag(AbilityTag);
	}
}

void ADC1Player::TryJumpOrGlide()
{
	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(GetCharacterMovement());
	if (false == IsValid(CMC))
		return;

	// Falling -> Gliding
	if (true == CMC->IsFalling())
	{
		if(true == CMC->CanGliding())
			CMC->SetGlide(true);
	}
	// Gliding -> Falling
	else if (true == CMC->IsGliding())
	{
		CMC->SetGlide(false);
	}
	// Jump
	else
	{
		Jump();
	}
}

void ADC1Player::Change_AttackCollider(UPrimitiveComponent* AttackCollider, bool IsEquip)
{
	UPrimitiveComponent* NewCollider = (nullptr == AttackCollider && false == IsEquip) ? HandAttackCollider : AttackCollider;
	
	if (ActiveCollider == NewCollider || nullptr == NewCollider) return;

	if(IsValid(ActiveCollider))
		ActiveCollider->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnAttackColliderOverlap);

	ActiveCollider = NewCollider;
	ActiveCollider->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnAttackColliderOverlap);
}

void ADC1Player::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	HairMesh->SetLeaderPoseComponent(GetMesh());
	HairEquipMesh->SetLeaderPoseComponent(GetMesh());
}

void ADC1Player::BeginPlay()
{
	Super::BeginPlay();
}

void ADC1Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CurrentTargetArmLength, DeltaTime, ZoomInSpeed);
}

void ADC1Player::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Clear_Resource();
	Super::EndPlay(EndPlayReason);
}

void ADC1Player::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 하차는 재초기화 안 함
	if (nullptr != GetRiddenPawn())
	{
		return;
	}

	InitializeAbilitySystem();
	InitializeEquip();
	InitializeAppearance();
}

APawn* ADC1Player::GetRiddenPawn() const
{
	return Cast<APawn>(GetAttachParentActor());
}

FRotator ADC1Player::GetViewRotation() const
{
	const APawn* RiddenPawn = GetRiddenPawn();
	if (nullptr != RiddenPawn && nullptr != RiddenPawn->GetController())
	{
		return RiddenPawn->GetController()->GetControlRotation();
	}

	return Super::GetViewRotation();
}

void ADC1Player::UnPossessed()
{
	// 탑승은 정리 안 함
	if (nullptr == GetRiddenPawn())
	{
		Clear_Resource();
	}

	Super::UnPossessed();
}

void ADC1Player::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
	InitializeEquip();
	InitializeAppearance();
}

void ADC1Player::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	StaminaWidgetComponent->SetVisibility(IsLocallyControlled());
}

void ADC1Player::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	// CMC가 MovementMode를 이미 갱신한 뒤 호출된다
	if (UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(GetCharacterMovement()))
	{
		bool OnGliding = CMC->IsGliding();
		bool OnClimbing = CMC->IsClimbing();

		ApplyGlideState(OnGliding);
		ApplyClimbState(OnClimbing);

		// Weapon Visiblity Setting
		if (UDC1EquipComponent* EquipComponent = GetEquipComponent())
		{
			EquipComponent->RefreshWeaponVisibility();
		}

		const bool WasClimbing = (PrevMovementMode == MOVE_Custom && PreviousCustomMode == CMOVE_Climb);
		if (true == HasAuthority() && true == WasClimbing && 
			true == CMC->IsFalling() && true == bPressedJump)
		{
			ApplyClimbJumpStamina();
		}
	}
}

void ADC1Player::OnHealthDepleted()
{
	// 이미 사망했으면 무시 ( 기절 중 추가 피격은 기절 게이지가 처리 )
	if (true == bDead)
		return;

	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();

	if (true == HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			// 이미 기절 중이면 무시
			if (true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Downed))
				return;

			// 진행 중인 Ability 취소 ( 벽타기, 활공, 작업, 조준 등 )
			ASC->CancelAllAbilities();

			// Down Ability 실행
			PlayAbility(DC1PlayerTags::Ability_Player_Downed);
		}

		// 하차만 ( 팰 회수는 사망 시 )
		if (nullptr != PS)
		{
			if (UDC1PalPartyComponent* PalParty = PS->GetPalPartyComponent())
			{
				PalParty->HandleOwnerDowned();
			}
		}
	}

	// 본인 클라만 UI 처리. 탑승 중엔 팰을 조종하고 있어 IsLocallyControlled가 false라 PC로 판단한다
	const APlayerController* PC = (nullptr != PS) ? PS->GetPlayerController() : nullptr;
	if (nullptr == PC || false == PC->IsLocalController())
		return;

	if (UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
	{
		// 열려 있는 화면 정리 ( 텔레포트, 인벤토리 등 )
		while (true == UIManager->HasActiveScreen())
		{
			UIManager->PopScreen();
		}
	}
}

void ADC1Player::OnDeath()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();

	if (true == HasAuthority())
	{
		DeathTime = GetWorld()->GetTimeSeconds();

		// 기절 → 사망. 기절 태그는 사망으로 대체한다
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			CancelAbility(DC1PlayerTags::Ability_Player_Downed);
		}

		// 소환 팰 회수 ( 하차는 기절 때 이미 처리 )
		if (nullptr != PS)
		{
			if (UDC1PalPartyComponent* PalParty = PS->GetPalPartyComponent())
			{
				PalParty->HandleOwnerDied();
			}
		}
	}

	// 사망 몽타주 ( 서버 / 본인 / 다른 클라 각자 재생 - HP가 모두에게 복제되어 OnDeath가 각자 불린다 )
	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		UAnimMontage* DeathMontage = AnimData->FindMontageByTag(DC1AnimTags::Anim_Action_Death);
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (true == IsValid(DeathMontage) && nullptr != AnimInstance)
		{
			AnimInstance->Montage_Play(DeathMontage);
		}
	}

	// 이동 정지 ( 서버 + 본인 클라 예측 ). 기절 중엔 이동이 가능하므로 사망에서 막는다
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
		CMC->DisableMovement();
	}

	// 본인 클라만 UI 처리. 탑승 중엔 팰을 조종하고 있어 IsLocallyControlled가 false라 PC로 판단한다
	const APlayerController* PC = (nullptr != PS) ? PS->GetPlayerController() : nullptr;
	if (nullptr == PC || false == PC->IsLocalController())
		return;

	if (UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
	{
		// 사망 화면은 UI 제작 후 여기서 Push ( 열린 화면 정리는 기절 때 처리 )
	}
}

void ADC1Player::ApplyClimbState(bool IsApply)
{
	// 보정 후 리플레이로 여러 번 호출될 수 있어 Add/Remove(카운트 누적) 대신 값을 직접 설정한다
	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Climbing, IsApply ? 1 : 0);

		// 이동 여부 태그는 벽타기 중에만 의미가 있다. 이탈 시 남지 않게 끈다
		if (false == IsApply)
		{
			ASC->SetLooseGameplayTagCount(DC1StateTags::State_Climbing_Moving, 0);
		}

		if (true == HasAuthority())
		{
			if (true == IsApply)
			{
				if (false == ClimbStaminaCostHandle.IsValid())
				{
					if (false == IsValid(ClimbStaminaCostClass))
						return;

					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ClimbStaminaCostClass, 1.f, ASC->MakeEffectContext());

					if (SpecHandle.IsValid())
					{
						ClimbStaminaCostHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}	
			}
			else
			{
				RemoveGameplayEffect(ASC, ClimbStaminaCostHandle);
			}
		}
	}
}

void ADC1Player::RemoveGameplayEffect(UDC1AbilitySystemComponent* ASC, FActiveGameplayEffectHandle& Handle)
{
	if (nullptr == ASC || false == Handle.IsValid())
	{
		return;
	}

	// 제거 시 GE의 OnCompleteAlways(회복 지연 GE)가 이어받는다
	ASC->RemoveActiveGameplayEffect(Handle);

	// 핸들을 비우지 않으면 다음 행동에서 "이미 적용 중"으로 보고 적용을 건너뛴다
	Handle.Invalidate();
}

void ADC1Player::ApplyClimbJumpStamina()
{
	if (false == IsValid(ClimbJumpCostClass))
		return;

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(ClimbJumpCostClass, 1.f, ASC->MakeEffectContext());
		if (false == SpecHandle.IsValid())
			return;

		SpecHandle.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_StaminaCost, -ClimbJumpStaminaCost);

		ASC->ApplyGameplayEffectSpecToSelf(*(SpecHandle.Data).Get());
	}
}

bool ADC1Player::CheckRemainStamina(float Stamina) const
{
	UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent());
	if (nullptr == ASC)
		return false;

	if (ASC->GetNumericAttribute(UDC1CharacterAttributeSet::GetStaminaAttribute()) <= Stamina)
		return false;

	return true;
}

void ADC1Player::ApplyGlideState(bool IsApply)
{
	// 보정 후 리플레이로 여러 번 호출될 수 있어 Add/Remove(카운트 누적) 대신 값을 직접 설정한다
	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Gliding, IsApply ? 1 : 0);

		if (UDC1EquipComponent* EquipComponent = GetEquipComponent())
		{
			EquipComponent->SetGliderVisibility(IsApply);
		}

		if (true == HasAuthority())
		{
			if (true == IsApply)
			{
				if (false == GlideStaminaCostHandle.IsValid())
				{
					if (false == IsValid(GlideStaminaCostClass))
						return;

					FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GlideStaminaCostClass, 1.f, ASC->MakeEffectContext());

					if (SpecHandle.IsValid())
					{
						GlideStaminaCostHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					}
				}
			}
			else
			{
				RemoveGameplayEffect(ASC, GlideStaminaCostHandle);
			}
		}
	}
}

FGameplayTag ADC1Player::ConvertWeaponTagToAnimTag(const FGameplayTag& Tag)
{
	// 무기 상태 태그 -> Draw 몽타주 태그. 매핑에 없는 태그는 빈 태그를 돌려준다
	if (Tag.MatchesTagExact(DC1StateTags::State_Weapon_None))		return DC1AnimTags::Anim_Draw_None;
	if (Tag.MatchesTagExact(DC1StateTags::State_Weapon_Melee))	return DC1AnimTags::Anim_Draw_Melee;
	if (Tag.MatchesTagExact(DC1StateTags::State_Weapon_Bow))		return DC1AnimTags::Anim_Draw_Bow;

	return FGameplayTag();
}

void ADC1Player::HandleOnCharacterMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	// 소모 GE는 서버에서만 계산된다
	if (false == HasAuthority())
		return;

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(GetCharacterMovement());
	if (nullptr == CMC || false == CMC->IsClimbing())
		return;   // 벽타기 밖에서는 ApplyClimbState(false)가 이미 0으로 꺼둠

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (nullptr == ASC)
		return;

	const bool bMoving = GetVelocity().SizeSquared() > FMath::Square(ClimbMovingSpeedThreshold);
	ASC->SetLooseGameplayTagCount(DC1StateTags::State_Climbing_Moving, bMoving ? 1 : 0);
}

void ADC1Player::HandleDamaged(const FOnAttributeChangeData& Data)
{
	if (false == HasAuthority() || Data.NewValue <= 0.f || nullptr == Data.GEModData)
		return;

	FGameplayEventData Payload = {};
	Payload.ContextHandle = Data.GEModData->EffectSpec.GetContext();
	Payload.Target = this;
	Payload.Instigator = Data.GEModData->EffectSpec.GetContext().GetEffectCauser();
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, DC1PlayerTags::Event_Player_Damaged, Payload);
}

void ADC1Player::InitializeEquip()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS)) return;

	if (UDC1EquipComponent* Equip = PS->GetEquipComponent())
	{
		Equip->HandlePawnReady();
	}
}

void ADC1Player::InitializeAppearance()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS)) return;

	if (UDC1AppearanceComponent* Appearance = PS->GetAppearanceComponent())
	{
		Appearance->SetParts(GetMesh(), HairMesh, HeadMesh);
	}
}

void ADC1Player::InitializeAbilitySystem()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS))
	{
		return;
	}
	UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
	ASC->InitAbilitySystem(PS, this, DC1AssetTags::Asset_Ability_Player);

	// Health Delegate Bind
	BindHealthWatcher();

	// Weapon State Change Delegate Register
	if (WeaponStateTagHandles.Num() == 0)
	{
		if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
		{
			for (const FGameplayTag& Tag : AnimData->GetAnimInstanceKeys())
			{
				WeaponStateTagHandles.Add(Tag,
					ASC->RegisterAndCallGameplayTagEvent(Tag,
						FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(
							this, &ThisClass::OnWeaponStateChanged)));
			}
		}
	}

	// Aim State Delegate Register
	RegisterStateHandle(DC1StateTags::State_Aiming, &ThisClass::OnAimStateChanged, ASC);

	// ZoomIn State Delegate Register
	RegisterStateHandle(DC1StateTags::State_ZoomIn, &ThisClass::OnZoomStateChanged, ASC);

	// Down State Delegate Register
	RegisterStateHandle(DC1StateTags::State_Downed, &ThisClass::OnDownStateChanged, ASC);

	// Dead State Delegate Register
	RegisterStateHandle(DC1StateTags::State_Dead, &ThisClass::OnDeadStateChanged, ASC);

	// CharacterMovement Delegate Register
	OnCharacterMovementUpdated.AddUniqueDynamic(this, &ThisClass::HandleOnCharacterMovementUpdated);

	// Changed Damage Delegate Register
	if (false == DamagedHandle.IsValid())
	{
		DamagedHandle = ASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetDamageAttribute()).AddUObject(this, &ThisClass::HandleDamaged);
	}
}

void ADC1Player::Clear_Resource()
{
	OnCharacterMovementUpdated.RemoveAll(this);

	if (ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>())
	{
		UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
		if (nullptr == ASC)
			return;
		
		ASC->SetAvatarActor(nullptr);

		for (TPair pair : WeaponStateTagHandles)
		{
			ASC->UnregisterGameplayTagEvent(pair.Value, pair.Key);
		}
		WeaponStateTagHandles.Reset();

		for (auto& Pair : PlayerStateTagHandles)
		{
			if (Pair.Value.IsValid())
			{
				ASC->UnregisterGameplayTagEvent(Pair.Value, Pair.Key);
				Pair.Value.Reset();
			}
		}
		PlayerStateTagHandles.Reset();

		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Gliding, 0);
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Climbing, 0);
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Climbing_Moving, 0);

		if (true == HasAuthority())
		{
			RemoveGameplayEffect(ASC, ClimbStaminaCostHandle);
			RemoveGameplayEffect(ASC, GlideStaminaCostHandle);
		}

		ASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetDamageAttribute()).RemoveAll(this);
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).RemoveAll(this);
	}
}

void ADC1Player::OnWeaponStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount <= 0) return;

	if (const UDC1AnimData* AnimData = UDC1AssetManager::GetAssetByTag<UDC1AnimData>(DC1AssetTags::Asset_Anim_Player))
	{
		if (TSubclassOf<UAnimInstance> LayerClass = AnimData->FindAnimInstanceByTag(Tag))
		{
			if (UAnimInstance* Instance = GetMesh()->GetAnimInstance())
			{
				Instance->LinkAnimClassLayers(LayerClass);

				if (false == OldWeaponTag.MatchesTag(Tag))
				{
					OldWeaponTag = Tag;

					UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(GetCharacterMovement());
					if (false == IsValid(CMC))
						return;
					// 무기 교체로 인한 Visibility, Draw Animation 차단
					if (true == CMC->IsWeaponBlockedByMovement())
						return;

					FGameplayTag DrawAnimTag = ConvertWeaponTagToAnimTag(Tag);
					if (false == DrawAnimTag.IsValid())
						return;
					UAnimMontage* DrawAnimMontage = AnimData->FindMontageByTag(DrawAnimTag);
					if (false == IsValid(DrawAnimMontage))
						return;
					Instance->Montage_Play(DrawAnimMontage);
				}
			}
		}
	}
}

void ADC1Player::OnAimStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	IsAiming = NewCount > 0;

	if (IsAiming)
	{
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;

		// 스프린트 의도의 주인은 소유 클라다. 서버에서 꺼봐야 다음 ServerMove가 되돌린다
		if (true == IsLocallyControlled())
		{
			CancelAbility(DC1PlayerTags::Ability_Player_Sprint);
			CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement())->SetWalk(true);
		}
	}
	else
	{
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;

		if (true == IsLocallyControlled())
		{
			CastChecked<UDC1CharacterMovementComponent>(GetCharacterMovement())->SetWalk(false);
		}
	}
}

void ADC1Player::OnZoomStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	CurrentTargetArmLength = NewCount > 0 ? FixedTargetArmLength - ZoomInLength : FixedTargetArmLength;
}

void ADC1Player::OnDownStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	IsDown = NewCount > 0;

	// BuildMode, InteractEnable 관리
	RefreshIncapacitated();
}

void ADC1Player::OnDeadStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	// 클라는 Down 해제가 Dead보다 먼저 올 수 있어 Dead에서도 한 번 더 갱신
	RefreshIncapacitated();
}

void ADC1Player::RefreshIncapacitated()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (nullptr == PS)
		return;

	ADC1PlayerController* PC = Cast<ADC1PlayerController>(PS->GetPlayerController());
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
		return;

	const bool IsDead = IsDownOrDead();

	// 빌드 취소가 먼저 -> SetBuildMode(false)가 SetInteractEnabled(true) 호출
	UDC1BuildPlacementComponent* BuildComp = PC->GetBuildPlacementComponent();
	if (true == IsDead && true == ::IsValid(BuildComp))
	{
		BuildComp->CancelBuildMode();
	}

	UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
	if (true == ::IsValid(InteractComp))
	{
		InteractComp->SetInteractEnabled(false == IsDead);
	}
}

void ADC1Player::RegisterStateHandle(const FGameplayTag& Tag, void (ADC1Player::*InFunc)(FGameplayTag, int32), UDC1AbilitySystemComponent* ASC)
{
	if (nullptr == ASC || nullptr != PlayerStateTagHandles.Find(Tag))
		return;

	PlayerStateTagHandles.Add(Tag, 
		ASC->RegisterAndCallGameplayTagEvent(Tag,
		FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(this, InFunc)));
}
