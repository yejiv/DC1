// Fill out your copyright notice in the Description page of Project Settings.

#include "Component/DC1EquipComponent.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "GameFramework/PlayerState.h"
#include "Tags/DC1GamePlayTags.h"

#include "Character/Player/DC1Player.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "Item/Equip/Tool/DC1Tool.h"
#include "Item/Equip/Glider/DC1Glider.h"

#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_EquipStat.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/DC1CharacterMovementComponent.h"
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
		SyncEquipsFromInventory();
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
	DOREPLIFETIME(UDC1EquipComponent, CurrentGlider);
	DOREPLIFETIME(UDC1EquipComponent, CurrentBodyArmorID);
	DOREPLIFETIME(UDC1EquipComponent, CurrentHairEquipID);
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

	for (ADC1Tool* OldTool : ToolSlots)
	{
		if (true == IsValid(OldTool))
		{
			OldTool->Destroy();
		}
	}
	ToolSlots.Reset();
	CurrentTool = nullptr;

	for (const TSubclassOf<ADC1Tool>& ToolType : ToolClass)
	{
		if (nullptr == ToolType) continue;

		ADC1Tool* Tool = GetWorld()->SpawnActor<ADC1Tool>(ToolType, Player->GetActorTransform(), SpawnParams);
		if (false == IsValid(Tool)) continue;

		Tool->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Tool->SetActorHiddenInGame(true);

		ToolSlots.Add(Tool);
	}

	// 폰이 없던 동안 바뀐 장비 칸도 여기서 한 번에 맞춘다
	SyncEquipsFromInventory();
}

void UDC1EquipComponent::SyncEquipsFromInventory()
{
	if (false == IsValid(GetOwningPlayer())) return;

	EquipSlots.SetNum(DC1EquipSlotLayout::SlotCount);

	// 장비 칸 변경 이벤트와 같은 경로로 모든 칸을 한 번씩 맞춘다
	// 지금 든 무기 칸도 이 루프에서 HandleWeaponChanged가 들어준다 (잠긴/빈 칸은 주먹)
	for (int32 EquipSlotIndex = 0; EquipSlotIndex < DC1EquipSlotLayout::SlotCount; ++EquipSlotIndex)
	{
		HandleEquipmentSlotChanged(EquipSlotIndex);
	}
}

void UDC1EquipComponent::HandlePawnReady()
{
	// 스폰과 부착은 서버만. 권한 검사는 Init_Equip이 한다
	Init_Equip();

	// 클라 포함. PlayerState가 폰보다 먼저 도착하면 OnRep_CurrentWeapon이 폰을 못 찾고
	// 그냥 지나간다. 그때 놓친 콜라이더 적용을 여기서 만회한다
	ApplyWeaponCollider();
	OnRep_CurrentBodyArmor();
	OnRep_CurrentHairEquipArmor();
}

float UDC1EquipComponent::GetCurrentWeaponAttackPower() const
{
	const int32 EquipSlotIndex = GetCurrentWeaponEquipIndex();
	if (false == EquipSlots.IsValidIndex(EquipSlotIndex)) return 0.f;

	const FName ItemID = EquipSlots[EquipSlotIndex].ItemID;
	if (NAME_None == ItemID) return 0.f;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(ItemID);
	if (nullptr == EquipRow) return 0.f;

	return EquipRow->AttackPower;
}

float UDC1EquipComponent::GetCurrentWeaponDamageRate(const FGameplayTag& InWorkTypeTag) const
{
	const int32 EquipSlotIndex = GetCurrentWeaponEquipIndex();
	if (false == EquipSlots.IsValidIndex(EquipSlotIndex)) return 1.f;

	const FName ItemID = EquipSlots[EquipSlotIndex].ItemID;
	if (NAME_None == ItemID) return 1.f;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(ItemID);
	if (nullptr == EquipRow) return 1.f;

	if (InWorkTypeTag == DC1WorkTags::Work_Lumbering) return EquipRow->LumberingDamageRate;
	if (InWorkTypeTag == DC1WorkTags::Work_Mining) return EquipRow->MiningDamageRate;

	return 1.f;
}

void UDC1EquipComponent::SetGliderVisibility(bool IsVisiblity)
{
	if (nullptr == CurrentGlider) return;

	CurrentGlider->SetActorHiddenInGame(!IsVisiblity);
}

bool UDC1EquipComponent::CanShowWeapon() const
{
	if (nullptr != CurrentTool || true == IsPalSphereEquipped) 
		return false;

	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player))
		return false;

	UDC1CharacterMovementComponent* CMC = Cast<UDC1CharacterMovementComponent>(Player->GetCharacterMovement());
	if (false == IsValid(CMC))
		return false;

	if (true == CMC->IsWeaponBlockedByMovement())
		return false;

	return true;
}

void UDC1EquipComponent::RefreshWeaponVisibility()
{
	if (false == GetOwner()->HasAuthority())
		return;

	if (nullptr == CurrentWeapon)
		return;

	CurrentWeapon->SetActorHiddenInGame(!CanShowWeapon());
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

UAbilitySystemComponent* UDC1EquipComponent::GetOwningAbilitySystem() const
{
	const ADC1PlayerState* OwnerState = Cast<ADC1PlayerState>(GetOwner());
	if (nullptr == OwnerState) return nullptr;

	return OwnerState->GetAbilitySystemComponent();
}

void UDC1EquipComponent::HandleEquipmentSlotChanged(int32 InEquipSlotIndex)
{
	// 폰이 없으면 건너뛴다. 폰이 붙을 때 Init_Equip이 전체를 다시 맞춘다
	if (false == IsValid(GetOwningPlayer())) return;
	if (false == EquipSlots.IsValidIndex(InEquipSlotIndex)) return;

	UAbilitySystemComponent* ASC = GetOwningAbilitySystem();
	if (nullptr == ASC) return;

	// HP 비율 기록
	const float OldMaxHealth = ASC->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
	const float HealthRatio = (OldMaxHealth > 0.f) ? ASC->GetNumericAttribute(UDC1AttributeSet::GetHealthAttribute()) / OldMaxHealth : 1.f;

	// Shield 수치 기록
	const float OldMaxShield = ASC->GetNumericAttribute(UDC1PlayerSet::GetMaxShieldAttribute());

	// 모든 칸 공통: 이전 적용 해제 + ItemID 갱신. 적용은 아래 타입별 함수가 한다
	RefreshEquipSlot(InEquipSlotIndex);

	EDC1EquipSlotType SlotType = DC1EquipSlotLayout::GetSlotType(InEquipSlotIndex);

	switch (SlotType)
	{
	case EDC1EquipSlotType::Weapon:
		HandleWeaponChanged(InEquipSlotIndex);
		break;
	case EDC1EquipSlotType::Head:
	case EDC1EquipSlotType::Body:
		HandleArmorChanged(InEquipSlotIndex);
		break;
	case EDC1EquipSlotType::Shield:
		HandleShieldChanged(InEquipSlotIndex);
		break;
	case EDC1EquipSlotType::Glider:
		HandleGliderChanged(InEquipSlotIndex);
		break;
	case EDC1EquipSlotType::Accessory:
		break;
	case EDC1EquipSlotType::Food:
		break;
	}

	// 기록한 HP 비율에 따라 현재 HP 갱신
	const float NewMaxHealth = ASC->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
	if (false == FMath::IsNearlyEqual(OldMaxHealth, NewMaxHealth))
	{
		ASC->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), NewMaxHealth * HealthRatio);
	}

	// MaxShield 수치 다를 시 최대로 갱신
	const float NewMaxShield = ASC->GetNumericAttribute(UDC1PlayerSet::GetMaxShieldAttribute());
	if (false == FMath::IsNearlyEqual(OldMaxShield, NewMaxShield))
	{
		ASC->SetNumericAttributeBase(UDC1PlayerSet::GetShieldAttribute(), NewMaxShield);
	}
}

void UDC1EquipComponent::RefreshEquipSlot(int32 InEquipSlotIndex)
{
	if (false == EquipSlots.IsValidIndex(InEquipSlotIndex)) return;

	FDC1EquipSlot& EquipSlot = EquipSlots[InEquipSlotIndex];

	// 이전 적용 해제. 칸에 무엇이 들어 있는지만 보므로 타입 분기가 필요 없다
	if (ADC1Equip* OldEquip = EquipSlot.EquipActor)
	{
		// 든 무기였다면 파괴된 액터를 붙잡지 않도록 먼저 비운다
		if (OldEquip == CurrentWeapon)
		{
			CurrentWeapon = nullptr;
		}

		if (OldEquip == CurrentGlider)
		{
			CurrentGlider = nullptr;
		}
		OldEquip->Destroy();
	}
	EquipSlot.EquipActor = nullptr;

	if (UAbilitySystemComponent* ASC = GetOwningAbilitySystem())
	{
		ASC->RemoveActiveGameplayEffect(EquipSlot.StatEffectHandle);
	}
	EquipSlot.StatEffectHandle.Invalidate();

	// 인벤토리 기준으로 지금 적용할 아이템 갱신 (비었으면 NAME_None)
	EquipSlot.ItemID = NAME_None;

	if (UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent())
	{
		const FDC1InventorySlotInfo& SlotInfo = Inventory->GetEquipmentSlot(InEquipSlotIndex);
		if (false == SlotInfo.IsEmpty())
		{
			EquipSlot.ItemID = SlotInfo.ItemID;
		}
	}
}

ADC1Equip* UDC1EquipComponent::SpawnEquipActor(int32 InEquipSlotIndex)
{
	if (false == EquipSlots.IsValidIndex(InEquipSlotIndex)) return nullptr;

	FDC1EquipSlot& EquipSlot = EquipSlots[InEquipSlotIndex];
	if (NAME_None == EquipSlot.ItemID) return nullptr;

	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player)) return nullptr;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(EquipSlot.ItemID);
	if (nullptr == EquipRow) return nullptr;

	// 장착 시점 1회 로드라 동기 로드로 처리
	UClass* EquipClass = EquipRow->EquipClass.LoadSynchronous();
	if (nullptr == EquipClass)
	{
		LOG_WARNING(TEXT("[Equip] %s : EquipClass가 비어 있음"), *EquipSlot.ItemID.ToString());
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADC1Equip* Equip = GetWorld()->SpawnActor<ADC1Equip>(EquipClass, Player->GetActorTransform(), SpawnParams);
	if (false == IsValid(Equip)) return nullptr;

	// 안 든 장비도 캐릭터를 따라다녀야 원점에 남거나 관련성에서 밀려나지 않는다
	// 소켓 부착과 숨김은 타입별 적용 함수가 한다
	Equip->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

	// 타입이 맞지 않아도 칸에 저장해 둬야 다음 Refresh에서 파괴된다
	EquipSlot.EquipActor = Equip;

	return Equip;
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

int32 UDC1EquipComponent::GetCurrentWeaponEquipIndex() const
{
	return DC1EquipSlotLayout::GetEquipIndex(EDC1EquipSlotType::Weapon, CurrentWeaponIndex);
}

void UDC1EquipComponent::OnRep_CurrentWeapon()
{
	ApplyWeaponCollider();

	OnWeaponChanged.Broadcast();
}

void UDC1EquipComponent::OnRep_CurrentBodyArmor()
{
	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player))
		return;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(CurrentBodyArmorID);

	USkeletalMesh* Mesh = { nullptr };
	if (nullptr != EquipRow)
	{
		Mesh = EquipRow->EquipMesh.LoadSynchronous();
	}
	Player->Change_EquipMesh(Mesh, EDC1EquipSlotType::Body);

	OnArmorChanged.Broadcast();
}

void UDC1EquipComponent::OnRep_CurrentHairEquipArmor()
{
	ADC1Player* Player = GetOwningPlayer();
	if (false == IsValid(Player))
		return;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(CurrentHairEquipID);

	if (nullptr == EquipRow)
	{
		Player->Change_EquipMesh(nullptr, EDC1EquipSlotType::Head);
	}
	else
	{
		Player->Change_EquipMesh(EquipRow->EquipMesh.LoadSynchronous(), EDC1EquipSlotType::Head, EquipRow->AttachBoneName, EquipRow->AttachOffset);
	}

	OnArmorChanged.Broadcast();
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
	const int32 EquipSlotIndex = GetCurrentWeaponEquipIndex();
	if (false == EquipSlots.IsValidIndex(EquipSlotIndex)) return;

	SetWeaponState(WEAPONSTATE::DETACH);
	CurrentWeapon = Cast<ADC1Weapon>(EquipSlots[EquipSlotIndex].EquipActor);
	SetWeaponState(WEAPONSTATE::ATTACH);

	ApplyWeaponStateTag();
	OnRep_CurrentWeapon();
}

void UDC1EquipComponent::HandleWeaponChanged(int32 InEquipSlotIndex)
{
	if (ADC1Equip* Equip = SpawnEquipActor(InEquipSlotIndex))
	{
		// 안 든 무기는 숨겨 둔다. 표시는 SetWeaponState(ATTACH)가 한다
		Equip->SetActorHiddenInGame(true);

		if (nullptr == Cast<ADC1Weapon>(Equip))
		{
			LOG_WARNING(TEXT("[Equip] %s : 무기 칸인데 EquipClass가 ADC1Weapon이 아님"), *EquipSlots[InEquipSlotIndex].ItemID.ToString());
		}
	}

	// 지금 든 칸이 바뀌었으면 바로 교체 (비었으면 주먹)
	if (InEquipSlotIndex == GetCurrentWeaponEquipIndex())
	{
		ApplyWeaponAttach();
	}
}

void UDC1EquipComponent::HandleGliderChanged(int32 InEquipSlotIndex)
{
	if (ADC1Equip* Equip = SpawnEquipActor(InEquipSlotIndex))
	{
		Equip->SetActorHiddenInGame(true);

		if (nullptr == Cast<ADC1Glider>(Equip))
		{
			LOG_WARNING(TEXT("[Equip] %s : Glider 아닌 아이템 장착"), *EquipSlots[InEquipSlotIndex].ItemID.ToString());
		}
	}

	if (false == EquipSlots.IsValidIndex(InEquipSlotIndex)) return;

	CurrentGlider = Cast<ADC1Glider>(EquipSlots[InEquipSlotIndex].EquipActor);

	if (nullptr == CurrentGlider) return;

	if (const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(EquipSlots[InEquipSlotIndex].ItemID))
	{
		CurrentGlider->SetGlideSpeed(EquipRow->Speed);
	}

	if (ADC1Player* Player = GetOwningPlayer())
	{
		CurrentGlider->AttachToComponent(
			Player->GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			CurrentGlider->AttachSocketName
		);
	}
}

void UDC1EquipComponent::HandleArmorChanged(int32 InEquipSlotIndex)
{
	const FDC1EquipSlot& EquipSlot = EquipSlots[InEquipSlotIndex];

	EDC1EquipSlotType SlotType = DC1EquipSlotLayout::GetSlotType(InEquipSlotIndex);

	switch (SlotType)
	{
	case EDC1EquipSlotType::Head:
		CurrentHairEquipID = EquipSlot.ItemID;
		OnRep_CurrentHairEquipArmor();
		break;
	case EDC1EquipSlotType::Body:
		CurrentBodyArmorID = EquipSlot.ItemID;
		OnRep_CurrentBodyArmor();
		break;
	}

	if (NAME_None == EquipSlot.ItemID) return;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(EquipSlot.ItemID);
	if (nullptr == EquipRow) return;

	ApplyEquipStatEffect(GetOwningAbilitySystem(), InEquipSlotIndex, EquipRow->DefensePower, EquipRow->Health, 0.f);
}

void UDC1EquipComponent::HandleShieldChanged(int32 InEquipSlotIndex)
{
	const FDC1EquipSlot& EquipSlot = EquipSlots[InEquipSlotIndex];
	if (NAME_None == EquipSlot.ItemID) return;

	const FDC1EquipItemRow* EquipRow = DC1ItemHelper::FindEquipItemRow(EquipSlot.ItemID);
	if (nullptr == EquipRow) return;

	ApplyEquipStatEffect(GetOwningAbilitySystem(), InEquipSlotIndex, 0.f, 0.f, EquipRow->Health);
}

void UDC1EquipComponent::ApplyEquipStatEffect(UAbilitySystemComponent* InASC, int32 InEquipSlotIndex, float InDefensePower, float InMaxHealth, float InMaxShield)
{
	if (nullptr == InASC) return;

	FGameplayEffectSpecHandle Spec = InASC->MakeOutgoingSpec(UDC1GameplayEffect_EquipStat::StaticClass(), 1.f, InASC->MakeEffectContext());
	if (false == Spec.IsValid()) return;

	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Equip_Defense, InDefensePower);
	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Equip_Health, InMaxHealth);
	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Equip_Shield, InMaxShield);

	EquipSlots[InEquipSlotIndex].StatEffectHandle = InASC->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UDC1EquipComponent::SetWeaponState(WEAPONSTATE WeaponState)
{
	if (nullptr == CurrentWeapon) return;

	if (WeaponState == WEAPONSTATE::ATTACH)
	{
		// 소켓 부착은 항상 한다. 표시 여부는 CanShowWeapon이 정한다
		if (ADC1Player* Player = GetOwningPlayer())
		{
			CurrentWeapon->AttachToComponent(
				Player->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				CurrentWeapon->AttackSocketName
			);
		}

		RefreshWeaponVisibility();
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
	// SubTool
	if (nullptr != CurrentTool->SubToolMesh->GetSkeletalMeshAsset())
	{
		// ToolMesh들이 Item Mesh 기준 오프셋을 잡아뒀기에 KeepRelativeTransform
		CurrentTool->SubToolMesh->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, CurrentTool->SubAttachSocketName);
	}

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
