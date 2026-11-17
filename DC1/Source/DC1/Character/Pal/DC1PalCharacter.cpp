// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Pal/DC1PalCharacter.h"
#include "Controller/DC1PalAIController.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1CharacterAttributeSet.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_PalPassive.h"
#include "Components/CapsuleComponent.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "GameplayEffectExtension.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Component/Pal/DC1PalRideComponent.h"
#include "Component/Pal/DC1PalMovementComponent.h"
#include "Component/Notification/DC1NotificationComponent.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"

#include "UI/World/DC1PalHealthWidget.h"
#include "Item/Pickup/DC1PickupItem.h"

#include "Character/Player/DC1PlayerController.h"
#include "Data/DA/Input/DC1InputData.h"
#include "Data/DA/Ability/DC1AbilityData.h"

#include "Camera/CameraTypes.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Perception/AIPerceptionSystem.h"

#include "Subsystem/DC1ExperienceSubsystem.h"
#include "Tags/DC1GamePlayTags.h"

namespace DC1PalCharacter
{
	// 드랍 방향을 줄 수만큼 나눈 칸 안에서 흔드는 폭(도). 두 줄이 같은 쪽으로 겹쳐 날아가지 않게
	constexpr float DropAngleJitter = 20.f;

	// 시체 중심에서 이만큼 위에서 튀어나온다. 래그돌 안에서 시작하지 않게
	constexpr float DropSpawnHeight = 30.f;

	// 가해자의 플레이어 (파트너면 주인). 야생 팰이면 nullptr
	static ADC1PlayerState* FindPlayerStateOf(const AActor* InCauser)
	{
		const APawn* Pawn = Cast<APawn>(InCauser);

		if (const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(Pawn))
		{
			if (EDC1Team::Player != Pal->GetTeam())
			{
				return nullptr;
			}

			// 내 팰이면 주인. 탑승 중이든 아니든 같다
			return Pal->GetOwnerPlayerState();
		}

		return (nullptr != Pawn) ? Pawn->GetPlayerState<ADC1PlayerState>() : nullptr;
	}

	// 가해자 파티 (파트너면 주인)
	static UDC1PalPartyComponent* FindPartyOf(const AActor* InCauser)
	{
		const ADC1PlayerState* PlayerState = FindPlayerStateOf(InCauser);
		return (nullptr != PlayerState) ? PlayerState->GetPalPartyComponent() : nullptr;
	}

	// IMC 는 로컬 전용이라 탄 사람 클라에서만
	static void SetRideIMCEnabled(AController* InController, bool InEnabled)
	{
		ADC1PlayerController* PC = Cast<ADC1PlayerController>(InController);
		if (nullptr != PC && true == PC->IsLocalController())
		{
			PC->SetIMCEnabled(DC1PalTags::IMC_Ride, InEnabled);
		}
	}
}

ADC1PalCharacter::ADC1PalCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UDC1PalMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	PalAttributeSet = CreateDefaultSubobject<UDC1PalAttributeSet>(TEXT("PalAttributeSet"));

	RideComponent = CreateDefaultSubobject<UDC1PalRideComponent>(TEXT("RideComponent"));

	// 피격 판정.
	HitboxComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Hitbox"));
	HitboxComponent->SetupAttachment(RootComponent);
	HitboxComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	HitboxComponent->SetCanEverAffectNavigation(false);
	HitboxComponent->SetCollisionProfileName(TEXT("EnemyPalHitbox"));

	// 머리 위 HP바. 위젯 클래스와 높이는 BP_Pal_* 에서 종마다 지정한다
	HealthWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthWidget"));
	HealthWidgetComponent->SetupAttachment(RootComponent);
	HealthWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);	// 거리와 무관하게 크기 고정
	HealthWidgetComponent->SetDrawAtDesiredSize(true);
	HealthWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HealthWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthWidgetComponent->SetTickWhenOffscreen(false);			// 화면 밖이면 돌지 않는다

	// 회전쓰지 않음
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	UCharacterMovementComponent* Movement = GetCharacterMovement();
	SetOrientToMovement(false); // 기본은 AIController의 focus가 회전을 구동
	Movement->RotationRate = FRotator(0.f, 360.f, 0.f);
	Movement->MaxWalkSpeed = 400.f;
	Movement->bUseRVOAvoidance = true;

	Movement->GetNavMovementProperties()->bUseAccelerationForPaths = true;
}

void ADC1PalCharacter::GetActorEyesViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	OutLocation = GetPawnViewLocation();
	OutRotation = GetActorRotation();
}

void ADC1PalCharacter::CalcCamera(float DeltaTime, FMinimalViewInfo& OutResult)
{
	if (APawn* Rider = RideComponent->GetRider())
	{
		Rider->CalcCamera(DeltaTime, OutResult);
		return;
	}

	Super::CalcCamera(DeltaTime, OutResult);
}

void ADC1PalCharacter::SetOrientToMovement(bool Enable)
{
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = Enable;
		Movement->bUseControllerDesiredRotation = (false == Enable);
	}
}

void ADC1PalCharacter::PostInitializeComponents()
{
	ApplyPalData(); //PalData 먼저로드
	Super::PostInitializeComponents();
}

void ADC1PalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 서버에선 OnRep_Team 이 안 온다
	ApplyTeamCollisionProfile();

	// 경험치. 거점 팰만 받지만 거점 배치 시점과 무관하게 걸어두고 핸들러에서 거른다
	if (true == HasAuthority() && IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->AddGameplayEventTagContainerDelegate(FGameplayTagContainer(DC1PlayerTags::Event_Exp),
			FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::HandleExpEvent));
	}

	if (false == IsValid(HealthWidgetComponent))
	{
		return;
	}

	// 위젯은 아직 안 만들어져 있을 수 있다. 데디케이티드 서버에서는 만들어지지 않는다
	HealthWidgetComponent->InitWidget();

	// 위젯이 자기 주인을 모르므로 팰이 직접 넣어준다.
	// GetOwningPlayerPawn 은 로컬 플레이어의 폰을 돌려주므로 쓸 수 없다
	if (UDC1PalHealthWidget* HealthWidget = Cast<UDC1PalHealthWidget>(HealthWidgetComponent->GetUserWidgetObject()))
	{
		HealthWidget->InitForPal(this);
	}
	else
	{
		return;
	}

	HealthWidgetComponent->SetVisibility(false);

	GetWorldTimerManager().SetTimer(HealthWidgetVisibilityTimer, this,
		&ThisClass::UpdateHealthWidgetVisibility, HealthWidgetCheckInterval, true);
}

void ADC1PalCharacter::NotifyControllerChanged()
{
	// Super 가 PreviousController 를 갱신한다
	AController* OldController = PreviousController;

	Super::NotifyControllerChanged();

	DC1PalCharacter::SetRideIMCEnabled(OldController, false);
	DC1PalCharacter::SetRideIMCEnabled(GetController(), true);

	// 서버·클라 설정이 같아야 예측이 맞는다
	const bool IsRidden = (nullptr != Cast<APlayerController>(GetController()));

	SetOrientToMovement(IsRidden);

	// 탑승 중 평소 속도도 달리기 속도. 질주는 키를 누르는 동안만(OnRideSprint)
	UCharacterMovementComponent* Movement = GetCharacterMovement();
	if (nullptr != Movement && nullptr != PalData)
	{
		Movement->MaxWalkSpeed = PalData->RunSpeed;
	}

	// 타고 내릴 때 질주가 남지 않게 (어빌리티가 끝나며 CMC 플래그도 끈다)
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAbilityByTag(DC1PalTags::Ability_Pal_RideSprint);
	}

	if (UDC1PalMovementComponent* PalMovement = Cast<UDC1PalMovementComponent>(Movement))
	{
		PalMovement->SetRideSprint(false);
	}
}

void ADC1PalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData);
	if (nullptr == EnhancedInputComponent || nullptr == InputData)
	{
		return;
	}

	if (const UInputAction* MoveAction = InputData->FindInputActionbyTag(DC1PalTags::Input_Ride_Move))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::OnRideMove);
	}

	if (const UInputAction* JumpAction = InputData->FindInputActionbyTag(DC1PalTags::Input_Ride_Jump))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::OnRideJumpPressed);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::OnRideJumpReleased);
	}

	if (const UInputAction* SprintAction = InputData->FindInputActionbyTag(DC1PalTags::Input_Ride_Sprint))
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::OnRideSprintPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::OnRideSprintReleased);
	}

	const FGameplayTag SkillInputTags[] = { DC1PalTags::Input_Ride_Skill1, DC1PalTags::Input_Ride_Skill2, DC1PalTags::Input_Ride_Skill3 };
	for (int32 Index = 0; Index < UE_ARRAY_COUNT(SkillInputTags); ++Index)
	{
		if (const UInputAction* SkillAction = InputData->FindInputActionbyTag(SkillInputTags[Index]))
		{
			EnhancedInputComponent->BindAction(SkillAction, ETriggerEvent::Started, this, &ThisClass::OnRideSkillPressed, Index);
		}
	}
}

void ADC1PalCharacter::GrantSkillAbilities()
{
	if (false == HasAuthority() || false == IsValid(AbilitySystemComponent) || nullptr == PalData)
	{
		return;
	}

	const UDC1AbilityData* AbilityData = UDC1AssetManager::GetAssetByTag<UDC1AbilityData>(PalData->AbilityDataTag);
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (nullptr == AbilityData || nullptr == Registry)
	{
		return;
	}

	for (const FGameplayTag& SkillTag : PalData->SkillTags)
	{
		if (const FDC1PalSkillDef* Def = Registry->FindSkillDef(SkillTag))
		{
			AbilityData->GiveVariant(AbilitySystemComponent, Def->ActivationTag, SkillTag, this);
		}
	}
}

void ADC1PalCharacter::OnRideSkillPressed(const FInputActionValue& Value, int32 InSkillIndex)
{
	// 카메라 방향으로 짧게 돈 뒤 서버에 발동을 요청
	RideComponent->RequestRideSkill(InSkillIndex);
}

void ADC1PalCharacter::OnRideMove(const FInputActionValue& Value)
{
	if (true == RideComponent->IsRideActionLocked())
	{
		return;
	}

	const FVector2D MoveVector = Value.Get<FVector2D>();
	const FRotationMatrix YawMatrix(FRotator(0.f, GetControlRotation().Yaw, 0.f));

	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::X), MoveVector.X);
	AddMovementInput(YawMatrix.GetUnitAxis(EAxis::Y), MoveVector.Y);
}

void ADC1PalCharacter::OnRideJumpPressed(const FInputActionValue& Value)
{
	if (true == RideComponent->IsRideActionLocked())
	{
		return;
	}

	Jump();
}

void ADC1PalCharacter::OnRideJumpReleased(const FInputActionValue& Value)
{
	StopJumping();
}

void ADC1PalCharacter::OnRideSprintPressed(const FInputActionValue& Value)
{
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->ActivateAbilityByTag(DC1PalTags::Ability_Pal_RideSprint);
	}
}

void ADC1PalCharacter::OnRideSprintReleased(const FInputActionValue& Value)
{
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAbilityByTag(DC1PalTags::Ability_Pal_RideSprint);
	}
}

void ADC1PalCharacter::UpdateHealthWidgetVisibility()
{
	if (false == IsValid(HealthWidgetComponent))
	{
		return;
	}

	// 알파(보스)는 머리 위 대신 화면 위 보스 HP바 ( UDC1PalBossViewModel )
	if (true == bIsAlpha)
	{
		HealthWidgetComponent->SetVisibility(false);
		return;
	}

	bool ShouldShow = (GetHealthRatio() < 1.f);

	if (false == ShouldShow)
	{
		if (const APawn* LocalPawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			const float DistanceSq = FVector::DistSquared(GetActorLocation(), LocalPawn->GetActorLocation());
			ShouldShow = (DistanceSq <= (HealthWidgetVisibleDistance * HealthWidgetVisibleDistance));
		}
	}

	if (HealthWidgetComponent->IsVisible() != ShouldShow)
	{
		HealthWidgetComponent->SetVisibility(ShouldShow);
	}
}

void ADC1PalCharacter::ApplyPalData()
{
	if (false == SpeciesTag.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: SpeciesTag is not set."), *GetName());
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: Can't find PalRegistryData. Check PDA_AssetData."), *GetName());
		return;
	}

	// 나중에 스폰 전에 비동기로 미리 올려두는 쪽으로 바꾸기.
	PalData = Registry->FindPalDataByTag(SpeciesTag).LoadSynchronous();
	if (false == IsValid(PalData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] %s: Failed to load PalData [%s]."), *GetName(), *SpeciesTag.ToString());
		return;
	}

	// Locomotion
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->MaxWalkSpeed = PalData->RunSpeed;
		Movement->RotationRate = FRotator(0.f, PalData->RotationRateYaw, 0.f);
	}

	if (UDC1PalMovementComponent* PalMovement = Cast<UDC1PalMovementComponent>(GetCharacterMovement()))
	{
		PalMovement->SetRideSprintSpeed(PalData->RideSprintSpeed);
	}

	// 맵에 놓은 알파는 액터 값이 원본
	PalInstance.bIsAlpha = bIsAlpha;
	ApplyAlphaScale();

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->InitAbilitySystem(this, this, PalData->AbilityDataTag);
		GrantSkillAbilities();
		BindHealthWatcher();

		// Health는 PostGameplayEffectExecute 가 SetHealth 로 바꾸므로 Damage 를 구독
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetDamageAttribute()).AddUObject(this, &ThisClass::HandleDamaged);

		// HP바용. BindHealthWatcher 가 RemoveAll(this) 를 하므로 반드시 그 뒤에 온다
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChangedForUI);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1PalAttributeSet::GetSanityAttribute()).AddUObject(this, &ThisClass::HandleSanityChangedForUI);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1CharacterAttributeSet::GetLevelAttribute()).AddUObject(this, &ThisClass::HandleLevelChangedForUI);
	}

	if (IsValid(PalAttributeSet))
	{
		const int32 FinalLevel = (true == bOverrideLevel)
			? LevelOverride
			: FMath::RandRange(PalData->MinLevel, FMath::Max(PalData->MinLevel, PalData->MaxLevel));

		PalAttributeSet->InitLevel(FinalLevel);
		PalInstance.Level = FinalLevel;

		if (PalInstance.TalentHealth < 0)
		{
			PalInstance.TalentHealth = FMath::RandRange(0, 100);
		}

		if (PalInstance.TalentAttack < 0)
		{
			PalInstance.TalentAttack = FMath::RandRange(0, 100);
		}

		if (PalInstance.TalentDefense < 0)
		{
			PalInstance.TalentDefense = FMath::RandRange(0, 100);
		}

		const float FinalMaxHealth = Registry->GetMaxHealthFor(*PalData, FinalLevel, PalInstance.TalentHealth, PalInstance.bIsAlpha);
		const float FinalAttack = Registry->GetAttackPowerFor(*PalData, FinalLevel, PalInstance.TalentAttack);
		const float FinalDefense = Registry->GetDefensePowerFor(*PalData, FinalLevel, PalInstance.TalentDefense);

		PalAttributeSet->InitMaxHealth(FinalMaxHealth);
		PalAttributeSet->InitHealth(FinalMaxHealth);
		PalAttributeSet->InitAttackPower(FinalAttack);
		PalAttributeSet->InitDefensePower(FinalDefense);
		PalAttributeSet->InitWorkSpeed(PalData->WorkSpeedStat);
		PalAttributeSet->InitMaxStamina(PalData->Stamina);
		PalAttributeSet->InitStamina(PalData->Stamina);
	}

	if (EDC1PalGender::None == PalInstance.Gender)
	{
		PalInstance.Gender = FMath::RandBool() ? EDC1PalGender::Male : EDC1PalGender::Female;
	}

	if (false == PalInstance.bPassivesRolled)
	{
		Registry->RollPassives(PalInstance.PassiveTags);
		PalInstance.bPassivesRolled = true;
	}

	ApplyPassives(*Registry);
}

void ADC1PalCharacter::ApplyPassives(const UDC1PalRegistryData& InRegistry)
{
	if (false == IsValid(AbilitySystemComponent))
	{
		return;
	}

	for (const FGameplayTag& PassiveTag : PalInstance.PassiveTags)
	{
		const FDC1PalPassiveDef* PassiveDef = InRegistry.FindPassiveDef(PassiveTag);
		if (nullptr == PassiveDef)
		{
			continue;
		}

		FGameplayEffectSpecHandle Spec = AbilitySystemComponent->MakeOutgoingSpec(
			UDC1GameplayEffect_PalPassive::StaticClass(), 1.f, AbilitySystemComponent->MakeEffectContext());
		if (false == Spec.IsValid())
		{
			continue;
		}

		Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_PalPassive_Attack, PassiveDef->AttackMultiplier);
		Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_PalPassive_Defense, PassiveDef->DefenseMultiplier);
		Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_PalPassive_WorkSpeed, PassiveDef->WorkSpeedMultiplier);

		AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	}
}

void ADC1PalCharacter::HandleDamaged(const FOnAttributeChangeData& Data)
{
	// Damage 메타 어트리뷰트가 채워지는 순간만 본다. 곧바로 0으로 되돌리는 변경은 무시
	if (false == HasAuthority() || Data.NewValue <= 0.f || nullptr == Data.GEModData)
	{
		return;
	}

	AActor* Causer = Data.GEModData->EffectSpec.GetContext().GetEffectCauser();
	LastDamageCauser = Causer;

	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		// 타러 달려오는 중에 피격
		if (true == PalController->IsRideApproaching())
		{
			if (UDC1PalPartyComponent* Party = DC1PalCharacter::FindPartyOf(this))
			{
				Party->StopRideApproach(true);
			}
		}

		PalController->ProvokeBy(Causer);
	}

	// 플레이어가 때린 야생 팰이면 그 플레이어의 파트너도 같이 싸운다. 파트너가 때린 건 파트너 자신의 도발로 처리된다
	if (EDC1Team::Wild == GetTeam() && nullptr == Cast<ADC1PalCharacter>(Causer))
	{
		const UDC1PalPartyComponent* CauserParty = DC1PalCharacter::FindPartyOf(Causer);
		const ADC1PalCharacter* Partner = (nullptr != CauserParty) ? CauserParty->GetDeployedPal() : nullptr;
		if (ADC1PalAIController* PartnerController = IsValid(Partner) ? Cast<ADC1PalAIController>(Partner->GetController()) : nullptr)
		{
			PartnerController->NotifyOwnerHit(this);
		}
	}

	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->ActivateAbilityByTag(DC1PalTags::Ability_Pal_Hit);
	}

	if (nullptr != HitSound)
	{
		Multicast_PlayHitSound(GetActorLocation());
	}
}

void ADC1PalCharacter::Multicast_PlayHitSound_Implementation(FVector_NetQuantize InLocation)
{
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, InLocation);
}

void ADC1PalCharacter::HandleHealthChangedForUI(const FOnAttributeChangeData& Data)
{
	if (false == IsValid(PalAttributeSet))
	{
		return;
	}

	UpdateHealthWidgetVisibility();

	OnPalHealthChanged.Broadcast(Data.NewValue, PalAttributeSet->GetMaxHealth());
}

void ADC1PalCharacter::HandleSanityChangedForUI(const FOnAttributeChangeData& Data)
{
	OnPalSanityChanged.Broadcast(Data.NewValue, GetMaxSanity());
}

float ADC1PalCharacter::GetSanity() const
{
	return IsValid(PalAttributeSet) ? PalAttributeSet->GetSanity() : 0.f;
}

float ADC1PalCharacter::GetMaxSanity() const
{
	return IsValid(PalAttributeSet) ? PalAttributeSet->GetMaxSanity() : 0.f;
}

void ADC1PalCharacter::HandleLevelChangedForUI(const FOnAttributeChangeData& Data)
{
	if (false == IsValid(HealthWidgetComponent))
	{
		return;
	}

	if (UDC1PalHealthWidget* HealthWidget = Cast<UDC1PalHealthWidget>(HealthWidgetComponent->GetUserWidgetObject()))
	{
		HealthWidget->InitForPal(this);
	}
}

void ADC1PalCharacter::HandleExpEvent(FGameplayTag InEventTag, const FGameplayEventData* InPayload)
{
	if (nullptr == InPayload || nullptr == GetBasePalBox())
	{
		return;
	}

	GainExperience(FMath::RoundToInt(InPayload->EventMagnitude));
}

void ADC1PalCharacter::GainExperience(int32 InAmount)
{
	if (false == HasAuthority() || false == IsValid(PalAttributeSet) || false == IsValid(AbilitySystemComponent) || nullptr == PalData)
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		return;
	}

	if (0 == Registry->ApplyExperience(PalInstance, InAmount))
	{
		return;
	}

	const int32 NewLevel = PalInstance.Level;
	const float NewMaxHealth = Registry->GetMaxHealthFor(*PalData, NewLevel, PalInstance.TalentHealth, PalInstance.bIsAlpha);

	// Base만 갱신 (패시브 배율 유지)
	AbilitySystemComponent->SetNumericAttributeBase(UDC1CharacterAttributeSet::GetLevelAttribute(), static_cast<float>(NewLevel));
	AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetMaxHealthAttribute(), NewMaxHealth);
	AbilitySystemComponent->SetNumericAttributeBase(UDC1CharacterAttributeSet::GetAttackPowerAttribute(), Registry->GetAttackPowerFor(*PalData, NewLevel, PalInstance.TalentAttack));
	AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetDefensePowerAttribute(), Registry->GetDefensePowerFor(*PalData, NewLevel, PalInstance.TalentDefense));

	// 사망 제외
	if (PalAttributeSet->GetHealth() > 0.f)
	{
		AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), NewMaxHealth);
	}
}

void ADC1PalCharacter::StopAndUnregister()
{
	if (IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->CancelAbilities();
	}

	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->StopBehavior();
	}

	// 남들이 계속 감지해 쫓지 않도록
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->UnregisterSource(*this);
	}
}

int32 ADC1PalCharacter::GetLevel() const
{
	if (false == IsValid(PalAttributeSet))
	{
		return 1;
	}

	return FMath::Max(1, FMath::RoundToInt(PalAttributeSet->GetLevel()));
}

float ADC1PalCharacter::GetHealthRatio() const
{
	if (false == IsValid(PalAttributeSet))
	{
		return 0.f;
	}

	const float Max = PalAttributeSet->GetMaxHealth();
	if (Max <= 0.f)
	{
		return 0.f;
	}

	return FMath::Clamp(PalAttributeSet->GetHealth() / Max, 0.f, 1.f);
}

float ADC1PalCharacter::GetCaptureRate() const
{
	if (nullptr == PalData)
	{
		return 0.f;
	}

	return (true == bIsAlpha) ? (PalData->CaptureRate * PalData->AlphaCaptureRate) : PalData->CaptureRate;
}

EDC1PalElement ADC1PalCharacter::GetElement() const
{
	return (nullptr != PalData) ? PalData->Element : EDC1PalElement::Neutral;
}

EDC1PalElement ADC1PalCharacter::GetElement2() const
{
	return (nullptr != PalData) ? PalData->Element2 : EDC1PalElement::Neutral;
}

FDC1PalInstance ADC1PalCharacter::Fold()
{
	FDC1PalInstance Instance = PalInstance;

	Instance.SpeciesTag = SpeciesTag;
	Instance.PalID = InstanceID.IsValid() ? InstanceID : FGuid::NewGuid();

	if (IsValid(PalAttributeSet))
	{
		Instance.CurrentHealth = PalAttributeSet->GetHealth();
		Instance.CurrentSanity = PalAttributeSet->GetSanity();
		Instance.Level = GetLevel();
	}

	StopAndUnregister();
	Destroy();

	return Instance;
}

void ADC1PalCharacter::OnCaptureFailed(AActor* InThrower)
{
	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->ProvokeBy(InThrower);
	}
}

void ADC1PalCharacter::BeginCapture()
{
	// 파괴하지 않음. 실패 시 되돌려야 함
	StopAndUnregister();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void ADC1PalCharacter::CancelCapture()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->SetMovementMode(MOVE_Walking);
	}

	// StopAndUnregister가 뺀 지각 복구
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->RegisterSource(*this);
	}

	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->TryStartBehavior();
	}
}

ADC1PalCharacter* ADC1PalCharacter::SpawnFromInstanceData(UWorld* InWorld, const FDC1PalInstance& InInstance, const FTransform& InTransform, ADC1PlayerState* InOwner, ADC1PalBoxActor* InBasePalBox)
{
	if (nullptr == InWorld || false == InInstance.IsValidPal())
	{
		return nullptr;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == IsValid(Registry))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Spawn: Can't find PalRegistryData. Check PDA_AssetData."));
		return nullptr;
	}

	const UDC1PalData* SpeciesData = Registry->FindPalDataByTag(InInstance.SpeciesTag).LoadSynchronous();
	if (false == IsValid(SpeciesData))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Spawn: Failed to load PalData [%s]."), *InInstance.SpeciesTag.ToString());
		return nullptr;
	}

	UClass* PalClass = SpeciesData->PalClass.LoadSynchronous();
	if (nullptr == PalClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Spawn: No PalClass for [%s]. Check DA_Pal_*."), *InInstance.SpeciesTag.ToString());
		return nullptr;
	}

	ADC1PalCharacter* Pal = InWorld->SpawnActorDeferred<ADC1PalCharacter>(PalClass, InTransform, nullptr, nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	
	if (false == IsValid(Pal))
	{
		return nullptr;
	}

	Pal->SpeciesTag = InInstance.SpeciesTag;
	Pal->InstanceID = InInstance.PalID;
	Pal->Team = EDC1Team::Player;
	Pal->OwnerPlayerState = InOwner;
	Pal->bIsAlpha = InInstance.bIsAlpha;

	// OnPossess 가 이걸 보고 트리를 고르므로 FinishSpawning 전에
	Pal->BasePalBox = InBasePalBox;

	// 원본을 통째로 보관
	Pal->PalInstance = InInstance;
	Pal->bOverrideLevel = true;
	Pal->LevelOverride = FMath::Max(1, InInstance.Level);

	Pal->FinishSpawning(InTransform);

	if (IsValid(Pal->PalAttributeSet))
	{
		Pal->PalAttributeSet->SetHealth(InInstance.CurrentHealth);

		if (0.f <= InInstance.CurrentSanity)
		{
			Pal->PalAttributeSet->SetSanity(InInstance.CurrentSanity);
		}
	}

	return Pal;
}

void ADC1PalCharacter::OnDeath()
{
	StopAndUnregister();

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (IsValid(HitboxComponent))
	{
		HitboxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));

		MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

		MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComp->SetSimulatePhysics(true);

		MeshComp->SetAllPhysicsLinearVelocity(FVector::ZeroVector);
	}

	// 처치 경험치, 쓰러뜨림 알림, 드랍
	if (true == HasAuthority() && EDC1Team::Wild == Team)
	{
		AActor* Killer = LastDamageCauser.Get();

		// 경험치 전송
		if (UDC1ExperienceSubsystem* ExpSubsystem = GetWorld()->GetSubsystem<UDC1ExperienceSubsystem>())
		{
			const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
			const int32 Amount = ::IsValid(Registry) ? Registry->GetDropExpFor(PalData, GetLevel(), bIsAlpha) : 0;
			ExpSubsystem->Grant(Killer, DC1PlayerTags::Event_Exp_Shared_Kill, Amount);
		}

		// 쓰러뜨린 사람 화면에만. 팰이 쓰러뜨렸으면 그 팰이 주어
		const ADC1PlayerState* KillerPlayerState = DC1PalCharacter::FindPlayerStateOf(Killer);
		UDC1NotificationComponent* NotificationComponent = (nullptr != KillerPlayerState) ? KillerPlayerState->GetNotificationComponent() : nullptr;
		if (nullptr != NotificationComponent)
		{
			const ADC1PalCharacter* KillerPal = Cast<ADC1PalCharacter>(Killer);
			const FGameplayTag KillerSpecies = (nullptr != KillerPal) ? KillerPal->SpeciesTag : FGameplayTag();
			const FString KillerNickname = (nullptr != KillerPal) ? KillerPal->PalInstance.Nickname : FString();

			NotificationComponent->Client_NotifyPalDefeated(KillerSpecies, KillerNickname, SpeciesTag);
		}

		// 드랍아이템 소환
		SpawnDrops();
	}

	OnPalDied.Broadcast();
}

void ADC1PalCharacter::SpawnDrops()
{
	if (nullptr == PalData)
	{
		return;
	}

	const int32 DropNum = PalData->Drops.Num();
	if (0 == DropNum)
	{
		return;
	}

	const FVector Origin = GetActorLocation() + FVector::UpVector * DC1PalCharacter::DropSpawnHeight;

	const UCapsuleComponent* Capsule = GetCapsuleComponent();
	const float BodyRadius = (nullptr != Capsule) ? Capsule->GetScaledCapsuleRadius() : 0.f;

	const float AngleStep = 360.f / DropNum;
	const float StartAngle = FMath::FRandRange(0.f, 360.f);

	for (int32 Index = 0; Index < DropNum; ++Index)
	{
		const FDC1PalDropDef& Drop = PalData->Drops[Index];

		if (FMath::FRand() >= Drop.Chance)
		{
			continue;
		}

		const float Yaw = StartAngle + AngleStep * Index + FMath::FRandRange(-DC1PalCharacter::DropAngleJitter, DC1PalCharacter::DropAngleJitter);
		const FVector Direction = FRotator(0.f, Yaw, 0.f).Vector();

		const int32 Count = FMath::RandRange(Drop.MinCount, Drop.MaxCount);
		ADC1PickupItem* Pickup = ADC1PickupItem::SpawnDrop(GetWorld(), Drop.ItemID, Count, Origin + Direction * BodyRadius);
		if (nullptr == Pickup)
		{
			continue;
		}

		UPrimitiveComponent* PickupRoot = Cast<UPrimitiveComponent>(Pickup->GetRootComponent());
		if (nullptr == PickupRoot)
		{
			continue;
		}

		PickupRoot->SetPhysicsLinearVelocity(Direction * DropScatterSpeed + FVector::UpVector * DropLaunchSpeed);
	}
}

UAbilitySystemComponent* ADC1PalCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADC1PalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADC1PalCharacter, Team);
	DOREPLIFETIME(ADC1PalCharacter, OwnerPlayerState);
	DOREPLIFETIME(ADC1PalCharacter, BasePalBox);
	DOREPLIFETIME(ADC1PalCharacter, bIsAlpha);
}

ETeamAttitude::Type ADC1PalCharacter::GetTeamAttitudeTowards(const AActor& Other) const
{
	return DC1Team::GetAttitude(Team, DC1Team::ResolveTeam(Other));
}

FGenericTeamId ADC1PalCharacter::GetGenericTeamId() const
{
	return DC1Team::ToGenericTeamId(Team);
}

void ADC1PalCharacter::OnRep_Team()
{
	OnTeamChanged();
}

void ADC1PalCharacter::OnRep_IsAlpha()
{
	ApplyAlphaScale();
}

void ADC1PalCharacter::ApplyAlphaScale()
{
	if (false == bIsAlpha || nullptr == PalData)
	{
		return;
	}

	SetActorScale3D(FVector(PalData->AlphaScale));
}

void ADC1PalCharacter::ApplyTeamCollisionProfile()
{
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionProfileName((EDC1Team::Player == Team) ? TEXT("AllyPal") : TEXT("EnemyPal"));
	}

	if (IsValid(HitboxComponent))
	{
		HitboxComponent->SetCollisionProfileName((EDC1Team::Player == Team) ? TEXT("AllyPalHitbox") : TEXT("EnemyPalHitbox"));
	}
}

void ADC1PalCharacter::OnTeamChanged()
{
	ApplyTeamCollisionProfile();

	// 적대 판정은 sight query 생성 시점에 한 번만 해서 진영이 바뀌면 양쪽을 다 갱신!

	// 남들이 나를 다시 판정
	if (UAIPerceptionSystem* PerceptionSystem = UAIPerceptionSystem::GetCurrent(this))
	{
		PerceptionSystem->UnregisterSource(*this);
		PerceptionSystem->RegisterSource(*this);
	}

	// 내가 남들을 다시 판정 + 진영에 맞는 행동 트리로 갈아탄다
	if (ADC1PalAIController* PalController = Cast<ADC1PalAIController>(GetController()))
	{
		PalController->HandleTeamChanged();
	}

	if (IsValid(HealthWidgetComponent))
	{
		if (UDC1PalHealthWidget* HealthWidget = Cast<UDC1PalHealthWidget>(HealthWidgetComponent->GetUserWidgetObject()))
		{
			// 색 정하기
			HealthWidget->RefreshTeamColor();
		}
	}
}
