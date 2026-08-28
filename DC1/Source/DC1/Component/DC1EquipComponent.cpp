// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/DC1EquipComponent.h"

#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Tags/DC1GamePlayTags.h"

#include "Character/Player/DC1Player.h"
#include "Item/Equip/Weapon/DC1Weapon.h"
#include "Item/Equip/Tool/DC1Tool.h"

#include "Component/DC1AbilitySystemComponent.h"

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
}

void UDC1EquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDC1EquipComponent, CurrentWeaponIndex);
	DOREPLIFETIME(UDC1EquipComponent, CurrentWeapon);
}

// Called every frame
void UDC1EquipComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

// TODO : 인벤토리 연동 전 임시 구현. 아래 본문은 통째로 교체 예정
//        (WeaponClass 직접 스폰 -> 인벤토리의 아이템 ID 기반 스폰)
void UDC1EquipComponent::Init_Equip()
{
	AActor* Owner = GetOwner();
	if (false == IsValid(Owner) || false == Owner->HasAuthority()) return;

	ADC1Player* Player = Cast<ADC1Player>(Owner);
	if (false == IsValid(Player)) return;

	WeaponSlots.SetNum(CanWeaponEquip);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Player;
	SpawnParams.Instigator = Player;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const int32 SlotNum = FMath::Min(WeaponClass.Num(), WeaponSlots.Num());
	for (int32 Index = 0; Index < SlotNum; ++Index)
	{
		if (nullptr == WeaponClass[Index]) continue;

		ADC1Weapon* Weapon = GetWorld()->SpawnActor<ADC1Weapon>(WeaponClass[Index], Player->GetActorTransform(), SpawnParams);
		if (false == IsValid(Weapon)) continue;

		// 안 든 무기도 캐릭터를 따라다녀야 원점에 남거나 관련성에서 밀려나지 않는다
		Weapon->AttachToComponent(Player->GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
		Weapon->SetActorHiddenInGame(true);

		WeaponSlots[Index] = Weapon;
	}

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

	ApplyWeaponAttach();
}

void UDC1EquipComponent::Server_ChangeWeapon_Implementation(int32 Delta)
{
	CurrentWeaponIndex += Delta;
	if (CurrentWeaponIndex < 0) CurrentWeaponIndex = CanWeaponEquip - 1;
	else if (CurrentWeaponIndex >= CanWeaponEquip) CurrentWeaponIndex = 0;

	ApplyWeaponAttach();
}

void UDC1EquipComponent::OnRep_CurrentWeaponIndex()
{
	// UI
}

void UDC1EquipComponent::OnRep_CurrentWeapon()
{
	ApplyWeaponCollider();
}

void UDC1EquipComponent::ApplyWeaponCollider()
{
	if (ADC1Player* Player = Cast<ADC1Player>(GetOwner()))
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
	if (ADC1Player* Player = Cast<ADC1Player>(GetOwner()))
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
		// 도구 착용 중에는 무기 표시를 보류한다. UnequipTool에서 다시 꺼낸다
		if (nullptr != CurrentTool) return;

		CurrentWeapon->SetActorHiddenInGame(false);
		if (ADC1Player* Player = Cast<ADC1Player>(GetOwner()))
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
	ADC1Player* Player = Cast<ADC1Player>(GetOwner());
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


