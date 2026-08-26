// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/Player/DC1PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "Tags/DC1GamePlayTags.h"

#include "Subsystem/DC1UIManagerSubsystem.h"
#include "UI/Tab/DC1TabWidget.h"
#include "Tags/UI/DC1GameplayTag_UI.h"

#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1EquipComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "ViewModel/Inventory/DC1InventoryViewModel.h"
#include "Component/Inventory/DC1PlayerInventoryComponent.h"
#include "UI/Tab/Inventory/DC1InventoryWidget.h"
#include "Component/Build/DC1BuildPlacementComponent.h"
#include "Component/Build/DC1InteractComponent.h"


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

void ADC1PlayerController::BeginPlay()
{
	Super::BeginPlay();

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

	UDC1TabWidget* MainTabWidget = Cast<UDC1TabWidget>(UIManager->CreateWidgetWithTag(DC1UITags::UI_Main_Tab.GetTag()));

	// 인벤토리 ViewModel 생성
	InventoryViewModel = NewObject<UDC1InventoryViewModel>(this);
	if (IsValid(InventoryViewModel))
	{
		ADC1PlayerState* OwnerPlayerState = Cast<ADC1PlayerState>(GetPlayerState<APlayerState>());
		if (IsValid(OwnerPlayerState))
		{
			if (UDC1PlayerInventoryComponent* PlayerInventory = OwnerPlayerState->GetInventoryComponent())
			{
				InventoryViewModel->SetPlayerInventoryComponent(PlayerInventory);

				UDC1InventoryWidget* PlayerInventoryWidget = MainTabWidget->GetInventoryWidget();
				if (PlayerInventoryWidget)
				{
					PlayerInventoryWidget->SetViewModel(InventoryViewModel);
				}
			}
		}
	}
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
}

void ADC1PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

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
		// ChangeWeapon
		if (const UInputAction* ChangeWeaponAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_ChangeWeapon))
		{
			EnhancedInputComponent->BindAction(ChangeWeaponAction, ETriggerEvent::Triggered, this, &ThisClass::ChangeWeapon);
		}

		SetUpUIInput(EnhancedInputComponent, InputData);
	}

	SetUp_DebugInput(); // TODO : Debug region 제거 시 같이 삭제
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

void ADC1PlayerController::OnMove(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	const FVector2D MoveVector = Value.Get<FVector2D>();

	FRotator YawRotator(0.f, GetControlRotation().Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);

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

	if (UCharacterMovementComponent* CMC = ControlObject->GetCharacterMovement())
	{
		if (true == CMC->IsCrouching())
			return;
	}

	ControlObject->SetSprint(true);
}

void ADC1PlayerController::RestoreSprint(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}
	ControlObject->SetSprint(false);
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
	ControlObject->Jump();
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

		ControlObject->SetSprint(false);
		IsCrouch ? ControlObject->UnCrouch() : ControlObject->Crouch();
	}
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
	if (false == IsValid(ControlObject))
	{
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(ControlObject->GetAbilitySystemComponent()))
	{
		ASC->AbilityInputPressedByTag(DC1PlayerTags::Ability_Player_Attack);
	}
}

void ADC1PlayerController::OnAttackReleased(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(ControlObject->GetAbilitySystemComponent()))
	{
		ASC->AbilityInputReleasedByTag(DC1PlayerTags::Ability_Player_Attack);
	}
}

void ADC1PlayerController::ChangeWeapon(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	if (false == IsValid(ControlObject->Equipment))
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

	ControlObject->Equipment->Server_ChangeWeapon(Delta);
}

#pragma region Debug
// TODO : 무기 상태 전환 테스트용. 정식 장착 로직 나오면 이 region 통째로 제거
//        (.h 의 동명 region, SetupInputComponent 의 SetUp_DebugInput() 호출 1줄도 같이)
void ADC1PlayerController::SetUp_DebugInput()
{
	InputComponent->BindKey(EKeys::NumPadOne, IE_Pressed, this, &ThisClass::OnDebugWeaponNone);
	InputComponent->BindKey(EKeys::NumPadTwo, IE_Pressed, this, &ThisClass::OnDebugWeaponMelee);
	InputComponent->BindKey(EKeys::NumPadThree, IE_Pressed, this, &ThisClass::OnDebugWeaponBow);
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
}

void ADC1PlayerController::OnDebugWeaponNone()
{
	ServerSetWeaponState(DC1StateTags::State_Weapon_None);
}

void ADC1PlayerController::OnDebugWeaponMelee()
{
	ServerSetWeaponState(DC1StateTags::State_Weapon_Melee);
}

void ADC1PlayerController::OnDebugWeaponBow()
{
	ServerSetWeaponState(DC1StateTags::State_Weapon_Bow);
}

void ADC1PlayerController::ServerSetWeaponState_Implementation(FGameplayTag NewStateTag)
{
	ADC1PlayerState* PS = GetPlayerState<ADC1PlayerState>();
	if (false == IsValid(PS))
	{
		UE_LOG(LogTemp, Warning, TEXT("[DebugWeapon] PlayerState X"));
		return;
	}

	if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent()))
	{
		ASC->SetExclusiveStateTag(DC1StateTags::State_Weapon, NewStateTag);
	}
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