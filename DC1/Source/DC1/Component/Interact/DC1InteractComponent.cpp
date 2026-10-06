// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Interact/DC1InteractComponent.h"
#include "Define/DC1Enums.h"
#include "Actor/BuildActor/Interact/DC1InteractBuildActor.h"
#include "Character/Player/DC1Player.h"

#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Interface/DC1InteractableInterface.h"

#include "Character/Player/DC1PlayerController.h"
#include "Tags/Input/DC1GameplayTag_Input.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"

#include "Subsystem/DC1UIManagerSubsystem.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "UI/World/Interact/DC1InteractPromptWidget.h"

#include "Subsystem/DC1BuildDestroySubsystem.h"


// Sets default values for this component's properties
UDC1InteractComponent::UDC1InteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetIsReplicatedByDefault(true);
}

IDC1InteractableInterface* UDC1InteractComponent::GetCurrentInteractable() const
{
	if (false == CurrentTarget.IsValid())
	{
		return nullptr;
	}

	return Cast<IDC1InteractableInterface>(CurrentTarget.Get());
}

ADC1Player* UDC1InteractComponent::GetPlayer() const
{
	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	return PC->GetPawn<ADC1Player>();
}

UDC1UIManagerSubsystem* UDC1InteractComponent::GetUIManager() const
{
	UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return nullptr;
	}

	UGameInstance* GameInstance = World->GetGameInstance();
	if (false == ::IsValid(GameInstance))
	{
		return nullptr;
	}

	return GameInstance->GetSubsystem<UDC1UIManagerSubsystem>();
}

void UDC1InteractComponent::SetInteractEnabled(bool InEnabled)
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
	{
		return;
	}

	SetComponentTickEnabled(InEnabled);

	SetCurrentTarget(nullptr);
}

void UDC1InteractComponent::SetTargetLocked(bool InLocked)
{
	IsTargetLocked = InLocked;
}

void UDC1InteractComponent::BeginPlay()
{
	Super::BeginPlay();

	SetUpInteractInput();
	SetInteractEnabled(true);

	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
	{
		return;
	}
	
	UDC1UIManagerSubsystem* UIManager = GetUIManager();
	if (false == ::IsValid(UIManager))
	{
		return;
	}

	UIManager->OnScreenStackChanged.AddUObject(this, &ThisClass::UpdatePromptAndIMC);
}

void UDC1InteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TickHoldConfirm(DeltaTime);

	if (true == IsTargetLocked)
	{
		return;
	}

	SetCurrentTarget(FindBestTarget());
}

void UDC1InteractComponent::RequestCancelBuild(ADC1InteractBuildActor* InBuildTarget)
{
	Server_CancelBuild(InBuildTarget);
}

void UDC1InteractComponent::RequestStartWork(ADC1InteractBuildActor* InBuildTarget)
{
	Server_StartWork(InBuildTarget);
}

void UDC1InteractComponent::RequestStopWork()
{
	Server_StopWork();
}

void UDC1InteractComponent::Server_CancelBuild_Implementation(ADC1InteractBuildActor* InBuildTarget)
{
	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (false == ::IsValid(InBuildTarget) || false == InBuildTarget->IsUnderConstruction() || false == IsInInteractRange(Player, InBuildTarget))
	{
		return;
	}

	UWorld* World = GetWorld();
	if (false == ::IsValid(World))
	{
		return;
	}

	UDC1BuildDestroySubsystem* BuildDestroySubsystem = World->GetSubsystem<UDC1BuildDestroySubsystem>();
	if (true == ::IsValid(BuildDestroySubsystem))
	{
		BuildDestroySubsystem->RequestDestroy(InBuildTarget);
	}
}

void UDC1InteractComponent::Server_StartWork_Implementation(ADC1InteractBuildActor* InBuildTarget)
{
	Server_StopWork_Implementation();

	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (false == ::IsValid(InBuildTarget) || false == IsInInteractRange(Player, InBuildTarget))
	{
		return;
	}

	InBuildTarget->AddWorker(Player);
	ServerWorkTarget = InBuildTarget;
}

void UDC1InteractComponent::Server_StopWork_Implementation()
{
	if (nullptr != ServerWorkTarget)
	{
		ADC1InteractBuildActor* BuildTarget = ServerWorkTarget.Get();
		if (true == ::IsValid(BuildTarget))
		{
			ADC1Player* Player = GetPlayer();
			if (false == ::IsValid(Player))
			{
				return;
			}

			BuildTarget->RemoveWorker(Player);
		}
	}

	ServerWorkTarget = nullptr;
}

AActor* UDC1InteractComponent::FindBestTarget() const
{
	const APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC))
	{
		return nullptr;
	}

	const ADC1Player* Player = PC->GetPawn<ADC1Player>();
	if (false == ::IsValid(Player))
	{
		return nullptr;
	}

	const USphereComponent* InteractDetectCollider = Player->GetInteractDetectCollider();
	if (false == ::IsValid(InteractDetectCollider))
	{
		return nullptr;
	}

	TArray<AActor*> OverlapBuffer;

	InteractDetectCollider->GetOverlappingActors(OverlapBuffer);
	if (true == OverlapBuffer.IsEmpty())
	{
		return nullptr;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	float BestDot = (FLT_MAX * -1.f);
	AActor* BestTarget = nullptr;

	for (AActor* OverlapActor : OverlapBuffer)
	{
		const IDC1InteractableInterface* Interactable = Cast<IDC1InteractableInterface>(OverlapActor);
		if (nullptr == Interactable)
		{
			continue;
		}

		const UPrimitiveComponent* InteractCollider = Interactable->GetInteractCollider();
		if (false == ::IsValid(InteractCollider))
		{
			continue;
		}

		FVector OverlapLocation = InteractCollider->GetComponentLocation();

		float Dot = FVector::DotProduct(ViewRotation.Vector(), (OverlapLocation - ViewLocation).GetSafeNormal());

		if (OverlapActor == CurrentTarget.Get())
		{
			Dot += CurrentTargetDotBonus;
		}

		if (Dot > BestDot)
		{
			BestTarget = OverlapActor;
			BestDot = Dot;
		}
	}

	return BestTarget;
}

void UDC1InteractComponent::SetCurrentTarget(AActor* InNewTarget)
{
	if (InNewTarget == CurrentTarget.Get())
	{
		return;
	}

	ReleaseAllHeldKeys();
	
	IDC1InteractableInterface* PrevInteractable = GetCurrentInteractable();
	if(nullptr != PrevInteractable)
	{
		PrevInteractable->GetOnInteractOptionsChanged().Remove(OptionsChangedHandle);
	}
	
	OptionsChangedHandle.Reset();
	HidePrompt();

	CurrentTarget = InNewTarget;

	IDC1InteractableInterface* CurrentInteractable = GetCurrentInteractable();
	if (nullptr != CurrentInteractable)
	{
		OptionsChangedHandle = CurrentInteractable->GetOnInteractOptionsChanged().AddUObject(this, &ThisClass::RefreshOptions);
		ShowPrompt();
	}
}

bool UDC1InteractComponent::IsInInteractRange(const ADC1Player* Player, const AActor* InTarget) const
{
	const USphereComponent* InteractDetectCollider = Player->GetInteractDetectCollider();
	if (false == ::IsValid(InteractDetectCollider))
	{
		return false;
	}

	const IDC1InteractableInterface* Interactble = Cast<IDC1InteractableInterface>(InTarget);
	if (nullptr == Interactble)
	{
		return false;
	}

	const UPrimitiveComponent* InteractCollider = Interactble->GetInteractCollider();

	if (false == ::IsValid(InteractDetectCollider) || false == ::IsValid(InteractCollider))
	{
		return false;
	}

	return InteractDetectCollider->IsOverlappingComponent(InteractCollider);
}

void UDC1InteractComponent::SetUpInteractInput()
{
	APlayerController* PC = GetOwner<APlayerController>();
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
	{
		return;
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PC->InputComponent);
	if (false == ::IsValid(EnhancedInputComponent))
	{
		return;
	}

	const UDC1InputData* InputData = UDC1AssetManager::GetAssetByTag<UDC1InputData>(DC1AssetTags::Asset_InputData);
	if (false == ::IsValid(InputData))
	{
		return;
	}

	if (const UInputAction* InteractF = InputData->FindInputActionbyTag(DC1InputInteractTags::Input_Interact_F))
	{
		EnhancedInputComponent->BindAction(InteractF, ETriggerEvent::Started, this, &ThisClass::OnInteractKeyPressed, EDC1InteractKey::F);
		EnhancedInputComponent->BindAction(InteractF, ETriggerEvent::Completed, this, &ThisClass::OnInteractKeyReleased, EDC1InteractKey::F);
	}

	if (const UInputAction* InteractV = InputData->FindInputActionbyTag(DC1InputInteractTags::Input_Interact_V))
	{
		EnhancedInputComponent->BindAction(InteractV, ETriggerEvent::Started, this, &ThisClass::OnInteractKeyPressed, EDC1InteractKey::V);
		EnhancedInputComponent->BindAction(InteractV, ETriggerEvent::Completed, this, &ThisClass::OnInteractKeyReleased, EDC1InteractKey::V);
	}

	if (const UInputAction* InteractC = InputData->FindInputActionbyTag(DC1InputInteractTags::Input_Interact_C))
	{
		EnhancedInputComponent->BindAction(InteractC, ETriggerEvent::Started, this, &ThisClass::OnInteractKeyPressed, EDC1InteractKey::C);
		EnhancedInputComponent->BindAction(InteractC, ETriggerEvent::Completed, this, &ThisClass::OnInteractKeyReleased, EDC1InteractKey::C);
	}
}

void UDC1InteractComponent::OnInteractKeyPressed(const FInputActionValue& Value, EDC1InteractKey InKey)
{
	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (false == ::IsValid(PC))
	{
		return;
	}

	const FDC1InteractOption* Option = FindOption(InKey);
	if (nullptr == Option)
	{
		return;
	}
	
	switch (Option->InputType)
	{
	case EDC1InteractInputType::Press :
	{
		IDC1InteractableInterface* Interactable = GetCurrentInteractable();
		if (nullptr == Interactable)
		{
			return;
		}

		Interactable->Interact(PC->GetPawn<ADC1Character>(), InKey);
		break;
	}

	case EDC1InteractInputType::HoldAbility :
	{
		PC->PressAbilityInput(Option->AbilityTag);
		HeldAbilityTags.Add(InKey, Option->AbilityTag);
		
		if (true == PromptWidget.IsValid())
		{
			PromptWidget->SetKeyHighlighted(InKey, true);
		}
		break;
	}

	case EDC1InteractInputType::HoldConfirm :
	{
		HoldConfirmKey = InKey;
		HoldConfirmElapsed = 0.f;
		break;
	}
	}
}

void UDC1InteractComponent::OnInteractKeyReleased(const FInputActionValue& Value, EDC1InteractKey InKey)
{
	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (true == ::IsValid(PC))
	{
		FGameplayTag ReleaseAbilityTag;
		if (true == HeldAbilityTags.RemoveAndCopyValue(InKey, ReleaseAbilityTag))
		{
			PC->ReleaseAbilityInput(ReleaseAbilityTag);
		}
	}

	if (true == HoldConfirmKey.IsSet() && HoldConfirmKey.GetValue() == InKey)
	{
		HoldConfirmKey.Reset();
		HoldConfirmElapsed = 0.f;
	}

	if (true == PromptWidget.IsValid())
	{
		PromptWidget->SetKeyHighlighted(InKey, false);
	}
}

const FDC1InteractOption* UDC1InteractComponent::FindOption(EDC1InteractKey InKey) const
{
	return CurrentOptions.FindByPredicate([InKey](const FDC1InteractOption& Option) { return Option.Key == InKey; });
}

void UDC1InteractComponent::ShowPrompt()
{
	IDC1InteractableInterface* Interactable = GetCurrentInteractable();
	if (nullptr == Interactable)
	{
		return;
	}

	const UPrimitiveComponent* InteractCollider = Interactable->GetInteractCollider();
	if (false == ::IsValid(InteractCollider))
	{
		return;
	}

	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	PromptComponent = Player->GetInteractPromptWidgetComponent();

	if (true == PromptComponent.IsValid())
	{
		FVector PromptLocation = InteractCollider->Bounds.Origin + FVector(0.f, 0.f, InteractCollider->Bounds.BoxExtent.Z);
		PromptComponent->SetWorldLocation(PromptLocation);

		// 이미 초기화된 경우에도 호출 가능한 함수
		PromptComponent->InitWidget();

		PromptWidget = Cast<UDC1InteractPromptWidget>(PromptComponent->GetUserWidgetObject());
	}

	RefreshOptions();
}

void UDC1InteractComponent::HidePrompt()
{
	if (true == PromptComponent.IsValid())
	{
		PromptComponent->SetVisibility(false);
	}

	PromptWidget = nullptr;
	CurrentOptions.Empty();

	UpdatePromptAndIMC();
}

void UDC1InteractComponent::RefreshOptions()
{
	ReleaseAllHeldKeys();

	CurrentOptions.Empty();

	IDC1InteractableInterface* Interactable = GetCurrentInteractable();
	if (nullptr == Interactable)
	{
		UpdatePromptAndIMC();
		return;
	}

	TArray<FDC1InteractOption> RawOptions;
	Interactable->GetInteractOptions(RawOptions);

	for (const FDC1InteractOption& Raw : RawOptions)
	{
		if (Raw.InputType == EDC1InteractInputType::HoldAbility && false == Raw.AbilityTag.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1InteractComponent :: Not Valid AbilityTag from Key : %s"), *UEnum::GetValueAsString(Raw.Key));
			continue;
		}

		if (CurrentOptions.ContainsByPredicate([&Raw](const FDC1InteractOption& Option) {
			return Option.Key == Raw.Key;
			}))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1InteractComponent :: Contains Key : %s"), *UEnum::GetValueAsString(Raw.Key));
			continue;
		}

		CurrentOptions.Add(Raw);
	}

	if (true == PromptWidget.IsValid())
	{
		PromptWidget->SetOptions(CurrentOptions);
	}

	UpdatePromptAndIMC();
}

void UDC1InteractComponent::ReleaseAllHeldKeys()
{
	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (true == ::IsValid(PC))
	{
		for (const TPair<EDC1InteractKey, FGameplayTag>& Pair : HeldAbilityTags)
		{
			PC->ReleaseAbilityInput(Pair.Value);
		}
	}

	HeldAbilityTags.Empty();
	HoldConfirmKey.Reset();
	HoldConfirmElapsed = 0.f;
}

void UDC1InteractComponent::TickHoldConfirm(float DeltaTime)
{
	if (false == HoldConfirmKey.IsSet())
	{
		return;
	}

	HoldConfirmElapsed += DeltaTime;

	const float Ratio = FMath::Clamp(HoldConfirmElapsed / HoldConfirmDuration, 0.f, 1.f);

	if (true == PromptWidget.IsValid())
	{
		PromptWidget->SetHoldRatio(HoldConfirmKey.GetValue(), Ratio);
	}

	if (Ratio >= 1.f)
	{
		// Interact 함수를 통해 Options 변경 가능성으로 인한 선 초기화
		EDC1InteractKey CachedKey = HoldConfirmKey.GetValue();
		HoldConfirmKey.Reset();
		HoldConfirmElapsed = 0.f;

		IDC1InteractableInterface* Interactable = GetCurrentInteractable();
		if (nullptr != Interactable)
		{
			ADC1Player* Player = GetPlayer();
			if (false == ::IsValid(Player))
			{
				return;
			}

			Interactable->Interact(Player, CachedKey);
		}
	}
}

void UDC1InteractComponent::UpdatePromptAndIMC()
{
	UDC1UIManagerSubsystem* UIManager = GetUIManager();
	bool HasActiveScreen = (true == ::IsValid(UIManager)) && (true == UIManager->HasActiveScreen());
	bool WantInteract = (false == CurrentOptions.IsEmpty()) && (false == HasActiveScreen);

	if (true == PromptComponent.IsValid())
	{
		PromptComponent->SetVisibility(WantInteract);
	}

	for (EDC1InteractKey Key : {EDC1InteractKey::F, EDC1InteractKey::V, EDC1InteractKey::C})
	{
		bool Enabled = (true == WantInteract) && (nullptr != FindOption(Key));
		SetKeyIMCLayer(Key, Enabled);
	}
}

void UDC1InteractComponent::SetKeyIMCLayer(EDC1InteractKey InKey, bool InEnabled)
{
	ADC1PlayerController* PC = GetOwner<ADC1PlayerController>();
	if (false == ::IsValid(PC) || false == PC->IsLocalController())
	{
		return;
	}

	if (false == InEnabled)
	{
		EnabledKeyLayers.Remove(InKey);
		PC->SetIMCEnabled(GetIMCTagFromKey(InKey), InEnabled);
	}
	else
	{
		if (true == EnabledKeyLayers.Contains(InKey))
		{
			return;
		}
		
		EnabledKeyLayers.Add(InKey);
		PC->SetIMCEnabled(GetIMCTagFromKey(InKey), InEnabled);
	}

}

const FGameplayTag UDC1InteractComponent::GetIMCTagFromKey(EDC1InteractKey InKey) const
{
	switch (InKey)
	{
	case EDC1InteractKey::F :
		return DC1IMCTags::IMC_Interact_F;
	case EDC1InteractKey::V:
		return DC1IMCTags::IMC_Interact_V;
	case EDC1InteractKey::C:
		return DC1IMCTags::IMC_Interact_C;
	default:
		return FGameplayTag::EmptyTag;
	}
}