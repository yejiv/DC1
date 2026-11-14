#include "Component/Player/DC1SatietyComponent.h"

#include "AbilitySystemGlobals.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystem/Attribute/DC1PlayerSet.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Component/Inventory/DC1InventoryTypes.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1ItemHelper.h"
#include "Tags/Player/DC1GameplayTag_Player.h"
#include "Tags/State/DC1GameplayTag_State.h"

UDC1SatietyComponent::UDC1SatietyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	// Server_EatFood 용
	SetIsReplicatedByDefault(true);
}

void UDC1SatietyComponent::BeginPlay()
{
	Super::BeginPlay();

	// 서버만 바인딩
	if (false == GetOwner()->HasAuthority())
	{
		return;
	}

	UDC1AbilitySystemComponent* ASC = GetDC1AbilitySystemComponent();
	if (true == ::IsValid(ASC))
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetSatietyAttribute()).AddUObject(this, &ThisClass::HandleSatietyChanged);
	}

	// 음식 자동 섭취 용 Equipslot 바인딩
	UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent();
	if (true == ::IsValid(Inventory))
	{
		Inventory->OnEquipmentSlotChanged.AddUObject(this, &ThisClass::HandleEquipmentSlotChanged);
	}
}

void UDC1SatietyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UDC1AbilitySystemComponent* ASC = GetDC1AbilitySystemComponent();
	if (true == ::IsValid(ASC))
	{
		ASC->GetGameplayAttributeValueChangeDelegate(UDC1PlayerSet::GetSatietyAttribute()).RemoveAll(this);
	}

	UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent();
	if (true == ::IsValid(Inventory))
	{
		Inventory->OnEquipmentSlotChanged.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UDC1SatietyComponent::RequestEatFood(int32 InSlotIndex)
{
	const UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent();
	if (false == ::IsValid(Inventory))
	{
		return;
	}
	// 클라 검사
	if (false == CanEatFood(Inventory->GetSlot(InSlotIndex).ItemID))
	{
		return;
	}

	Server_EatFood(InSlotIndex);
}

bool UDC1SatietyComponent::CanEatFood(FName InItemID) const
{
	// DT_FoodItem 확인
	if (nullptr == DC1ItemHelper::FindFoodItemRow(InItemID))
	{
		return false;
	}

	const UDC1AbilitySystemComponent* ASC = GetDC1AbilitySystemComponent();
	if (false == ::IsValid(ASC))
	{
		return false;
	}
	// 배부름 수치 풀이면 못먹음
	return ASC->GetNumericAttribute(UDC1PlayerSet::GetSatietyAttribute()) < ASC->GetNumericAttribute(UDC1PlayerSet::GetMaxSatietyAttribute());
}

UDC1PlayerInventoryComponent* UDC1SatietyComponent::GetInventoryComponent() const
{
	const ADC1PlayerState* PS = Cast<ADC1PlayerState>(GetOwner());
	return (nullptr != PS) ? PS->GetInventoryComponent() : nullptr;
}

UDC1AbilitySystemComponent* UDC1SatietyComponent::GetDC1AbilitySystemComponent() const
{
	const ADC1PlayerState* PS = Cast<ADC1PlayerState>(GetOwner());
	return (nullptr != PS) ? Cast<UDC1AbilitySystemComponent>(PS->GetAbilitySystemComponent()) : nullptr;
}

// _Implementation 이전 서버 검사
bool UDC1SatietyComponent::Server_EatFood_Validate(int32 InSlotIndex)
{
	return 0 <= InSlotIndex;
}

void UDC1SatietyComponent::Server_EatFood_Implementation(int32 InSlotIndex)
{
	// 서버 검사
	UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent();
	if (false == ::IsValid(Inventory))
	{
		return;
	}

	const FName ItemID = Inventory->GetSlot(InSlotIndex).ItemID;

	if (false == CanEatFood(ItemID))
	{
		return;
	}

	if (false == TryEatFood(ItemID))
	{
		return;
	}

	Inventory->RemoveItem(InSlotIndex);
}

bool UDC1SatietyComponent::TryEatFood(FName InItemID)
{
	const FDC1FoodItemRow* FoodRow = DC1ItemHelper::FindFoodItemRow(InItemID);
	UDC1AbilitySystemComponent* ASC = GetDC1AbilitySystemComponent();
	if (nullptr == FoodRow || false == ::IsValid(ASC))
	{
		return false;
	}

	FGameplayEventData Payload;
	Payload.EventMagnitude = FoodRow->SatietyAmount;

	return 0 < ASC->HandleGameplayEvent(DC1PlayerTags::Event_Player_Eat, &Payload);
}

void UDC1SatietyComponent::HandleSatietyChanged(const FOnAttributeChangeData& InData)
{
	UpdateStarvingState(InData.NewValue);

	// UpdateStarvingState로 굶주림 판정 후 자동 섭취 시도
	TryAutoEatFromPouch();
}

void UDC1SatietyComponent::HandleEquipmentSlotChanged(int32 InEquipSlotIndex)
{
	// 바뀐 슬롯이 음식 슬롯이면 자동 섭취 시도
	if (EDC1EquipSlotType::Food != DC1EquipSlotLayout::GetSlotType(InEquipSlotIndex))
	{
		return;
	}

	TryAutoEatFromPouch();
}

void UDC1SatietyComponent::TryAutoEatFromPouch()
{
	if (true == bAutoEating)
	{
		return;
	}

	// 밑의 TryEatFood, RemoveEquipmentItem 2가지 함수 호출로 인해 다시 TryAutoEatFromPouch로 들어옴 ( TryEatFood -> HandleSatietyChanged, RemoveEquipmentItem -> HandleEquipmentSlotChanged )
	// 재진입 방지 위한 가드
	TGuardValue<bool> AutoEatGuard(bAutoEating, true);

	UDC1AbilitySystemComponent* ASC = GetDC1AbilitySystemComponent();
	UDC1PlayerInventoryComponent* Inventory = GetInventoryComponent();
	if (false == ::IsValid(ASC) || false == ::IsValid(Inventory))
	{
		return;
	}

	// 기준 이상이면 return
	const float Satiety = ASC->GetNumericAttribute(UDC1PlayerSet::GetSatietyAttribute());
	const float MaxSatiety = ASC->GetNumericAttribute(UDC1PlayerSet::GetMaxSatietyAttribute());
	if (Satiety > MaxSatiety * AutoEatRatio)
	{
		return;
	}

	// 앞 칸 부터 먹기
	const int32 UnlockedFoodCount = Inventory->GetUnlockedSlotCount(EDC1EquipSlotType::Food);
	for (int32 LocalIndex = 0; LocalIndex < UnlockedFoodCount; ++LocalIndex)
	{
		const int32 EquipIndex = DC1EquipSlotLayout::GetEquipIndex(EDC1EquipSlotType::Food, LocalIndex);
		const FName ItemID = Inventory->GetEquipmentSlot(EquipIndex).ItemID;

		if (false == CanEatFood(ItemID))
		{
			continue;
		}

		if (false == TryEatFood(ItemID))
		{
			return;
		}

		Inventory->RemoveEquipmentItem(EquipIndex, 1);
		return;
	}
}

void UDC1SatietyComponent::UpdateStarvingState(float InSatiety)
{
	UDC1AbilitySystemComponent* ASC = GetDC1AbilitySystemComponent();
	if (false == ::IsValid(ASC))
	{
		return;
	}

	// 포만감 0 이상 Starving 태그 제거
	if (InSatiety > 0.f)
	{
		ASC->SetExclusiveStateTag(DC1StateTags::State_Satiety_Starving, FGameplayTag());
		return;
	}

	if (true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Satiety_Starving))
	{
		return;
	}

	// 포만감 0일때 Starving 태그 부여
	ASC->SetExclusiveStateTag(DC1StateTags::State_Satiety_Starving, DC1StateTags::State_Satiety_Starving);
}