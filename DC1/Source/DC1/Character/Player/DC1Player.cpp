// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Player/DC1Player.h"

#include "Define/DC1Macro.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"

#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1CharacterMovementComponent.h"
#include "Component/DC1EquipComponent.h"
#include "Component/Appearance/DC1AppearanceComponent.h"

#include "AbilitySystemBlueprintLibrary.h"

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

	// Appearance Component
	AppearanceComponent = CreateDefaultSubobject<UDC1AppearanceComponent>("AppearanceComponent");
}

UAbilitySystemComponent* ADC1Player::GetAbilitySystemComponent() const
{
	if (ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>())
	{
		return PS->GetAbilitySystemComponent();
	}
	return nullptr;
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

	AppearanceComponent->SetParts(GetMesh(), HairMesh, HeadMesh);
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
	InitializeAbilitySystem();
	InitializeEquip();
}

void ADC1Player::UnPossessed()
{
	Clear_Resource();
	Super::UnPossessed();
}

void ADC1Player::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitializeAbilitySystem();
	InitializeEquip();
}

void ADC1Player::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	StaminaWidgetComponent->SetVisibility(IsLocallyControlled());
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

void ADC1Player::InitializeAbilitySystem()
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS))
	{
		return;
	}
	UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
	ASC->InitAbilitySystem(PS, this, DC1AssetTags::Asset_Ability_Player);

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
}

void ADC1Player::Clear_Resource()
{
	if (ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>())
	{
		UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent());
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

void ADC1Player::RegisterStateHandle(const FGameplayTag& Tag, void (ADC1Player::*InFunc)(FGameplayTag, int32), UDC1AbilitySystemComponent* ASC)
{
	if (nullptr == ASC || nullptr != PlayerStateTagHandles.Find(Tag))
		return;

	PlayerStateTagHandles.Add(Tag, 
		ASC->RegisterAndCallGameplayTagEvent(Tag,
		FOnGameplayEffectTagCountChanged::FDelegate::CreateUObject(this, InFunc)));
}
