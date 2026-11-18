// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Pickup/DC1PickupItem.h"
#include "Character/Player/DC1Player.h"
#include "Character/Player/DC1PlayerController.h"
#include "Character/Player/DC1PlayerState.h"
#include "Component/Interact/DC1InteractComponent.h"
#include "Component/Inventory/Player/DC1PlayerInventoryComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Data/DataTable/DC1DataTableTypes.h"
#include "Define/DC1Enums.h"
#include "Define/DC1ItemHelper.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameMode/DC1GameMode.h"
#include "Net/UnrealNetwork.h"
#include "Tags/Player/DC1GameplayTag_Player.h"

#include "Component/DC1AbilitySystemComponent.h"

namespace
{
	// F 상호작용 범위
	constexpr float InteractRadius = 50.f;

	// 겹쳐서 스폰됐을 때 서로 밀어내는 최대 속도 - 작아야 튕겨 날아가지 않고 천천히 벌어짐
	constexpr float PickupMaxDepenetrationVelocity = 30.f;
}

ADC1PickupItem::ADC1PickupItem()
	: Super()
{
	// 낙하 중엔 위치를 복제 - 멈춘 뒤 DormantAll ( StopPhysics )
	SetReplicateMovement(true);

	// 메쉬마다 단순 콜리전이 있어야 함 ( Complex As Simple은 시뮬레이션 불가 )
	ItemMesh->SetCollisionProfileName(DC1CollisionProfile::Item);
	ItemMesh->SetGenerateOverlapEvents(true);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->SetLinearDamping(0.5f);
	ItemMesh->SetAngularDamping(2.f);

	// OnComponentSleep을 받으려면 필요
	ItemMesh->BodyInstance.bGenerateWakeEvents = true;
	ItemMesh->BodyInstance.SetMaxDepenetrationVelocity(PickupMaxDepenetrationVelocity);

	InteractCollider = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollider"));
	InteractCollider->SetupAttachment(ItemMesh);
	InteractCollider->InitSphereRadius(InteractRadius);
	InteractCollider->SetCollisionProfileName(DC1CollisionProfile::Interaction);

	// 물리 몸체에 합쳐지지 않게 - 위치만 따라감
	InteractCollider->BodyInstance.bAutoWeld = false;

	PickupOption.Key = EDC1InteractKey::F;
	PickupOption.DisplayText = NSLOCTEXT("Interact", "Pickup", "줍기");
	PickupOption.InputType = EDC1InteractInputType::Press;
}

ADC1PickupItem* ADC1PickupItem::SpawnDrop(UWorld* InWorld, FName InItemID, int32 InCount, const FVector& InLocation, bool InInteractOnly)
{
	if (false == ::IsValid(InWorld) || true == InItemID.IsNone() || InCount <= 0)
	{
		return nullptr;
	}

	// 클라에는 GameMode가 없음 == 서버 검사 
	const ADC1GameMode* GameMode = InWorld->GetAuthGameMode<ADC1GameMode>();
	if (false == ::IsValid(GameMode))
	{
		return nullptr;
	}

	const TSubclassOf<ADC1PickupItem> PickupClass = GameMode->GetPickupClass();
	if (false == ::IsValid(PickupClass))
	{
		return nullptr;
	}

	const FTransform SpawnTransform(InLocation);
	ADC1PickupItem* Pickup = InWorld->SpawnActorDeferred<ADC1PickupItem>(PickupClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (false == ::IsValid(Pickup))
	{
		return nullptr;
	}

	Pickup->InitPickup(InItemID, InCount, InInteractOnly);
	Pickup->FinishSpawning(SpawnTransform);

	return Pickup;
}

void ADC1PickupItem::TryPickup(ADC1Player* InPlayer, bool InByInteract)
{
	// 같은 프레임 두 번째 플레이어 || F 입력과 겹침이 같이 온 경우
	if (true == IsActorBeingDestroyed() || false == ::IsValid(InPlayer))
	{
		return;
	}

	// 플레이어가 기절 / 죽어있는 경우
	if (true == InPlayer->IsDownOrDead())
	{
		return;
	}

	const ADC1PlayerState* PlayerState = InPlayer->GetPlayerState<ADC1PlayerState>();
	if (false == ::IsValid(PlayerState))
	{
		return;
	}

	UDC1PlayerInventoryComponent* Inventory = PlayerState->GetInventoryComponent();
	if (false == ::IsValid(Inventory))
	{
		return;
	}

	// 충돌 줍기만 막음
	if (false == InByInteract)
	{
		if (true == IsInteractOnly || false == CanCarryWeight(Inventory))
		{
			return;
		}
	}

	// 빈 칸 있을 때만 줍기
	if (true == Inventory->AddItem(ItemID, Count))
	{
		ReleasePickup();
		// PickUp Animation 실행을 위한 GameplayCue
		if (UAbilitySystemComponent* ASC = PlayerState->GetAbilitySystemComponent())
		{
			ASC->ExecuteGameplayCue(DC1PlayerTags::GameplayCue_Player_Pickup);
		}
	}
}

UPrimitiveComponent* ADC1PickupItem::GetInteractCollider() const
{
	return InteractCollider;
}

void ADC1PickupItem::GetInteractOptions(TArray<FDC1InteractOption>& OutOptions) const
{
	OutOptions.Add(PickupOption);
}

void ADC1PickupItem::Interact(ADC1Character* InInteractOwner, EDC1InteractKey InKey)
{
	if (InKey != PickupOption.Key || false == ::IsValid(InInteractOwner))
	{
		return;
	}

	ADC1PlayerController* PC = InInteractOwner->GetController<ADC1PlayerController>();
	if (false == ::IsValid(PC))
	{
		return;
	}

	UDC1InteractComponent* InteractComp = PC->GetInteractComponent();
	if (true == ::IsValid(InteractComp))
	{
		InteractComp->RequestPickupItem(this);
	}
}

void ADC1PickupItem::BeginPlay()
{
	Super::BeginPlay();

	// 클라 + 서버 로드
	ApplyItemMesh();

	if (false == HasAuthority())
	{
		return;
	}

	// 서버
	SetLifeSpan(PickupLifeSpan);

	ItemMesh->OnComponentSleep.AddDynamic(this, &ThisClass::OnPhysicsSleep);
	GetWorldTimerManager().SetTimer(SettleTimer, this, &ThisClass::StopPhysics, PhysicsSettleTimeout, false);

	ItemMesh->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPickupOverlap);
}

void ADC1PickupItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(SettleTimer);

	Super::EndPlay(EndPlayReason);
}

void ADC1PickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ItemID);
}

void ADC1PickupItem::InitPickup(FName InItemID, int32 InCount, bool InInteractOnly)
{
	ItemID = InItemID;
	Count = InCount;
	IsInteractOnly = InInteractOnly;
}

bool ADC1PickupItem::CanCarryWeight(const UDC1PlayerInventoryComponent* InInventory) const
{
	// 귀중품은 무게 제외
	if (true == DC1ItemHelper::IsValuableItem(ItemID))
	{
		return true;
	}

	const FDC1ItemRow* Row = DC1ItemHelper::FindItemRow(ItemID);
	if (nullptr == Row)
	{
		return true;
	}

	return (InInventory->GetCurrentWeight() + Row->Weight * Count) <= InInventory->GetMaxWeight();
}

void ADC1PickupItem::OnPickupOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 현재 플레이어만 주움 -> 팰 운반 작업 시 추가 작업 필요
	TryPickup(Cast<ADC1Player>(OtherActor), false);
}

void ADC1PickupItem::OnPhysicsSleep(UPrimitiveComponent* SleepingComponent, FName BoneName)
{
	StopPhysics();
}

void ADC1PickupItem::StopPhysics()
{
	GetWorldTimerManager().ClearTimer(SettleTimer);

	if (false == ItemMesh->IsSimulatingPhysics())
	{
		return;
	}

	// 콜리전은 유지 - 다른 아이템이 위에 얹히고 플레이어 겹침
	ItemMesh->SetSimulatePhysics(false);

	// bRepPhysics = false + 최종 위치를 보낸 뒤 쉼
	ForceNetUpdate();
	SetNetDormancy(DORM_DormantAll);
}

void ADC1PickupItem::OnRep_ItemID()
{
	ApplyItemMesh();
}

void ADC1PickupItem::ApplyItemMesh()
{
	// 행이 없거나 WorldMesh가 비면 BP 기본 메쉬 유지
	const FDC1ItemRow* Row = DC1ItemHelper::FindItemRow(ItemID);
	if (nullptr == Row || true == Row->WorldMesh.IsNull())
	{
		return;
	}

	// 끊김 생길시 비동기 필요
	UStaticMesh* WorldMesh = Row->WorldMesh.LoadSynchronous();
	if (true == ::IsValid(WorldMesh))
	{
		ItemMesh->SetStaticMesh(WorldMesh);
	}
}

void ADC1PickupItem::ReleasePickup()
{
	Destroy();
}
