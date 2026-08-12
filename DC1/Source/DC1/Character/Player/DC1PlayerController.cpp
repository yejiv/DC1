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

ADC1PlayerController::ADC1PlayerController(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void ADC1PlayerController::SetCursorMode(bool OnTurn)
{
	IsTurn = OnTurn;
}

void ADC1PlayerController::BeginPlay()
{
	Super::BeginPlay();

	SetUp_IMC(DC1IMCTags::IMC_Default);

	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	// UI ���濡 ���� IMC ���� ���ε�
	UIManager->OnScreenStackChanged.AddUObject(this, &ThisClass::OnUIChange);

	FTimerHandle FadeOutTimerHandle;
	GetWorldTimerManager().SetTimer(FadeOutTimerHandle, FTimerDelegate::CreateWeakLambda(UIManager, [UIManager]()
		{
			UIManager->FadeOut(2.f);
		}), 2.f, false);
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

	// IMC ���� ���ε� ���� 
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
		// Attack
		if (const UInputAction* AttackAction = InputData->FindInputActionbyTag(DC1InputTags::Input_Default_Attack))
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ThisClass::OnAttack);
		}

#pragma region UI_TEST
		// Open TechTree
		if (const UInputAction* UI_TechAction = InputData->FindInputActionbyTag(DC1InputTags::Input_UI_TechTree))
		{
			EnhancedInputComponent->BindAction(UI_TechAction, ETriggerEvent::Started, this, &ThisClass::OpenTech);
		}
#pragma endregion
	}

	SetUp_DebugInput(); // TODO : Debug region 제거 시 같이 삭제
}

void ADC1PlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);

	ControlObject = Cast<ADC1Player>(InPawn);
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

void ADC1PlayerController::OnAttack(const FInputActionValue& Value)
{
	if (false == IsValid(ControlObject))
	{
		return;
	}

	ControlObject->PlayAbility(DC1PlayerTags::Ability_Player_Attack);
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
		SetUp_IMC(DC1IMCTags::IMC_UI);
	}
	else
	{
		SetUp_IMC(DC1IMCTags::IMC_Default);
	}

	IsActiveScreen = IsActiveNow;
}

void ADC1PlayerController::SetUp_IMC(const FGameplayTag& IMC_Tag)
{
	if (const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			if (true == CurrentIMCTag.IsValid())
			{
				Subsystem->RemoveMappingContext(InputData->FindInputMappingContextbyTag(CurrentIMCTag));
			}

			Subsystem->AddMappingContext(InputData->FindInputMappingContextbyTag(IMC_Tag), 0);

			CurrentIMCTag = IMC_Tag;
		}
	}
}

#pragma region UI_TEST
void ADC1PlayerController::OpenTech()
{
	UDC1UIManagerSubsystem* UIManager = GetGameInstance()->GetSubsystem<UDC1UIManagerSubsystem>();
	if (false == ::IsValid(UIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to Find UIManager"));
		return;
	}

	if (IsOpen)
	{
		UIManager->PopScreen();
	}
	else
	{
		UDC1TabWidget* TabWidget = Cast<UDC1TabWidget>(UIManager->PushScreen(DC1UITags::UI_Main_Tab, true));
		if (nullptr != TabWidget)
		{
			TabWidget->SelectTab(DC1UITags::UI_Main_Tab_TechTree);
		}
	}

	IsOpen = !IsOpen;
}
#pragma endregion