// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/DC1PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "OnlineSubsystem.h"
#include "Define/DC1Macro.h"
#include "Save/DC1SaveSubsystem.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "Tags/DC1GamePlayTags.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "UI/Tab/DC1TabWidget.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Pal/DC1PalPartyComponent.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1EquipComponent.h"
#include "Component/DC1CharacterMovementComponent.h"


#include "Component/Build/DC1BuildPlacementComponent.h"
#include "Component/Interact/DC1InteractComponent.h"

#include "Interface/DC1InteractableInterface.h"

#include "Subsystem/DC1ViewModelSubsystem.h"

#include "AbilitySystem/Effects/DC1GameplayEffect_Damage.h"

// Debug : 외형 테스트용. Debug region 제거 시 같이 제거
#include "Component/Appearance/DC1AppearanceComponent.h"
#include "Data/DA/Appearance/DC1AppearanceRegistryData.h"


ADC1PlayerController::ADC1PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BuildPlacementComponent = CreateDefaultSubobject<UDC1BuildPlacementComponent>("BuildPlacementComponent");

	InteractComponent = CreateDefaultSubobject<UDC1InteractComponent>("InteractComponent");
}

void ADC1PlayerController::SetCursorMode(bool OnTurn)
{
	IsTurn = OnTurn;
}

void ADC1PlayerController::SetIMCEnabled(const FGameplayTag& IMCTag, bool Enabled)
{
	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			const FDC1InputMappingContext* IMCData = InputData->FindIMCEntrybyTag(IMCTag);
			if (nullptr == IMCData || nullptr == IMCData->InputMappingContext)
			{
				UE_LOG(LogTemp, Warning, TEXT("ADC1PlayerController :: Cant Find IMC Data or IMC : %s"), *IMCTag.ToString());
				return;
			}

			if (IMCData->Priority == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("ADC1PlayerController :: %s is Base IMC. Cant Add/Remove"), *IMCTag.ToString());
				return;
			}

			if (true == Enabled)
			{
				Subsystem->AddMappingContext(IMCData->InputMappingContext, IMCData->Priority);
			}
			else
			{
				Subsystem->RemoveMappingContext(IMCData->InputMappingContext);
			}
		}
	}
}

void ADC1PlayerController::SetBuildMode(bool Enabled)
{
	if (false == ::IsValid(ControlObject))
	{
		return;
	}

	if (true == ::IsValid(InteractComponent))
	{
		InteractComponent->SetInteractEnabled(!Enabled);
	}

	UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(ControlObject->GetAbilitySystemComponent());
	if (true == ::IsValid(ASC))
	{
		int32 NewCount = (true == Enabled) ? 1 : 0;

		ASC->SetLooseGameplayTagCount(DC1StateTags::State_Build_Placement, NewCount);
	}
}

void ADC1PlayerController::PressAbilityInput(const FGameplayTag& InAbilityTag)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(ControlObject->GetAbilitySystemComponent()))
	{
		ASC->AbilityInputPressedByTag(InAbilityTag);
	}
}

void ADC1PlayerController::ReleaseAbilityInput(const FGameplayTag& InAbilityTag)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(ControlObject->GetAbilitySystemComponent()))
	{
		ASC->AbilityInputReleasedByTag(InAbilityTag);
	}
}

void ADC1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (false == IsLocalController())
	{
		return;
	}

	// Subsystem Log Check
	ShowSubsystemLog();

	SetBaseIMC(DC1IMCTags::IMC_Default);

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// UI Changed -> IMC Chage Binding
	UIManager->OnScreenStackChanged.AddUObject(this, &ThisClass::OnUIChange);

	FTimerHandle FadeOutTimerHandle;
	GetWorldTimerManager().SetTimer(FadeOutTimerHandle, FTimerDelegate::CreateWeakLambda(UIManager, [UIManager]()
		{
			UIManager->FadeOut(2.f);
		}), 2.f, false);

	InitPlayerStateBindings();
}

void ADC1PlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// Change IMC Binding Remove
	UIManager->OnScreenStackChanged.RemoveAll(this);

	if (false == IsLocalController())
	{
		return;
	}

	if (UDC1ViewModelSubsystem* ViewModelSubsystem = GetLocalPlayer()->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		ViewModelSubsystem->ClearAll();
	}
}

void ADC1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

		SetUpDefaultInput(EnhancedInputComponent, InputData);
		SetUpUIInput(EnhancedInputComponent, InputData);
	}

	SetUp_DebugInput();
}

void ADC1PlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	ControlObject = Cast<ADC1Player>(InPawn);
}

bool ADC1PlayerController::InputKey(const FInputKeyEventArgs& Params)
{
	if (IE_Pressed == Params.Event && true == IsActiveScreen)
	{
		UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
		if (true == ::IsValid(UIManager))
		{
			UIManager->RouteInputKey(Params.Key);
		}
	}

	return Super::InputKey(Params);
}

void ADC1PlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	InitPlayerStateBindings();
}

void ADC1PlayerController::Server_SavePlayerData_Implementation()
{
	const bool bSaved = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>()->SavePlayerData(GetPlayerState<ADC1PlayerState>());
	LOG(TEXT("[Save] SavePlayerData Result=%d"), bSaved);
}

void ADC1PlayerController::OnMove(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(ControlObject->GetCharacterMovement());
	if (false == IsValid(CMC))
	{
		return;
	}

	const FVector2D MoveVector = Value.Get<FVector2D>();

	// ClimbAtTop 판정
	if (true == CMC->CanMantle() && MoveVector.X > 0.f)
	{
		ControlObject->PlayAbility(DC1PlayerTags::Ability_Player_ClimbAtTop);
	}

	const bool IsClimbing = CMC->IsClimbing();

	FVector Forward = {};
	FVector Right = {};

	if (IsClimbing)
	{
		Right = CMC->GetClimbRightVector();
		Forward = CMC->GetClimbUpVector();
	}
	else
	{
		FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);
		Forward = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
		Right = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);
	}

	ControlObject->AddMovementInput(Forward * MoveVector.X);
	ControlObject->AddMovementInput(Right * MoveVector.Y);
}

void ADC1PlayerController::OnTurn(const FInputActionValue& Value)
{
	if (false == IsTurn)
	{
		return;
	}

	const FVector2D LookVector = Value.Get<FVector2D>();
	AddYawInput(LookVector.X);
	AddPitchInput(-1.f * LookVector.Y);
}

void ADC1PlayerController::OnSprint(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	// 웅크림/스태미나 검사는 GA_Player_Sprint의 CanActivateAbility가 한다
	ControlObject->PlayAbility(DC1PlayerTags::Ability_Player_Sprint);
}

void ADC1PlayerController::RestoreSprint(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}
	ControlObject->CancelAbility(DC1PlayerTags::Ability_Player_Sprint);
}

void ADC1PlayerController::OnToggleMove(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}
	ControlObject->ToggleMove();
}

void ADC1PlayerController::OnJump(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	ControlObject->UnCrouch();
	ControlObject->TryJumpOrGlide();
}

void ADC1PlayerController::StopJumping(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	ControlObject->StopJumping();
}

void ADC1PlayerController::OnCrouch(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	if (UCharacterMovementComponent* CMC = ControlObject->GetCharacterMovement())
	{
		bool IsCrouch = CMC->IsCrouching();
		bool IsFalling = CMC->IsFalling();

		if (true == IsFalling)
			return;

		if (false == IsCrouch && false == ControlObject->CanCrouch())
			return;

		ControlObject->CancelAbility(DC1PlayerTags::Ability_Player_Sprint);
		IsCrouch ? ControlObject->UnCrouch() : ControlObject->Crouch();
	}
}

void ADC1PlayerController::OnDodge(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	ControlObject->PlayAbility(DC1PlayerTags::Ability_Player_Dodge);
}

void ADC1PlayerController::OnAttack(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	ControlObject->PlayAbility(DC1PlayerTags::Ability_Player_Attack);
}

void ADC1PlayerController::OnAttackPressed(const FInputActionValue& Value)
{
	PressAbilityInput(DC1PlayerTags::Ability_Player_Attack);
}

void ADC1PlayerController::OnAttackReleased(const FInputActionValue& Value)
{
	ReleaseAbilityInput(DC1PlayerTags::Ability_Player_Attack);
}

void ADC1PlayerController::ChangeWeapon(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS) || false == IsValid(PS->GetEquipComponent()))
	{
		return;
	}

	const float AxisValue = Value.Get<float>();
	if (true == FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(ControlObject->GetAbilitySystemComponent()))
	{
		ASC->CancelAbilityByTag(DC1PlayerTags::Ability_Player_Attack);
	}

	// 절대 인덱스가 아니라 증감량을 보낸다. 빠르게 굴려도 서버에서 순서대로 누적된다
	const int32 Delta = (AxisValue > 0.f) ? 1 : -1;

	PS->GetEquipComponent()->Server_ChangeWeapon(Delta);
}

void ADC1PlayerController::OnThrowPressed(const FInputActionValue& Value)
{
	PressAbilityInput(DC1PlayerTags::Ability_Player_PalSphere);
}

void ADC1PlayerController::OnThrowReleased(const FInputActionValue& Value)
{
	ReleaseAbilityInput(DC1PlayerTags::Ability_Player_PalSphere);
}

void ADC1PlayerController::OnCallPalPressed(const FInputActionValue& Value)
{
	const ADC1PlayerState* DC1PlayerState = GetPlayerState<ADC1PlayerState>();
	if (nullptr == DC1PlayerState)
	{
		return;
	}

	if (UDC1PalPartyComponent* Party = DC1PlayerState->GetPalPartyComponent())
	{
		Party->Server_ToggleDeploy();
	}
}

void ADC1PlayerController::OnPalSlotPrevPressed(const FInputActionValue& Value)
{
	const ADC1PlayerState* DC1PlayerState = GetPlayerState<ADC1PlayerState>();
	if (nullptr == DC1PlayerState)
	{
		return;
	}

	if (UDC1PalPartyComponent* Party = DC1PlayerState->GetPalPartyComponent())
	{
		Party->CycleSlot(-1);
	}
}

void ADC1PlayerController::OnPalSlotNextPressed(const FInputActionValue& Value)
{
	const ADC1PlayerState* DC1PlayerState = GetPlayerState<ADC1PlayerState>();
	if (nullptr == DC1PlayerState)
	{
		return;
	}

	if (UDC1PalPartyComponent* Party = DC1PlayerState->GetPalPartyComponent())
	{
		Party->CycleSlot(1);
	}
}

void ADC1PlayerController::OnClimbReleasePressed(const FInputActionValue& Value)
{
	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(ControlObject->GetCharacterMovement());
	if (false == IsValid(CMC))
		return;

	CMC->ReleaseClimb();
	ControlObject->Server_ReleaseClimb();
}

void ADC1PlayerController::InitPlayerStateBindings()
{
	ADC1PlayerState* OwnerPlayerState = GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(OwnerPlayerState))
	{
		return;		// 아직 도착 전. OnRep_PlayerState가 다시 부른다
	}

	ULocalPlayer* OwnerLocalPlayer = GetLocalPlayer();
	if (false == ::IsValid(OwnerLocalPlayer))
	{
		return;
	}

	// VM 생성/조회는 서브시스템이, 위젯은 스스로
	if (UDC1ViewModelSubsystem* ViewModelSubsystem = OwnerLocalPlayer->GetSubsystem<UDC1ViewModelSubsystem>())
	{
		ViewModelSubsystem->BindAll(OwnerPlayerState);
	}

	// Customize 적용
	UDC1SaveSubsystem* SaveSubsystem = GetGameInstance()->GetSubsystem<UDC1SaveSubsystem>();
	if (false == ::IsValid(SaveSubsystem))
	{
		return;
	}

	FDC1AppearanceData LocalAppearance;
	if (true == SaveSubsystem->LoadCustomizeData(&LocalAppearance))
	{
		OwnerPlayerState->GetAppearanceComponent()->ImportSaveData(LocalAppearance);
	}

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	UIManager->ShowHUD();

	if (false == ::IsValid(CachedMainTabWidget))
	{
		CachedMainTabWidget = Cast<UDC1TabWidget>(UIManager->CreateWidgetWithTag(DC1UITags::UI_Main_Tab.GetTag()));
	}
}

#pragma region Debug
// TODO : HP 감소 / HUD 확인용. 테스트 끝나면 이 region 통째로 제거
//        (.h 의 동명 region, SetupInputComponent 의 SetUp_DebugInput() 호출 1줄도 같이)
namespace
{
	constexpr float DebugDamageAmount = 10.f;

	// 태그 이름순으로 다음 항목. 현재 태그가 없으면(빈 태그 포함) 첫 항목
	FGameplayTag GetNextDebugAppearanceTag(const TMap<FGameplayTag, FDC1AppearancePartDef>& InEntries, const FGameplayTag& InCurrent)
	{
		TArray<FGameplayTag> Keys;
		InEntries.GenerateKeyArray(Keys);
		if (0 == Keys.Num())
		{
			return InCurrent;
		}

		Keys.Sort([](const FGameplayTag& A, const FGameplayTag& B) { return A.GetTagName().LexicalLess(B.GetTagName()); });

		const int32 CurrentIndex = Keys.IndexOfByKey(InCurrent);
		return Keys[(CurrentIndex + 1) % Keys.Num()];
	}
}

void ADC1PlayerController::SetUp_DebugInput()
{
	if (nullptr == InputComponent)
	{
		return;
	}

	InputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &ThisClass::OnDebugDamageSelf);
	InputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &ThisClass::OnDebugSavePlayer);

	InputComponent->BindKey(EKeys::NumPadThree, IE_Pressed, this, &ThisClass::OnDebugNextHair);
	InputComponent->BindKey(EKeys::NumPadFour, IE_Pressed, this, &ThisClass::OnDebugNextHead);
	InputComponent->BindKey(EKeys::NumPadFive, IE_Pressed, this, &ThisClass::OnDebugRandomHairColor);
	InputComponent->BindKey(EKeys::NumPadSix, IE_Pressed, this, &ThisClass::OnDebugRandomSkinColor);
}

void ADC1PlayerController::OnDebugNextHair()
{
	const UDC1AppearanceRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1AppearanceRegistryData>(DC1AssetTags::Asset_Appearance_Registry);
	if (false == ::IsValid(Registry))
	{
		LOG_WARNING(TEXT("[Debug] Appearance Registry not found"));
		return;
	}

	DebugAppearance.HairTag = GetNextDebugAppearanceTag(Registry->GetHairs(), DebugAppearance.HairTag);
	ApplyDebugAppearance();
}

void ADC1PlayerController::OnDebugNextHead()
{
	const UDC1AppearanceRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1AppearanceRegistryData>(DC1AssetTags::Asset_Appearance_Registry);
	if (false == ::IsValid(Registry))
	{
		LOG_WARNING(TEXT("[Debug] Appearance Registry not found"));
		return;
	}

	DebugAppearance.HeadTag = GetNextDebugAppearanceTag(Registry->GetHeads(), DebugAppearance.HeadTag);
	ApplyDebugAppearance();
}

void ADC1PlayerController::OnDebugRandomHairColor()
{
	DebugAppearance.HairColor = FLinearColor::MakeRandomColor();
	ApplyDebugAppearance();
}

void ADC1PlayerController::OnDebugRandomSkinColor()
{
	DebugAppearance.SkinColor = FLinearColor::MakeRandomColor();
	ApplyDebugAppearance();
}

void ADC1PlayerController::ApplyDebugAppearance()
{
	ADC1PlayerState* DebugPlayerState = GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(DebugPlayerState) || false == ::IsValid(DebugPlayerState->GetAppearanceComponent()))
	{
		LOG_WARNING(TEXT("[Debug] No PlayerState or AppearanceComponent"));
		return;
	}

	DebugPlayerState->GetAppearanceComponent()->ApplyAppearance(DebugAppearance);

	LOG(TEXT("[Debug] Appearance Hair=%s Head=%s HairColor=%s SkinColor=%s"),
		*DebugAppearance.HairTag.ToString(),
		*DebugAppearance.HeadTag.ToString(),
		*DebugAppearance.HairColor.ToString(),
		*DebugAppearance.SkinColor.ToString());
}

void ADC1PlayerController::OnDebugSavePlayer()
{
	LOG(TEXT("[Debug] Save requested"));
	Server_SavePlayerData();
}

void ADC1PlayerController::OnDebugDamageSelf()
{
	ServerDebugDamageSelf(DebugDamageAmount);
}

void ADC1PlayerController::ServerDebugDamageSelf_Implementation(float InDamageAmount)
{
	ADC1PlayerState* OwnerPlayerState = GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(OwnerPlayerState))
	{
		return;
	}

	UAbilitySystemComponent* ASC = OwnerPlayerState->GetAbilitySystemComponent();
	if (nullptr == ASC)
	{
		return;
	}

	FGameplayEffectContextHandle Context = ASC->MakeEffectContext();
	Context.AddSourceObject(this);

	FGameplayEffectSpecHandle Spec = ASC->MakeOutgoingSpec(UDC1GameplayEffect_Damage::StaticClass(), 1.f, Context);
	if (false == Spec.IsValid())
	{
		return;
	}

	// 체력 감산은 AttributeSet의 PostGameplayEffectExecute가 한다. 여기서는 양수로 넘긴다
	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Damage, InDamageAmount);

	ASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}
#pragma endregion

void ADC1PlayerController::OnUIChange()
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	bool IsActiveNow = UIManager->HasActiveScreen();

	if (IsActiveNow == IsActiveScreen)
	{
		return;
	}

	if (true == UIManager->HasActiveScreen())
	{
		SetBaseIMC(DC1IMCTags::IMC_UI);
	}
	else
	{
		SetBaseIMC(DC1IMCTags::IMC_Default);
	}

	IsActiveScreen = IsActiveNow;
}

void ADC1PlayerController::SetBaseIMC(const FGameplayTag& IMCTag)
{
	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (true == CurrentIMCTag.IsValid())
			{
				Subsystem->RemoveMappingContext(InputData->FindInputMappingContextbyTag(CurrentIMCTag));
			}

			Subsystem->AddMappingContext(InputData->FindInputMappingContextbyTag(IMCTag), 0);

			CurrentIMCTag = IMCTag;
		}
	}
}

void ADC1PlayerController::ShowSubsystemLog()
{
	IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
	LOG(TEXT("[OSS] Subsystem = %s"),
		OSS ? *OSS->GetSubsystemName().ToString() : TEXT("None"));
}

#pragma region UI_TEST
void ADC1PlayerController::OpenTabWidget(FGameplayTag InUITag, FGameplayTag InTab)
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	UDC1TabWidget* TabWidget = Cast<UDC1TabWidget>(UIManager->PushScreen(InUITag, true));
	if (nullptr != TabWidget)
	{
		if (true == InTab.IsValid())
		{
			if (false == TabWidget->SelectTab(InTab))
			{
				TabWidget->SelectFirstInactiveTab();
			}
		}
		else
		{
			TabWidget->SelectFirstInactiveTab();
		}
	}
}
#pragma endregion

void ADC1PlayerController::SetUpDefaultInput(UEnhancedInputComponent* EnhancedInputComponent, const UDC1InputData* InputData)
{
	// Move
	if (const UInputAction* MoveAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Move))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::OnMove);
	}
	// Turn
	if (const UInputAction* TurnAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Turn))
	{
		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ThisClass::OnTurn);
	}
	// Sprint
	if (const UInputAction* SprintAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Sprint))
	{
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ThisClass::OnSprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::RestoreSprint);
	}
	// ToggleMove
	if (const UInputAction* ToggleMoveAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Toggle_Move))
	{
		EnhancedInputComponent->BindAction(ToggleMoveAction, ETriggerEvent::Started, this, &ThisClass::OnToggleMove);
	}
	// Jump
	if (const UInputAction* JumpAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Jump))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::OnJump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);
	}
	// Crouch
	if (const UInputAction* CrouchAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Crouch))
	{
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &ThisClass::OnCrouch);
	}
	// Attack
	if (const UInputAction* AttackAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Attack))
	{
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ThisClass::OnAttackPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::OnAttack);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ThisClass::OnAttackReleased);
	}
	// Dodge
	if (const UInputAction* DodgeAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Dodge))
	{
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Started, this, &ThisClass::OnDodge);
	}

	// ChangeWeapon
	if (const UInputAction* ChangeWeaponAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_ChangeWeapon))
	{
		EnhancedInputComponent->BindAction(ChangeWeaponAction, ETriggerEvent::Triggered, this, &ThisClass::ChangeWeapon);
	}

	// Throw
	if (const UInputAction* ThrowAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Throw))
	{
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Started, this, &ThisClass::OnThrowPressed);
		EnhancedInputComponent->BindAction(ThrowAction, ETriggerEvent::Completed, this, &ThisClass::OnThrowReleased);
	}

	// CallPal
	if (const UInputAction* CallPalAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_CallPal))
	{
		EnhancedInputComponent->BindAction(CallPalAction, ETriggerEvent::Started, this, &ThisClass::OnCallPalPressed);
	}

	// PalSlot Prev / Next
	if (const UInputAction* PalSlotPrevAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_PalSlotPrev))
	{
		EnhancedInputComponent->BindAction(PalSlotPrevAction, ETriggerEvent::Started, this, &ThisClass::OnPalSlotPrevPressed);
	}

	if (const UInputAction* PalSlotNextAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_PalSlotNext))
	{
		EnhancedInputComponent->BindAction(PalSlotNextAction, ETriggerEvent::Started, this, &ThisClass::OnPalSlotNextPressed);
	}

	// Climb
	if (const UInputAction* ClimbReleaseAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_ClimbRelease))
	{
		EnhancedInputComponent->BindAction(ClimbReleaseAction, ETriggerEvent::Started, this, &ThisClass::OnClimbReleasePressed);
	}
}

void ADC1PlayerController::SetUpUIInput(UEnhancedInputComponent* EnhancedInputComponent, const UDC1InputData* InputData)
{
	// Open Tab - Inventory Widget
	if (const UInputAction* OpenInventory = InputData->FindInputActionbyTag(DC1InputTags::Input_UI_Inventory))
	{
		EnhancedInputComponent->BindAction(OpenInventory, ETriggerEvent::Started, this, &ThisClass::OpenTabWidget, DC1UITags::UI_Main_Tab.GetTag(), DC1UITags::UI_Main_Tab_Inventory.GetTag());
	}

	// Open Tab - HasPal Widget
	if (const UInputAction* OpenHasPal = InputData->FindInputActionbyTag(DC1InputTags::Input_UI_HasPal))
	{
		EnhancedInputComponent->BindAction(OpenHasPal, ETriggerEvent::Started, this, &ThisClass::OpenTabWidget, DC1UITags::UI_Main_Tab.GetTag(), DC1UITags::UI_Main_Tab_HasPal.GetTag());
	}

	// Open Tab - TechTree Widget
	if (const UInputAction* OpenTechTree = InputData->FindInputActionbyTag(DC1InputTags::Input_UI_TechTree))
	{
		EnhancedInputComponent->BindAction(OpenTechTree, ETriggerEvent::Started, this, &ThisClass::OpenTabWidget, DC1UITags::UI_Main_Tab.GetTag(), DC1UITags::UI_Main_Tab_TechTree.GetTag());
	}

	// Open Build Widget
	if (const UInputAction* OpenBuild = InputData->FindInputActionbyTag(DC1InputTags::Input_UI_Build))
	{
		EnhancedInputComponent->BindAction(OpenBuild, ETriggerEvent::Started, this, &ThisClass::OpenTabWidget, DC1UITags::UI_Main_Build.GetTag(), FGameplayTag::EmptyTag);
	}

	// Open PalBox Widget
	if (const UInputAction* OpenPalBox = InputData->FindInputActionbyTag(DC1InputTags::Input_UI_PalBox))
	{
		EnhancedInputComponent->BindAction(OpenPalBox, ETriggerEvent::Started, this, &ThisClass::OpenTabWidget, DC1UITags::UI_Main_PalBox.GetTag(), FGameplayTag::EmptyTag);
	}
}