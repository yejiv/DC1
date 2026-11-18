// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/Interact/DC1InteractComponent.h"
#include "Define/DC1Enums.h"
#include "Actor/BuildActor/Interact/Production/DC1ProductionBuildActor.h"
#include "Actor/BuildActor/Interact/Production/Craft/DC1CraftBuildActor.h"
#include "Item/Pickup/DC1PickupItem.h"
#include "Character/Player/DC1Player.h"

#include "Components/SphereComponent.h"

#include "Component/Widget/DC1ScreenOffsetWidgetComponent.h"
#include "Interface/DC1InteractableInterface.h"

#include "Character/Player/DC1PlayerController.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Tags/Input/DC1GameplayTag_Input.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Build/DC1GameplayTag_Build.h"
#include "Tags/Player/DC1GameplayTag_Player.h"

#include "Subsystem/DC1UIManagerSubsystem.h"

#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "System/DC1AssetManager.h"
#include "Data/DA/Input/DC1InputData.h"

#include "UI/World/Interact/DC1InteractPromptWidget.h"

#include "Subsystem/DC1BuildDestroySubsystem.h"

#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UDC1InteractComponent::UDC1InteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	PrimaryComponentTick.TickGroup = TG_PostUpdateWork;

	SetIsReplicatedByDefault(true);

	// 작업 종류( 건설 / 벌목 / 채광 등 )는 작업 GA가 대상에서 읽음 - 옵션은 Interact.Work 하나
	InteractionToAbility.Add(DC1InteractTags::Interact_Work, DC1PlayerTags::Ability_Player_Work);
	InteractionToAbility.Add(DC1InteractTags::Interact_Revive, DC1PlayerTags::Ability_Player_Revive);
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

	bool IsEnabled = InEnabled;

	// Player Down / Dead 이벤트 이후 다른 이유로 인한 변경 예외처리 (ex.BuildMode 변경, Down → Dead 전환 시 Down 해제)
	ADC1Character* Owner = PC->GetPawn<ADC1Character>();
	if (true == InEnabled && true == ::IsValid(Owner) && true == Owner->IsDownOrDead())
	{
		IsEnabled = false;
	}

	SetComponentTickEnabled(IsEnabled);

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

	UIManager->OnScreenStackChanged.AddUObject(this, &ThisClass::HandleScreenStackChanged);
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

void UDC1InteractComponent::RequestCollectProduct(ADC1ProductionBuildActor* InProductionTarget)
{
	Server_CollectProduct(InProductionTarget);
}

void UDC1InteractComponent::RequestPickupItem(ADC1PickupItem* InPickup)
{
	Server_PickupItem(InPickup);

	UGameplayStatics::PlaySound2D(this, PickupSound);
}

void UDC1InteractComponent::RequestRegisterRecipe(ADC1CraftBuildActor* InCraftTarget, FName InRecipeID, int32 InCraftCount)
{
	Server_RegisterRecipe(InCraftTarget, InRecipeID, InCraftCount);
}

void UDC1InteractComponent::RequestCancelRecipe(ADC1CraftBuildActor* InCraftTarget)
{
	Server_CancelRecipe(InCraftTarget);
}

void UDC1InteractComponent::Server_CancelBuild_Implementation(ADC1InteractBuildActor* InBuildTarget)
{
	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (true == Player->IsDownOrDead())
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
		BuildDestroySubsystem->RequestDestroy(InBuildTarget, Player->GetPlayerState(), EDC1BuildRefundMode::ToInstigator);
	}
}

void UDC1InteractComponent::Server_CollectProduct_Implementation(ADC1ProductionBuildActor* InProductionTarget)
{
	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (true == Player->IsDownOrDead())
	{
		return;
	}

	if (false == ::IsValid(InProductionTarget) || false == IsInInteractRange(Player, InProductionTarget))
	{
		return;
	}

	InProductionTarget->CollectProduct(Player);
}

void UDC1InteractComponent::Server_PickupItem_Implementation(ADC1PickupItem* InPickup)
{
	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (true == Player->IsDownOrDead())
	{
		return;
	}

	if (false == ::IsValid(InPickup) || false == IsInInteractRange(Player, InPickup))
	{
		return;
	}

	// F 입력 Pickup -> InByInteract = true
	InPickup->TryPickup(Player, true);
}

void UDC1InteractComponent::Server_RegisterRecipe_Implementation(ADC1CraftBuildActor* InCraftTarget, FName InRecipeID, int32 InCraftCount)
{
	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (true == Player->IsDownOrDead())
	{
		return;
	}

	if (false == ::IsValid(InCraftTarget) || false == IsInInteractRange(Player, InCraftTarget))
	{
		return;
	}

	InCraftTarget->RegisterRecipe(Player, InRecipeID, InCraftCount);
}

void UDC1InteractComponent::Server_CancelRecipe_Implementation(ADC1CraftBuildActor* InCraftTarget)
{
	ADC1Player* Player = GetPlayer();
	if (false == ::IsValid(Player))
	{
		return;
	}

	if (true == Player->IsDownOrDead())
	{
		return;
	}

	if (false == ::IsValid(InCraftTarget) || false == IsInInteractRange(Player, InCraftTarget))
	{
		return;
	}

	InCraftTarget->CancelRecipe(Player);
}

void UDC1InteractComponent::HandleScreenStackChanged()
{
	UDC1UIManagerSubsystem* UIManager = GetUIManager();
	if (true == ::IsValid(UIManager) && true == UIManager->HasActiveScreen())
	{
		ReleaseAllHeldKeys();
	}

	if (true == IsRefreshPending)
	{
		RefreshOptions();
		return;
	}

	UpdatePromptAndIMC();
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

	// 거리는 감지 반경으로 나눠 내적과 같은 크기(0 ~ 1)로 맞춤
	const FVector PlayerLocation = Player->GetActorLocation();
	const float DetectRadius = FMath::Max(InteractDetectCollider->GetScaledSphereRadius(), 1.f);

	float BestScore = (FLT_MAX * -1.f);
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

		const float Dot = FVector::DotProduct(ViewRotation.Vector(), (OverlapLocation - ViewLocation).GetSafeNormal());
		const float DistanceRatio = FVector::Dist(PlayerLocation, OverlapLocation) / DetectRadius;

		// 바닥에 모인 픽업처럼 시선 각도가 비슷하면 가까운 쪽
		float Score = Dot - (DistanceRatio * DistanceScoreWeight);

		if (OverlapActor == CurrentTarget.Get())
		{
			Score += CurrentTargetDotBonus;
		}

		if (Score > BestScore)
		{
			BestTarget = OverlapActor;
			BestScore = Score;
		}
	}

	return BestTarget;
}

void UDC1InteractComponent::SetCurrentTarget(AActor* InNewTarget)
{
	// 대상이 파괴되면 Get()이 nullptr -> stale이면 정리를 위해 통과시킨다
	if (InNewTarget == CurrentTarget.Get() && false == CurrentTarget.IsStale())
	{
		return;
	}

	ReleaseAllHeldKeys();
	
	// 기존 타겟 정리
	IDC1InteractableInterface* PrevInteractable = GetCurrentInteractable();
	if (nullptr != PrevInteractable)
	{
		PrevInteractable->GetOnInteractOptionsChanged().Remove(OptionsChangedHandle);
		PrevInteractable->OnLocalFocusChanged(false);
	}
	
	OptionsChangedHandle.Reset();
	HidePrompt();

	CurrentTarget = InNewTarget;

	// 새로운 타겟 상호작용 표시
	IDC1InteractableInterface* CurrentInteractable = GetCurrentInteractable();
	if (nullptr != CurrentInteractable)
	{
		OptionsChangedHandle = CurrentInteractable->GetOnInteractOptionsChanged().AddUObject(this, &ThisClass::RefreshOptions);
		ShowPrompt();
		CurrentInteractable->OnLocalFocusChanged(true);
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
		// 매핑 존재는 ShowPrompt에서 확인 - Release 짝을 맞추기 위해 변환된 태그를 저장
		const FGameplayTag AbilityTag = InteractionToAbility.FindRef(Option->InteractionTag);
		PC->PressAbilityInput(AbilityTag);
		HeldAbilityTags.Add(InKey, AbilityTag);
		
		UDC1InteractPromptWidget* Prompt = PromptWidget.Get();
		if (true == ::IsValid(Prompt))
		{
			Prompt->SetKeyHighlighted(InKey, true);
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

	UDC1InteractPromptWidget* Prompt = PromptWidget.Get();
	if (true == ::IsValid(Prompt))
	{
		Prompt->SetKeyHighlighted(InKey, false);
	}

	if (true == HeldAbilityTags.IsEmpty() && true == IsRefreshPending)
	{
		RefreshOptions();
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

	UDC1ScreenOffsetWidgetComponent* PromptComp = Player->GetInteractPromptWidgetComponent();
	PromptComponent = PromptComp;

	if (true == ::IsValid(PromptComp))
	{
		// 대상마다 다르게, 기본은 InteractCollider 중심
		const FVector PromptLocation = InteractCollider->Bounds.Origin;
		
		const FVector2D PromptScreenOffset = Interactable->GetPromptScreenOffset();

		PromptComp->SetWorldLocation(PromptLocation);
		PromptComp->SetScreenOffset(PromptScreenOffset);
		// 이미 초기화된 경우에도 호출 가능한 함수
		PromptComp->InitWidget();

		PromptWidget = Cast<UDC1InteractPromptWidget>(PromptComp->GetUserWidgetObject());
	}

	RefreshOptions();
}

void UDC1InteractComponent::HidePrompt()
{
	UWidgetComponent* PromptComp = PromptComponent.Get();
	if (true == ::IsValid(PromptComp))
	{
		PromptComp->SetVisibility(false);
	}

	PromptWidget = nullptr;
	CurrentOptions.Empty();

	UpdatePromptAndIMC();

	IsRefreshPending = false;
}

void UDC1InteractComponent::RefreshOptions()
{
	IDC1InteractableInterface* Interactable = GetCurrentInteractable();
	if (false == HeldAbilityTags.IsEmpty() && nullptr != Interactable && true == Interactable->ShouldKeepOptionsWhileHeld())
	{
		IsRefreshPending = true;
		return;
	}

	IsRefreshPending = false;

	ReleaseAllHeldKeys();

	CurrentOptions.Empty();

	if (nullptr == Interactable)
	{
		UpdatePromptAndIMC();
		return;
	}

	TArray<FDC1InteractOption> RawOptions;
	Interactable->GetInteractOptions(RawOptions);

	const ADC1Player* Player = GetPlayer();
	UDC1AbilitySystemComponent* ASC = (true == ::IsValid(Player)) ? Cast<UDC1AbilitySystemComponent>(Player->GetAbilitySystemComponent()) : nullptr;

	for (const FDC1InteractOption& Raw : RawOptions)
	{
		if (Raw.InputType == EDC1InteractInputType::HoldAbility)
		{
			const FGameplayTag* AbilityTag = InteractionToAbility.Find(Raw.InteractionTag);
			if (nullptr == AbilityTag)
			{
				UE_LOG(LogTemp, Warning, TEXT("UDC1InteractComponent :: No Ability Mapped - Key : %s, InteractionTag : %s"), *UEnum::GetValueAsString(Raw.Key), *Raw.InteractionTag.ToString());
				continue;
			}

			// 플레이어가 할 수 없는 작업( 발전 · 제약 … ) 등 지금 실행 불가 - 경고 없이 숨김 ( 정보 패널은 건축물 상태라 그대로 )
			if (nullptr == ASC || false == ASC->CanActivateAbilityByTag(*AbilityTag))
			{
				continue;
			}
		}

		if (true == CurrentOptions.ContainsByPredicate([&Raw](const FDC1InteractOption& Option) {
			return Option.Key == Raw.Key;
			}))
		{
			UE_LOG(LogTemp, Warning, TEXT("UDC1InteractComponent :: Contains Key : %s"), *UEnum::GetValueAsString(Raw.Key));
			continue;
		}

		CurrentOptions.Add(Raw);
	}

	UDC1InteractPromptWidget* Prompt = PromptWidget.Get();
	if (true == ::IsValid(Prompt))
	{
		Prompt->SetOptions(CurrentOptions);
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

	UDC1InteractPromptWidget* Prompt = PromptWidget.Get();
	if (true == ::IsValid(Prompt))
	{
		Prompt->SetHoldRatio(HoldConfirmKey.GetValue(), Ratio);
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

	UWidgetComponent* PromptComp = PromptComponent.Get();
	if (true == ::IsValid(PromptComp))
	{
		PromptComp->SetVisibility(WantInteract);
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