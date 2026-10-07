// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/DC1EquipComponent.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Tags/DC1GamePlayTags.h"

#include "Character/Player/DC1Player.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "Item/Equip/Tool/DC1Tool.h"

#include "Component/DC1AbilitySystemComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Character/Player/DC1PlayerState.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"
#include "Define/DC1Macro.h"

// Sets default values for this component's properties
UDC1EquipComponent::UDC1EquipComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	SetIsReplicatedByDefault(true);
}

// Called when the game starts
void UDC1EquipComponent::BeginPlay()
{
	Super::BeginPlay();

	// 장비 칸 변경은 서버에서만 실제 액터 교체로 이어진다
	if (true == GetOwner()->HasAuthority())
	{
		if (UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent())
		{
			Inventory->OnEquipmentSlotChanged.AddUObject(this, &ThisClass::HandleEquipmentSlotChanged);
		}

		// 구독 전에 인벤토리가 먼저 장착했을 수 있으므로 폰이 있으면 한 번 맞춘다
		SyncWeaponsFromInventory();
	}
}

void UDC1EquipComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent())
	{
		Inventory->OnEquipmentSlotChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UDC1EquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDC1EquipComponent, CurrentWeapon);
}

// Called every frame
void UDC1EquipComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UDC1EquipComponent::Init_Equip()
{
	AActor* Owner = GetOwner();
	if (false == IsValid(Owner) || false == Owner->HasAuthority()) return;

	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player)) return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ToolSlots.Reset();

	for (const TSubclassOf<ADC1Tool>& ToolType : ToolClass)
	{
		if (nullptr == ToolType) continue;

		ADC1Tool* Tool = GetWorld()->SpawnActor<ADC1Tool>(ToolType, Player->GetActorTransform(), SpawnParams);
		if (false == IsValid(Tool)) continue;

		Tool->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Tool->SetActorHiddenInGame(true);

		ToolSlots.Add(Tool);
	}

	// 폰이 없던 동안 바뀐 무기 칸도 여기서 한 번에 맞춘다
	SyncWeaponsFromInventory();
}

void UDC1EquipComponent::SyncWeaponsFromInventory()
{
	if (false == IsValid(GetOwningPlayer())) return;

	// 잠긴 칸 포함 전체 무기 칸 수만큼 (잠긴 칸은 비어 있어 주먹)
	WeaponSlots.SetNum(DC1EquipSlotLayout::GetTypeSlotCount(EDC1EquipSlotType::Weapon));
	for (int32 Index = 0; Index < WeaponSlots.Num(); ++Index)
	{
		RefreshWeaponSlot(Index);
	}

	ApplyWeaponAttach();
}

void UDC1EquipComponent::HandlePawnReady()
{
	// 스폰과 부착은 서버만. 권한 검사는 Init_Equip이 한다
	Init_Equip();

	// 클라 포함. PlayerState가 폰보다 먼저 도착하면 OnRep_CurrentWeapon이 폰을 못 찾고
	// 그냥 지나간다. 그때 놓친 콜라이더 적용을 여기서 만회한다
	ApplyWeaponCollider();
}

ADC1Player* UDC1EquipComponent::GetOwningPlayer() const
{
	const APlayerState* OwnerState = Cast<APlayerState>(GetOwner());
	if (nullptr == OwnerState) return nullptr;

	return Cast<ADC1Player>(OwnerState->GetPawn());
}

UDC1PlayerInventoryComponent* UDC1EquipComponent::GetInventoryComponent() const
{
	const ADC1PlayerState* OwnerState = Cast<ADC1PlayerState>(GetOwner());
	if (nullptr == OwnerState) return nullptr;

	return OwnerState->GetInventoryComponent();
}

void UDC1EquipComponent::HandleEquipmentSlotChanged(int32 InEquipSlotIndex)
{
	// TODO : 방어구/장신구/음식 칸은 ADC1Equip 파생 클래스가 생기면 여기서 분기
	if (EDC1EquipSlotType::Weapon != DC1EquipSlotLayout::GetSlotType(InEquipSlotIndex)) return;

	// 폰이 없으면 건너뛴다. 폰이 붙을 때 Init_Equip이 전체를 다시 맞춘다
	if (false == IsValid(GetOwningPlayer())) return;

	const int32 WeaponIndex = DC1EquipSlotLayout::GetLocalIndex(InEquipSlotIndex);
	RefreshWeaponSlot(WeaponIndex);

	// 지금 든 칸이 바뀌었으면 바로 교체 (비었으면 주먹)
	if (WeaponIndex == CurrentWeaponIndex)
	{
		ApplyWeaponAttach();
	}
}

void UDC1EquipComponent::RefreshWeaponSlot(int32 InWeaponIndex)
{
	if (false == WeaponSlots.IsValidIndex(InWeaponIndex)) return;

	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player)) return;

	// 든 무기였다면 파괴된 액터를 붙잡지 않도록 먼저 비운다
	if (ADC1Weapon* OldWeapon = WeaponSlots[InWeaponIndex])
	{
		if (OldWeapon == CurrentWeapon)
		{
			CurrentWeapon = nullptr;
		}
		OldWeapon->Destroy();
		WeaponSlots[InWeaponIndex] = nullptr;
	}

	UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent();
	if (nullptr == Inventory) return;

	const int32 EquipSlotIndex = DC1EquipSlotLayout::GetEquipIndex(EDC1EquipSlotType::Weapon, InWeaponIndex);
	const FDC1InventorySlotInfo& SlotInfo = Inventory->GetEquipmentSlot(EquipSlotIndex);
	if (true == SlotInfo.IsEmpty()) return;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(SlotInfo.ItemID);
	if (nullptr == EquipRow) return;

	// 장착 시점 1회 로드라 동기 로드로 처리
	UClass* EquipClass = EquipRow->EquipClass.LoadSynchronous();
	if (nullptr == EquipClass || false == EquipClass->IsChildOf(ADC1Weapon::StaticClass()))
	{
		LOG_WARNING(TEXT("[Equip] %s : EquipClass가 비어 있거나 ADC1Weapon이 아님"), *SlotInfo.ItemID.ToString());
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADC1Weapon* Weapon = GetWorld()->SpawnActor<ADC1Weapon>(EquipClass, Player->GetActorTransform(), SpawnParams);
	if (false == IsValid(Weapon)) return;

	// 안 든 무기도 캐릭터를 따라다녀야 원점에 남거나 관련성에서 밀려나지 않는다
	Weapon->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	Weapon->SetActorHiddenInGame(true);

	WeaponSlots[InWeaponIndex] = Weapon;
}

void UDC1EquipComponent::Server_ChangeWeapon_Implementation(int32 Delta)
{
	const int32 WeaponCount = GetUnlockedWeaponCount();
	if (WeaponCount <= 0) return;

	CurrentWeaponIndex += Delta;
	if (CurrentWeaponIndex < 0) CurrentWeaponIndex = WeaponCount - 1;
	else if (CurrentWeaponIndex >= WeaponCount) CurrentWeaponIndex = 0;

	ApplyWeaponAttach();
}

int32 UDC1EquipComponent::GetUnlockedWeaponCount() const
{
	if (const UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent())
	{
		return Inventory->GetUnlockedSlotCount(EDC1EquipSlotType::Weapon);
	}

	return DC1EquipSlotLayout::DefaultUnlockedWeaponCount;
}

void UDC1EquipComponent::OnRep_CurrentWeapon()
{
	ApplyWeaponCollider();

	OnWeaponChanged.Broadcast();
}

void UDC1EquipComponent::ApplyWeaponCollider()
{
	if (ADC1Player* Player = GetOwningPlayer())
	{
		if (nullptr == CurrentWeapon)
		{
			Player->Change_AttackCollider(nullptr, false);
		}
		else
		{
			Player->Change_AttackCollider(CurrentWeapon->GetAttackCollider(), true);
		}
	}
}

void UDC1EquipComponent::ApplyWeaponStateTag()
{
	if (ADC1Player* Player = GetOwningPlayer())
	{
		if (UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(Player->GetAbilitySystemComponent()))
		{
			FGameplayTag WeaponStateTag = (nullptr == CurrentWeapon) ? DC1StateTags::State_Weapon_None : CurrentWeapon->WeaponStateTag;
			ASC->SetExclusiveStateTag(DC1StateTags::State_Weapon, WeaponStateTag);
		}
	}
}

void UDC1EquipComponent::ApplyWeaponAttach()
{
	// 폰이 없어 Init_Equip이 아직 못 돌았으면 슬롯이 비어 있다
	if (false == WeaponSlots.IsValidIndex(CurrentWeaponIndex)) return;

	SetWeaponState(WEAPONSTATE::DETACH);
	CurrentWeapon = WeaponSlots[CurrentWeaponIndex];
	SetWeaponState(WEAPONSTATE::ATTACH);

	ApplyWeaponStateTag();
	OnRep_CurrentWeapon();
}

void UDC1EquipComponent::SetWeaponState(WEAPONSTATE WeaponState)
{
	if (nullptr == CurrentWeapon) return;

	if (WeaponState == WEAPONSTATE::ATTACH)
	{
		// 도구/팰스피어 착용 중에는 무기 표시를 보류한다. 각 Unequip에서 다시 꺼낸다
		if (nullptr != CurrentTool || true == IsPalSphereEquipped) return;

		CurrentWeapon->SetActorHiddenInGame(false);
		if (ADC1Player* Player = GetOwningPlayer())
		{
			CurrentWeapon->AttachToComponent(
				Player->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				CurrentWeapon->AttackSocketName
			);
		}
	}
	else
	{
		CurrentWeapon->SetActorHiddenInGame(true);
	}
}

void UDC1EquipComponent::EquipTool(const FGameplayTag& ToolTag)
{
	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player) || false == Player->HasAuthority()) return;

	ADC1Tool* Tool = nullptr;
	for (const TObjectPtr<ADC1Tool>& Candidate : ToolSlots)
	{
		if (true == IsValid(Candidate) && Candidate->ToolTag == ToolTag)
		{
			Tool = Candidate;
			break;
		}
	}

	if (nullptr == Tool) return;

	SetWeaponState(WEAPONSTATE::DETACH);

	CurrentTool = Tool;
	CurrentTool->AttachToComponent(
		Player->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		CurrentTool->AttachSocketName
	);
	CurrentTool->SetActorHiddenInGame(false);
}

void UDC1EquipComponent::UnequipTool(const FGameplayTag& ToolTag)
{
	AActor* Owner = GetOwner();
	if (false == IsValid(Owner) || false == Owner->HasAuthority()) return;

	// 다른 GA가 착용시킨 도구를 벗기지 않도록 요청자와 일치할 때만 처리한다
	if (nullptr == CurrentTool || CurrentTool->ToolTag != ToolTag) return;

	CurrentTool->SetActorHiddenInGame(true);
	CurrentTool = nullptr;

	SetWeaponState(WEAPONSTATE::ATTACH);
}

void UDC1EquipComponent::EquipPalSphere()
{
	AActor* Owner = GetOwner();
	if (false == IsValid(Owner) || false == Owner->HasAuthority()) return;

	IsPalSphereEquipped = true;

	SetWeaponState(WEAPONSTATE::DETACH);
}

void UDC1EquipComponent::UnequipPalSphere()
{
	AActor* Owner = GetOwner();
	if (false == IsValid(Owner) || false == Owner->HasAuthority()) return;

	if (false == IsPalSphereEquipped) return;

	IsPalSphereEquipped = false;

	SetWeaponState(WEAPONSTATE::ATTACH);
}
