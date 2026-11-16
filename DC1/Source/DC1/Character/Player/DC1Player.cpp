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
#include "Subsystem/DC1ViewModelSubsystem.h"
#include "ViewModel/Teleport/DC1TeleportViewModel.h"
#include "Component/Widget/DC1ScreenOffsetWidgetComponent.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

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

	// Interact Collider
	InteractCollider = CreateDefaultSubobject<USphereComponent>("InteractCollider");
	InteractCollider->SetupAttachment(RootComponent);
	InteractCollider->SetCollisionProfileName(DC1CollisionProfile::Interaction);
	InteractCollider->SetGenerateOverlapEvents(true);
	InteractCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Interact Option
	FDC1InteractOption InteractOption = {};
	InteractOption.InputType = EDC1InteractInputType::HoldAbility;
	InteractOption.Key = EDC1InteractKey::F;
	InteractOption.DisplayText = FText::FromString(TEXT("치료하기"));
	InteractOption.InteractionTag = DC1InteractTags::Interact_Revive;
	InteractOptions.Add(InteractOption);
}

UAbilitySystemComponent* ADC1Player::GetAbilitySystemComponent() const
{
	const ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();

	// 탑승 중엔 PlayerState 가 탄 팰에 있다
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

#pragma region Interact Interface
UPrimitiveComponent* ADC1Player::GetInteractCollider() const
{
	return InteractCollider;
}
void ADC1Player::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	OutOptions = InteractOptions;
}
#pragma endregion

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

void ADC1Player::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 사망 화면 카운트다운용 - 본인만
	DOREPLIFETIME_CONDITION(ADC1Player, RespawnAvailableTime, COND_OwnerOnly);
	DOREPLIFETIME(ADC1Player, RiddenPawn);
}

void ADC1Player::BeginPlay()
{
	Super::BeginPlay();

	FixedSpringArmHeight = SpringArm->GetRelativeLocation().Z;
	CurrentSpringArmHeight = FixedSpringArmHeight;
}

void ADC1Player::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, CurrentTargetArmLength, DeltaTime, ZoomInSpeed);

	FVector SpringArmLocation = SpringArm->GetRelativeLocation();
	SpringArmLocation.Z = FMath::FInterpTo(SpringArmLocation.Z, CurrentSpringArmHeight, DeltaTime, ZoomInSpeed);
	SpringArm->SetRelativeLocation(SpringArmLocation);
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

FRotator ADC1Player::GetViewRotation() const
{
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

	if (true == IsLocallyControlled())
	{
		IsLocalPlayerCharacter = true;
	}

	// 탄 팰이 있는데 조종이 풀리면 탑승
	const bool IsRiding = (nullptr != GetRiddenPawn() && nullptr == GetController());

	// 탑승 중에도 게이지 보임. 값은 VM 이 탄 팰의 스태미나
	StaminaWidgetComponent->SetVisibility(true == IsLocallyControlled() || (true == IsLocalPlayerCharacter && true == IsRiding));

	CurrentSpringArmHeight = (true == IsRiding) ? RideSpringArmHeight : FixedSpringArmHeight;
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
	if (false == IsValid(PS))
		return;

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
		UIManager->PopAllScreen();
	}
}

void ADC1Player::OnDeath()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();

	if (true == HasAuthority())
	{
		if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			CancelAbility(DC1PlayerTags::Ability_Player_Downed);
		}

		// 부활 대기 시간 시작점 설정
		RespawnAvailableTime = GetWorld()->GetTimeSeconds() + DeathScreenDelay + RespawnDelay;

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

	// 본인 클라만 Timer 설정
	if (false == IsValid(PS))
		return;
	ADC1PlayerController* PC = Cast<ADC1PlayerController>(PS->GetPlayerController());
	if (true == ::IsValid(PC) && true == PC->IsLocalController())
	{
		GetWorldTimerManager().SetTimer(DeathScreenTimerHandle, this, &ThisClass::ShowDeathScreen, DeathScreenDelay, false);
		if (UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>())
		{
			UIManager->SetHUDVisibility(false);
		}
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

void ADC1Player::Change_EquipMesh(USkeletalMesh* InMesh, EDC1EquipSlotType InSlotType)
{
	switch (InSlotType)
	{
	case EDC1EquipSlotType::Body:
		{
			USkeletalMeshComponent* BodyMesh = GetMesh();
			ADC1PlayerState * PS = GetPlayerState<ADC1PlayerState>();
			if (false == IsValid(PS) || false == IsValid(BodyMesh))
				return;

			BodyMesh->EmptyOverrideMaterials();
			if (nullptr == InMesh)
			{
				BodyMesh->SetSkeletalMesh(DefaultBodyMesh, false);
			}
			else
			{
				BodyMesh->SetSkeletalMesh(InMesh, false);
			}
			// 색상 재적용
			UDC1AppearanceComponent* AppearanceComponent = PS->GetAppearanceComponent();
			if (false == IsValid(AppearanceComponent))
				return;

			AppearanceComponent->ApplyBodyColor(BodyMesh);
			break;
		}
	case EDC1EquipSlotType::Head:
		{
			if (false == IsValid(HairEquipMesh))
				return;
			HairEquipMesh->SetSkeletalMesh(InMesh, false);

			// 투구를 쓰면 머리카락이 뚫고 나오지 않게 숨기고, 벗으면 다시 보인다
			if (true == IsValid(HairMesh))
			{
				HairMesh->SetVisibility(nullptr == InMesh);
			}
			break;
		}
	}
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

	// Downed Gauge Delegate Bind
	BindDownedGaugeWatcher();

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
	RefreshIncapacitated();

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

	if(true == DeathScreenTimerHandle.IsValid())
		GetWorldTimerManager().ClearTimer(DeathScreenTimerHandle);
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

void ADC1Player::BindDownedGaugeWatcher()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (nullptr == ASC)
		return;

	FOnGameplayAttributeValueChange& Delegate =
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetDownedGaugeAttribute());

	// ASC가 바뀌어 다시 불려도 중복 등록되지 않게
	Delegate.RemoveAll(this);
	Delegate.AddUObject(this, &ThisClass::HandleDownedGaugeChanged);
}

void ADC1Player::HandleDownedGaugeChanged(const FOnAttributeChangeData& Data)
{
	// [Debug] 기절 게이지 확인 ( UI 대신 )
	LOG(TEXT("[Downed] %s ( %s ) Gauge : %.1f -> %.1f"),
		*GetName(), true == HasAuthority() ? TEXT("Server") : (true == IsLocallyControlled() ? TEXT("Owner") : TEXT("Other")),
		Data.OldValue, Data.NewValue);

	// 게이지가 남아 있다가 0이 된 순간 = 기절 중 사망
	// 기절 태그로 판단하지 않는다 ( 클라에선 게이지 0과 태그 제거의 도착 순서가 보장되지 않음 )
	if (Data.OldValue > 0.f && Data.NewValue <= 0.f)
	{
		Die();
	}
}

bool ADC1Player::CanBeRevived() const
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		return true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Downed) &&
			false == ASC->HasMatchingGameplayTag(DC1StateTags::State_BeingRevived);
	}

	return false;
}

void ADC1Player::StartBeingRevived()
{
	if (false == HasAuthority())
		return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_BeingRevived, 1, EGameplayTagReplicationState::TagOnly);
	}
}

void ADC1Player::StopBeingRevived()
{
	if (false == HasAuthority())
		return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->SetLooseGameplayTagCount(DC1StateTags::State_BeingRevived, 0, EGameplayTagReplicationState::TagOnly);
	}
}

void ADC1Player::CompleteRevived()
{
	if (false == HasAuthority())
		return;

	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		const float MaxHealth = ASC->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
		ASC->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), MaxHealth * 0.1f);
	}

	CancelAbility(DC1PlayerTags::Ability_Player_Downed);
}

void ADC1Player::ShowDeathScreen()
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}
	UIManager->PushScreen(DC1UITags::UI_Main_Death);
}

void ADC1Player::OnDownStateChanged(const FGameplayTag Tag, int32 NewCount)
{
	IsDown = NewCount > 0;

	if (true == IsValid(InteractCollider))
	{
		InteractCollider->SetCollisionEnabled(IsDown ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	}

	OnInteractOptionsChanged.Broadcast();

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

	// 본인 클라만 UI 처리. 탑승 중엔 팰을 조종하고 있어 IsLocallyControlled가 false라 PC로 판단한다
	ADC1PlayerController* PC = Cast<ADC1PlayerController>(PS->GetPlayerController());
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
		return;

	const bool bWasIncapacitated = IsIncapacitated;
	IsIncapacitated = IsDownOrDead();
	
	if (true == IsIncapacitated && false == bWasIncapacitated)
	{
		// 빌드 취소가 먼저 -> SetBuildMode(false)가 SetInteractEnabled(true) 호출
		UDC1BuildPlacementComponent* BuildComp = PC->GetBuildPlacementComponent();
		if (true == ::IsValid(BuildComp))
		{
			BuildComp->CancelBuildMode();
		}

		// Down or Dead -> UI Pop -> 기절 UI
		UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
		if (false == ::IsValid(UIManager))
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
			return;
		}
		UIManager->PopAllScreen();
	}

	UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
	if (true == ::IsValid(InteractComp))
	{
		InteractComp->SetInteractEnabled(false == IsIncapacitated);

	}

	PC->SetGameplayBaseIMC(IsIncapacitated ? DC1IMCTags::IMC_Downed : DC1IMCTags::IMC_Default);
}

void ADC1Player::RegisterStateHandle(const FGameplayTag& Tag, void (ADC1Player::*InFunc)(FGameplayTag, int32), UDC1AbilitySystemComponent* ASC)
{
	if (nullptr == ASC || nullptr != PlayerStateTagHandles.Find(Tag))
		return;

	PlayerStateTagHandles.Add(Tag, 
		ASC->RegisterAndCallGameplayTagEvent(Tag,
		FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(this, InFunc)));
}
