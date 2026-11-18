// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/Pal/DC1PalPartyComponent.h"
#include "Component/Pal/DC1PalRideComponent.h"

#include "Character/Pal/DC1PalCharacter.h"
#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Controller/DC1PalAIController.h"
#include "Actor/BuildActor/Interact/PalBox/DC1PalBoxActor.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "AbilitySystem/Attribute/DC1PalAttributeSet.h"

#include "Data/DA/Pal/DC1PalData.h"
#include "Data/DA/Pal/DC1PalRegistryData.h"
#include "System/DC1AssetManager.h"
#include "Tags/Asset/DC1GameplayTag_Asset.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"
#include "Tags/Player/DC1GameplayTag_Player.h"
#include "Save/DC1SaveSubsystem.h"
#include "Subsystem/DC1ExperienceSubsystem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "NavigationSystem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

#include "Component/Pal/DC1PalBaseComponent.h"
#include "GameMode/DC1GameState.h"

namespace DC1PalParty
{
	// Possess 가 바꾼 시점을 되돌린다
	static void PossessKeepingView(APlayerController& InPC, APawn& InPawn)
	{
		const FRotator View = InPC.GetControlRotation();

		InPC.Possess(&InPawn);
		InPC.SetControlRotation(View);
		InPC.ClientSetRotation(View);
	}
}

UDC1PalPartyComponent::UDC1PalPartyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UDC1PalPartyComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDC1PalPartyComponent, Pals);
	DOREPLIFETIME(UDC1PalPartyComponent, DeployedPal);
	DOREPLIFETIME(UDC1PalPartyComponent, BoxPals);
}

void UDC1PalPartyComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	if (Pals.Num() < Capacity)
	{
		Pals.SetNum(Capacity);
	}

	if (BoxPals.Num() < BoxCapacity)
	{
		BoxPals.SetNum(BoxCapacity);
	}

	GetWorld()->GetTimerManager().SetTimer(BoxHealTimer, this, &ThisClass::HealStoredPals, BoxHealInterval, true);

	// 경험치 ( 주인 PS 의 ASC )
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		ExpFilter.AddTag(DC1PlayerTags::Event_Exp_Shared);
		ExpFilter.AddTag(DC1PlayerTags::Event_Exp_Pal);
		ExpEventHandle = ASC->AddGameplayEventTagContainerDelegate(ExpFilter,
			FGameplayEventTagMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::HandleExpEvent));
	}
}

void UDC1PalPartyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwner()))
	{
		ASC->RemoveGameplayEventTagContainerDelegate(ExpFilter, ExpEventHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UDC1PalPartyComponent::HandleExpEvent(FGameplayTag InEventTag, const FGameplayEventData* InPayload)
{
	if (nullptr == InPayload)
	{
		return;
	}

	GrantExperienceFor(FMath::RoundToInt(InPayload->EventMagnitude));
}

void UDC1PalPartyComponent::OnRep_Pals()
{
	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::OnRep_DeployedPal()
{
	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::OnRep_BoxPals()
{
	OnPalBoxChanged.Broadcast();
}

bool UDC1PalPartyComponent::HasPalAt(int32 InSlotIndex) const
{
	return Pals.IsValidIndex(InSlotIndex) && Pals[InSlotIndex].IsValidPal();
}

bool UDC1PalPartyComponent::IsPalAlive(int32 InSlotIndex) const
{
	return Pals.IsValidIndex(InSlotIndex) && Pals[InSlotIndex].IsDeployable();
}

bool UDC1PalPartyComponent::IsDeployed(int32 InSlotIndex) const
{
	if (false == ::IsValid(DeployedPal) || false == Pals.IsValidIndex(InSlotIndex))
	{
		return false;
	}

	// 슬롯을 따로 들고 있지 않는다. 액터의 InstanceID 로 목록에서 찾는다.
	// 값이 둘이면 복제 도착 순서에 따라 어긋날 수 있다
	return (Pals[InSlotIndex].PalID == DeployedPal->GetInstanceID());
}

bool UDC1PalPartyComponent::HasBase() const
{
	const ADC1GameState* GS = ADC1GameState::Get(this);
	if (false == ::IsValid(GS))
	{
		return false;
	}

	return GS->HasBase();
}

ADC1PalBoxActor* UDC1PalPartyComponent::GetBasePalBox() const
{
	const ADC1GameState* GS = ADC1GameState::Get(this);
	if (false == ::IsValid(GS))
	{
		return nullptr;
	}

	return GS->GetBasePalBox();
}

int32 UDC1PalPartyComponent::GetBaseCapacity() const
{
	UDC1PalBaseComponent* PalBase = GetBasePalComponent();
	if (false == ::IsValid(PalBase))
	{
		return 0;
	}

	return PalBase->GetBaseCapacity();
}

UDC1PalBaseComponent* UDC1PalPartyComponent::GetBasePalComponent() const
{
	ADC1PalBoxActor* PalBox = GetBasePalBox();
	if (false == ::IsValid(PalBox))
	{
		return nullptr;
	}

	return PalBox->GetPalBaseComponent();
}

void UDC1PalPartyComponent::HandleBaseChanged()
{
	// 스폰은 거점 컴포넌트가 직접 한다. 여기서는 화면만
	BroadcastChanged(EDC1PalContainer::Base);
}

bool UDC1PalPartyComponent::ReceivePal(const FDC1PalInstance& InPal)
{
	if (ROLE_Authority != GetOwnerRole() || false == InPal.IsValidPal())
	{
		return false;
	}

	// 팰 상자 -> 파티 순서로 빈칸
	for (const EDC1PalContainer Container : { EDC1PalContainer::Box, EDC1PalContainer::Party })
	{
		TArray<FDC1PalInstance>& Target = GetContainer(Container);

		const int32 EmptyIndex = Target.IndexOfByPredicate([](const FDC1PalInstance& Other)
			{
				return (false == Other.IsValidPal());
			});

		if (INDEX_NONE == EmptyIndex)
		{
			continue;
		}

		Target[EmptyIndex] = InPal;

		BroadcastChanged(Container);
		return true;
	}

	return false;
}

void UDC1PalPartyComponent::OnPostLoad()
{
	OnPalSlotChanged.Broadcast();
	OnPalBoxChanged.Broadcast();
}

void UDC1PalPartyComponent::HandlePalCaptured(const FDC1PalInstance& InPal)
{
	if (ROLE_Authority != GetOwnerRole() || false == InPal.IsValidPal())
	{
		return;
	}

	// 포획 경험치 (잡힌 팰 제외)
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (true == ::IsValid(Registry))
	{
		const UDC1PalData* PalData = (true == InPal.bIsAlpha) ? Registry->FindPalDataByTag(InPal.SpeciesTag).LoadSynchronous() : nullptr;
		const int32 Amount = Registry->GetDropExpFor(PalData, InPal.Level, InPal.bIsAlpha);

		if (UDC1ExperienceSubsystem* ExpSubsystem = GetWorld()->GetSubsystem<UDC1ExperienceSubsystem>())
		{
			ExpSubsystem->Grant(GetOwner(), DC1PlayerTags::Event_Exp_Shared_Capture, Amount);
		}
	}

	// 파티 → 상자 순서로 빈칸
	for (const EDC1PalContainer Container : { EDC1PalContainer::Party, EDC1PalContainer::Box })
	{
		TArray<FDC1PalInstance>& Target = GetContainer(Container);

		const int32 EmptyIndex = Target.IndexOfByPredicate([](const FDC1PalInstance& Pal)
			{
				return (false == Pal.IsValidPal());
			});

		if (INDEX_NONE == EmptyIndex)
		{
			continue;
		}

		Target[EmptyIndex] = InPal;

		BroadcastChanged(Container);
		return;
	}
}

void UDC1PalPartyComponent::Server_MovePal_Implementation(EDC1PalContainer InFrom, int32 InFromIndex, EDC1PalContainer InTo, int32 InToIndex)
{
	if (false == GetContainer(InFrom).IsValidIndex(InFromIndex) || false == GetContainer(InTo).IsValidIndex(InToIndex))
	{
		return;
	}

	if (InFrom == InTo && InFromIndex == InToIndex)
	{
		return;
	}

	if (false == GetContainer(InFrom)[InFromIndex].IsValidPal())
	{
		return;
	}

	const bool bBaseMove = (EDC1PalContainer::Base == InFrom || EDC1PalContainer::Base == InTo);

	// 거점이 없으면 거점 칸에 못 넣는다
	if (true == bBaseMove && false == HasBase())
	{
		return;
	}

	// 거점 칸끼리 정렬은 접기, 주인 변경 없음
	const bool bCrossBase = (true == bBaseMove && InFrom != InTo); // 거점과 다른 칸의 교환인가
	const int32 BaseIndex = (EDC1PalContainer::Base == InFrom) ? InFromIndex : InToIndex;

	UDC1PalBaseComponent* PalBase = GetBasePalComponent();

	// 거점 떠나는 팰은 Swap 전에 접음 -> 먼저 Swap하면 남의 칸으로 간 뒤라 체력이 기록되지 않음
	if (true == bCrossBase)
	{
		const FGuid LeavingID = PalBase->GetBasePals()[BaseIndex].PalID;
		if (true == LeavingID.IsValid())
		{
			PalBase->FoldBasePal(LeavingID);
		}
	}

	// 소환 중인 팰이 파티를 떠나면 회수
	const bool bDeployedLeavesParty = (EDC1PalContainer::Party == InFrom && EDC1PalContainer::Party != InTo && true == IsDeployed(InFromIndex))
		|| (EDC1PalContainer::Party != InFrom && EDC1PalContainer::Party == InTo && true == IsDeployed(InToIndex));

	if (true == bDeployedLeavesParty)
	{
		RetrieveDeployed();
	}

	Swap(GetContainer(InFrom)[InFromIndex], GetContainer(InTo)[InToIndex]);

	if (true == bCrossBase)
	{
		FDC1PalInstance& Entered = PalBase->GetBasePals()[BaseIndex];
		ADC1PlayerState* OwnerState = Cast<ADC1PlayerState>(GetOwner());
		const UGameInstance* GI = GetWorld()->GetGameInstance();
		UDC1SaveSubsystem* SaveSubsystem = (nullptr != GI) ? GI->GetSubsystem<UDC1SaveSubsystem>() : nullptr;
		if (true == Entered.IsValidPal() && nullptr != OwnerState && nullptr != SaveSubsystem)
		{
			// 거점 칸 팰의 주인 == 옮긴 사람
			Entered.OwnerSlotName = SaveSubsystem->MakeSlotName(OwnerState);
		}
	}

	// 거점 칸 교환은 전원 화면 갱신
	if (true == bBaseMove)
	{
		PalBase->RefreshBasePals();

		ADC1GameState* GS = ADC1GameState::Get(this);
		if (true == ::IsValid(GS))
		{
			GS->NotifyBaseChanged();
		}
	}

	if (EDC1PalContainer::Base != InFrom)
	{
		BroadcastChanged(InFrom);
	}

	if (EDC1PalContainer::Base != InTo && InFrom != InTo)
	{
		BroadcastChanged(InTo);
	}
}

bool UDC1PalPartyComponent::CanAcceptPal() const
{
	auto HasEmpty = [](const TArray<FDC1PalInstance>& InPals)
		{
			return InPals.ContainsByPredicate([](const FDC1PalInstance& Pal) { return (false == Pal.IsValidPal()); });
		};

	return HasEmpty(Pals) || HasEmpty(BoxPals);
}

void UDC1PalPartyComponent::HealStoredPals()
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return;
	}

	const float MaxSanity = GetDefault<UDC1PalAttributeSet>()->GetMaxSanity();

	bool bChanged = false;

	for (FDC1PalInstance& Pal : BoxPals)
	{
		if (false == Pal.IsValidPal())
		{
			continue;
		}

		const float MaxHealth = GetMaxHealthOf(Pal);
		if (Pal.CurrentHealth < MaxHealth && Registry->BoxHealRatePerSecond > 0.f)
		{
			Pal.CurrentHealth = FMath::Min(MaxHealth, Pal.CurrentHealth + (MaxHealth * Registry->BoxHealRatePerSecond * BoxHealInterval));
			bChanged = true;
		}

		// -1 = 아직 안 정함(가득)이라 건너뛴다
		if (0.f <= Pal.CurrentSanity && Pal.CurrentSanity < MaxSanity && Registry->BoxSanityHealPerSecond > 0.f)
		{
			Pal.CurrentSanity = FMath::Min(MaxSanity, Pal.CurrentSanity + (Registry->BoxSanityHealPerSecond * BoxHealInterval));
			bChanged = true;
		}
	}

	if (false == bChanged)
	{
		return;
	}

	BroadcastChanged(EDC1PalContainer::Box);
}

TArray<FDC1PalInstance>& UDC1PalPartyComponent::GetContainer(EDC1PalContainer InContainer)
{
	switch (InContainer)
	{
	case EDC1PalContainer::Box:		return BoxPals;
	case EDC1PalContainer::Base:
	{
		UDC1PalBaseComponent* PalBase = GetBasePalComponent();
		if (true == ::IsValid(PalBase))
		{
			return PalBase->GetBasePals();
		}

		static TArray<FDC1PalInstance> EmptyBasePals;
		EmptyBasePals.Reset();
		return EmptyBasePals;
	}
	default:						return Pals;
	}
}

const TArray<FDC1PalInstance>& UDC1PalPartyComponent::GetContainer(EDC1PalContainer InContainer) const
{
	return const_cast<UDC1PalPartyComponent*>(this)->GetContainer(InContainer);
}

void UDC1PalPartyComponent::BroadcastChanged(EDC1PalContainer InContainer)
{
	if (EDC1PalContainer::Party == InContainer)
	{
		OnPalSlotChanged.Broadcast();
	}
	else
	{
		OnPalBoxChanged.Broadcast();
	}
}

const FDC1PalInstance* UDC1PalPartyComponent::GetPalInfo(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	if (EDC1PalContainer::Party == InContainer)
	{
		return GetPalInfo(InSlotIndex);
	}

	const TArray<FDC1PalInstance>& Container = GetContainer(InContainer);
	return (Container.IsValidIndex(InSlotIndex) && Container[InSlotIndex].IsValidPal()) ? &Container[InSlotIndex] : nullptr;
}

float UDC1PalPartyComponent::GetPalHealth(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	if (EDC1PalContainer::Party == InContainer)
	{
		return GetPalHealth(InSlotIndex);
	}

	const FDC1PalInstance* Pal = GetPalInfo(InContainer, InSlotIndex);
	return (nullptr != Pal) ? Pal->CurrentHealth : 0.f;
}

float UDC1PalPartyComponent::GetPalMaxHealth(EDC1PalContainer InContainer, int32 InSlotIndex) const
{
	const FDC1PalInstance* Pal = GetPalInfo(InContainer, InSlotIndex);
	return (nullptr != Pal) ? GetMaxHealthOf(*Pal) : 0.f;
}

float UDC1PalPartyComponent::GetMaxHealthOf(const FDC1PalInstance& InPal)
{
	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return 0.f;
	}

	const UDC1PalData* PalData = Registry->FindPalDataByTag(InPal.SpeciesTag).LoadSynchronous();
	if (false == ::IsValid(PalData))
	{
		return 0.f;
	}

	return Registry->GetMaxHealthFor(*PalData, InPal.Level, InPal.TalentHealth, InPal.bIsAlpha);
}

void UDC1PalPartyComponent::GrantExperienceFor(int32 InAmount)
{
	if (ROLE_Authority != GetOwnerRole() || InAmount <= 0)
	{
		return;
	}

	const UDC1PalRegistryData* Registry = UDC1AssetManager::GetAssetByTag<UDC1PalRegistryData>(DC1AssetTags::Asset_Pal_Registry);
	if (false == ::IsValid(Registry))
	{
		return;
	}

	for (int32 Index = 0; Index < Pals.Num(); ++Index)
	{
		if (false == HasPalAt(Index))
		{
			continue;
		}

		// 소환 중이면 액터 기준
		if (true == IsDeployed(Index))
		{
			DeployedPal->GainExperience(InAmount);
			Pals[Index].Level = DeployedPal->GetLevel();
			Pals[Index].Experience = DeployedPal->GetExperience();
		}
		else
		{
			Registry->ApplyExperience(Pals[Index], InAmount);
		}
	}

	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::Server_ToggleDeploy_Implementation(int32 InSlotIndex)
{
	ToggleDeploy(InSlotIndex);
}

void UDC1PalPartyComponent::ToggleDeploy(int32 InSlotIndex)
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	// 꺼내둔 게 있으면 접는다
	if (::IsValid(DeployedPal))
	{
		RetrieveDeployed();
		return;
	}

	// 클라가 보낸 번호라 범위 밖일 수 있다. HasPalAt 이 IsValidIndex 로 걸러 준다
	if (false == HasPalAt(InSlotIndex))
	{
		return;
	}

	if (false == Pals[InSlotIndex].IsDeployable())
	{
		return;
	}

	FTransform SpawnTransform;
	if (false == FindDeployTransform(SpawnTransform))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Deploy: no navigable spot near the owner."));
		return;
	}

	ADC1PalCharacter* Spawned = ADC1PalCharacter::SpawnFromInstanceData(GetWorld(), Pals[InSlotIndex], SpawnTransform, Cast<ADC1PlayerState>(GetOwner()));
	if (false == ::IsValid(Spawned))
	{
		return;
	}

	Spawned->OnPalDied.AddUObject(this, &ThisClass::HandleDeployedPalDied);
	DeployedPal = Spawned;

	Spawned->BeginSummonFX();

	OnPalSlotChanged.Broadcast();
}

void UDC1PalPartyComponent::CycleSlot(int32 InDelta)
{
	if (0 == InDelta || Capacity <= 0)
	{
		return;
	}

	SelectedSlot = (SelectedSlot + InDelta + Capacity) % Capacity;

	OnPalSlotTranslated.Broadcast();
}

const FDC1PalInstance* UDC1PalPartyComponent::GetPalInfo(int32 Index) const
{
	if (false == Pals.IsValidIndex(Index))
		return nullptr;

	if (false == HasPalAt(Index))
		return nullptr;

	return &Pals[Index];
}

float UDC1PalPartyComponent::GetPalMaxHealth(int32 InSlotIndex) const
{
	return HasPalAt(InSlotIndex) ? GetMaxHealthOf(Pals[InSlotIndex]) : 0.f;
}

float UDC1PalPartyComponent::GetPalHealth(int32 InSlotIndex) const
{
	if (false == HasPalAt(InSlotIndex))
	{
		return 0.f;
	}

	if (true == IsDeployed(InSlotIndex))
	{
		return DeployedPal->GetHealthRatio() * GetPalMaxHealth(InSlotIndex);
	}

	return Pals[InSlotIndex].CurrentHealth;
}

void UDC1PalPartyComponent::RetrieveDeployed()
{
	GetWorld()->GetTimerManager().ClearTimer(DeathRetrieveTimer);

	// 접기 전에 하차
	StopRideApproach(false);
	EndRide(false);

	ADC1PalCharacter* Deployed = DeployedPal;
	DeployedPal = nullptr;

	if (false == ::IsValid(Deployed))
	{
		return;
	}

	// 파괴 전에 보냄 - 같은 액터 채널이라 클라에서도 사라지기 전에 도착 ( 잔상이 이 순간 포즈를 복사 )
	Deployed->Multicast_PlayDismissFX();

	UpdatePal(Deployed->Fold());
}

void UDC1PalPartyComponent::HandleDeployedPalDied()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	// 래그돌에서 하차
	StopRideApproach(false);
	EndRide(false);

	// UI 는 죽는 즉시 반응
	OnPalSlotChanged.Broadcast();

	GetWorld()->GetTimerManager().SetTimer(DeathRetrieveTimer, this, &ThisClass::RetrieveDeployed, DeathRetrieveDelay, false);
}

void UDC1PalPartyComponent::HandleOwnerDowned()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	// 팰은 남겨두고 AI 로 돌려보낸다
	StopRideApproach(true);
	EndRide(true);
}

void UDC1PalPartyComponent::HandleOwnerDied()
{
	if (ROLE_Authority != GetOwnerRole())
	{
		return;
	}

	// 달려오기 취소, 하차, 액터 정리까지 회수가 전부 처리한다
	RetrieveDeployed();
}

void UDC1PalPartyComponent::Server_ToggleRide_Implementation()
{
	// 올라타거나 내리는 중엔 무시
	if (::IsValid(Rider))
	{
		BeginDismount();
		return;
	}

	const ADC1PalAIController* PalController = GetDeployedPalController();
	if (nullptr != PalController && true == PalController->IsRideApproaching())
	{
		StopRideApproach(true);
		return;
	}

	BeginRideApproach();
}

void UDC1PalPartyComponent::StopRideApproach(bool InResumeBehavior)
{
	ADC1PalAIController* PalController = GetDeployedPalController();
	if (nullptr == PalController || false == PalController->IsRideApproaching())
	{
		return;
	}

	PalController->CancelRideApproach();

	if (true == InResumeBehavior)
	{
		PalController->TryStartBehavior();
	}

	Client_SetRideApproachInputBlocked(false);
}

void UDC1PalPartyComponent::Client_SetRideApproachInputBlocked_Implementation(bool InBlocked)
{
	if (ADC1PlayerController* PC = Cast<ADC1PlayerController>(GetOwnerController()))
	{
		PC->SetIMCEnabled(DC1PalTags::IMC_Ride, InBlocked);
	}
}

void UDC1PalPartyComponent::BeginRideApproach()
{
	ADC1PalCharacter* Pal = DeployedPal;
	const UDC1PalData* PalData = ::IsValid(Pal) ? Pal->GetPalData() : nullptr;
	if (nullptr == PalData || false == PalData->bRideable || Pal->GetHealthRatio() <= 0.f)
	{
		return;
	}

	if (false == Pal->GetMesh()->DoesSocketExist(PalData->RideSocketName))
	{
		UE_LOG(LogTemp, Warning, TEXT("[Pal] Ride: %s has no socket [%s]. Add it to the skeleton."),
			*GetNameSafe(Pal), *PalData->RideSocketName.ToString());
		return;
	}

	ADC1PalAIController* PalController = GetDeployedPalController();
	const APlayerController* PC = GetOwnerController();
	APawn* Player = (nullptr != PC) ? PC->GetPawn() : nullptr;
	if (nullptr == PalController || false == ::IsValid(Player))
	{
		return;
	}

	Client_SetRideApproachInputBlocked(true);

	if (false == PalController->RequestRideApproach(Player, RideAcceptanceRadius,
		FDC1OnRideApproachFinished::CreateUObject(this, &ThisClass::HandleRideApproachFinished)))
	{
		Client_SetRideApproachInputBlocked(false);
	}
}

void UDC1PalPartyComponent::HandleRideApproachFinished(bool InArrived)
{
	if (true == InArrived)
	{
		Ride();

		if (false == ::IsValid(Rider))
		{
			// 접근 때 멈춘 트리 재시작
			if (ADC1PalAIController* PalController = GetDeployedPalController())
			{
				PalController->TryStartBehavior();
			}
			Client_SetRideApproachInputBlocked(false);
		}
		return;
	}

	Client_SetRideApproachInputBlocked(false);
}

void UDC1PalPartyComponent::Ride()
{
	ADC1PalCharacter* Pal = DeployedPal;
	const UDC1PalData* PalData = ::IsValid(Pal) ? Pal->GetPalData() : nullptr;
	APlayerController* PC = GetOwnerController();
	ADC1Player* Player = (nullptr != PC) ? Cast<ADC1Player>(PC->GetPawn()) : nullptr;
	if (nullptr == PalData || false == ::IsValid(Player))
	{
		return;
	}

	// 붙이기·Possess 전에. 팀원 PossessedBy/UnPossessed 가 이걸로 탑승을 안다
	Player->SetRiddenPawn(Pal);

	// 내릴 때 다시 Possess
	RiddenPalController = Pal->GetController();

	if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
	{
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	// 콜리전 유지 (탑승 중 피격)

	const EDC1RideDirection Direction = Pal->GetRideComponent()->FindRideDirection(Player->GetActorLocation());

	Player->AttachToComponent(Pal->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PalData->RideSocketName);
	Rider = Player;

	Pal->GetRideComponent()->SetRideState(EDC1RideState::Mounting, Direction, Rider);

	// 붙인 뒤 바로 Possess
	DC1PalParty::PossessKeepingView(*PC, *Pal);

	GetWorld()->GetTimerManager().SetTimer(RideStepTimer, this, &ThisClass::FinishMount, PalData->RideMountDuration, false);
}

void UDC1PalPartyComponent::FinishMount()
{
	ADC1PalCharacter* Pal = DeployedPal;
	if (false == ::IsValid(Pal) || false == ::IsValid(Rider))
	{
		return;
	}

	// Riding 이 되면 팰이 앉는 자리에 고정
	Pal->GetRideComponent()->SetRideState(EDC1RideState::Riding, Pal->GetRideComponent()->GetRideDirection(), Rider);
}

void UDC1PalPartyComponent::BeginDismount()
{
	ADC1PalCharacter* Pal = DeployedPal;
	const UDC1PalData* PalData = ::IsValid(Pal) ? Pal->GetPalData() : nullptr;
	APlayerController* PC = GetOwnerController();
	if (nullptr == PalData || nullptr == PC || false == ::IsValid(Rider) || EDC1RideState::Riding != Pal->GetRideComponent()->GetRideState())
	{
		return;
	}

	Pal->StopJumping();

	DC1PalParty::PossessKeepingView(*PC, *Rider);

	Client_SetRideApproachInputBlocked(true);

	Pal->GetRideComponent()->SetRideState(EDC1RideState::Dismounting, PalData->RideDismountDirection, Rider);

	FTimerDelegate Finish = FTimerDelegate::CreateUObject(this, &ThisClass::EndRide, true);
	GetWorld()->GetTimerManager().SetTimer(RideStepTimer, Finish, PalData->RideDismountDuration, false);
}

void UDC1PalPartyComponent::EndRide(bool InReturnPalToAI)
{
	GetWorld()->GetTimerManager().ClearTimer(RideStepTimer);

	ACharacter* Player = Rider;
	AController* PalController = RiddenPalController;
	Rider = nullptr;
	RiddenPalController = nullptr;

	if (false == ::IsValid(Player))
	{
		return;
	}

	// 점프 키를 누른 채 내리면 떼는 입력이 팰에 안 와서 AI 가 계속 뛴다
	if (::IsValid(DeployedPal))
	{
		DeployedPal->StopJumping();

		// 다 내렸으면 옆 자리
		if (EDC1RideState::Dismounting == DeployedPal->GetRideComponent()->GetRideState())
		{
			Player->SetActorLocation(DeployedPal->GetRideComponent()->GetRideSideLocation());
		}

		DeployedPal->GetRideComponent()->SetRideState(EDC1RideState::None, DeployedPal->GetRideComponent()->GetRideDirection(), nullptr);
	}

	// Possess 뒤 떼기
	APlayerController* PC = GetOwnerController();
	if (nullptr != PC && Player != PC->GetPawn())
	{
		DC1PalParty::PossessKeepingView(*PC, *Player);
	}

	Client_SetRideApproachInputBlocked(false);

	Player->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	if (ADC1Player* RiderPlayer = Cast<ADC1Player>(Player))
	{
		RiderPlayer->SetRiddenPawn(nullptr);
	}
	Player->SetActorRotation(FRotator(0.f, Player->GetActorRotation().Yaw, 0.f));

	if (UCharacterMovementComponent* Movement = Player->GetCharacterMovement())
	{
		Movement->SetDefaultMovementMode();
	}

	if (false == ::IsValid(PalController))
	{
		return;
	}

	// 비켜 둔 AI 는 엔진이 정리 안 함
	ADC1PalCharacter* Pal = DeployedPal;
	if (true == InReturnPalToAI && ::IsValid(Pal))
	{
		// 카메라 기준 자리로 복귀
		if (ADC1PalAIController* PalAIController = Cast<ADC1PalAIController>(PalController))
		{
			PalAIController->ResetFollowAnchor();
		}

		PalController->Possess(Pal);
	}
	else
	{
		PalController->Destroy();
	}
}

APlayerController* UDC1PalPartyComponent::GetOwnerController() const
{
	const APlayerState* OwnerState = Cast<APlayerState>(GetOwner());
	return (nullptr != OwnerState) ? OwnerState->GetPlayerController() : nullptr;
}

ADC1PalAIController* UDC1PalPartyComponent::GetDeployedPalController() const
{
	return ::IsValid(DeployedPal) ? Cast<ADC1PalAIController>(DeployedPal->GetController()) : nullptr;
}

void UDC1PalPartyComponent::UpdatePal(const FDC1PalInstance& InPal)
{
	// 거점 팰 접기는 거점 컴포넌트가
	for (const EDC1PalContainer Container : { EDC1PalContainer::Party, EDC1PalContainer::Box })
	{
		TArray<FDC1PalInstance>& Target = GetContainer(Container);

		const int32 Index = Target.IndexOfByPredicate([&InPal](const FDC1PalInstance& Pal)
			{
				return (Pal.PalID == InPal.PalID);
			});

		if (INDEX_NONE != Index)
		{
			Target[Index] = InPal;
			BroadcastChanged(Container);
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[Pal] UpdatePal: no entry for ID %s."), *InPal.PalID.ToString(EGuidFormats::Digits));
}

bool UDC1PalPartyComponent::FindDeployTransform(FTransform& OutTransform) const
{
	const APlayerState* OwnerState = Cast<APlayerState>(GetOwner());
	const APawn* OwnerPawn = (nullptr != OwnerState) ? OwnerState->GetPawn() : nullptr;
	if (false == ::IsValid(OwnerPawn))
	{
		return false;
	}

	const FVector Ideal = OwnerPawn->GetActorLocation() + (OwnerPawn->GetActorForwardVector() * DeployDistance);

	UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
	if (nullptr == NavSystem)
	{
		return false;
	}

	FNavLocation Projected;
	if (false == NavSystem->ProjectPointToNavigation(Ideal, Projected, FVector(DeployDistance)))
	{
		return false;
	}

	OutTransform = FTransform(Projected.Location + FVector(0.f, 0.f, OwnerPawn->GetSimpleCollisionHalfHeight()));

	return true;
}
