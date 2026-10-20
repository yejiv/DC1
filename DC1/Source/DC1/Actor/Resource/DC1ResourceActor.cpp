// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Resource/DC1ResourceActor.h"
#include "Actor/Destroy/DC1DestroyActor.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Define/DC1Enums.h"
#include "GameFramework/GameStateBase.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

namespace
{
	// 고갈 후 이 시간 안에 받은 상태 변화만 "방금 부서짐"으로 보고 연출한다
	constexpr double RecentDepleteWindow = 1.0;
}

ADC1ResourceActor::ADC1ResourceActor()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 레벨 배치 전제 - 맞기 전까지 복제하지 않는다. 변화는 ForceNetUpdate로 밀어낸다
	NetDormancy = DORM_Initial;
	SetNetUpdateFrequency(10.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	// 근접 공격 콜라이더와 양쪽 다 GenerateOverlapEvents가 켜져 있어야 겹침이 성립한다
	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	ResourceMesh->SetupAttachment(SceneRoot);
	ResourceMesh->SetCollisionProfileName(DC1CollisionProfile::Resource);
	ResourceMesh->SetGenerateOverlapEvents(true);

	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	// Damage 메타 어트리뷰트가 여기 있어서 기존 데미지 GE가 그대로 먹는다
	AttributeSet = CreateDefaultSubobject<UDC1AttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADC1ResourceActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ADC1ResourceActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// BeginPlay가 아니라 여기 - 첫 OnRep이 BeginPlay보다 먼저 올 수 있고, ApplyScale이 이 값을 쓴다
	InitialMeshScale = ResourceMesh->GetRelativeScale3D();
}

void ADC1ResourceActor::BeginPlay()
{
	Super::BeginPlay();

	if (true == HasAuthority())
	{
		AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetMaxHealthAttribute(), MaxHealth);
		AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), MaxHealth);
	}

	// 클라도 Attribute 복제 시 Broadcast 된다
	HealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);

	ApplyResourceState(false);
}

void ADC1ResourceActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimer);

	if (true == ::IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
	}
	HealthChangedHandle.Reset();

	Super::EndPlay(EndPlayReason);
}

void ADC1ResourceActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ResourceState);
	DOREPLIFETIME(ThisClass, DepletedServerTime);
}

void ADC1ResourceActor::HandleHealthChanged(const FOnAttributeChangeData& InData)
{
	ApplyScale();

	// 같은 프레임 중복 타격, 리스폰 중 Health 복구는 무시
	if (false == HasAuthority() || EDC1ResourceActorState::Depleted == ResourceState)
	{
		return;
	}

	WakeForReplication();

	// TODO : 드랍 누적 += 클램프된 실제 감소량

	if (InData.NewValue <= 0.f)
	{
		SetResourceState(EDC1ResourceActorState::Depleted);
	}
}

void ADC1ResourceActor::SetResourceState(EDC1ResourceActorState InNewState)
{
	if (ResourceState == InNewState)
	{
		return;
	}

	ResourceState = InNewState;

	const bool IsDepleted = (EDC1ResourceActorState::Depleted == ResourceState);
	if (true == IsDepleted)
	{
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		DepletedServerTime = (true == ::IsValid(GameState)) ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

		GetWorldTimerManager().SetTimer(RespawnTimer, this, &ThisClass::Respawn, RespawnTime, false);
	}

	// Standalone 표시
	ApplyResourceState(IsDepleted);

	ForceNetUpdate();
}

void ADC1ResourceActor::Respawn()
{
	const float CurrentMaxHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
	AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), CurrentMaxHealth);

	SetResourceState(EDC1ResourceActorState::Active);
	SleepForReplication();
}

void ADC1ResourceActor::WakeForReplication()
{
	if (DORM_Awake == NetDormancy)
	{
		return;
	}

	SetNetDormancy(DORM_Awake);
}

void ADC1ResourceActor::SleepForReplication()
{
	SetNetDormancy(DORM_DormantAll);
}

void ADC1ResourceActor::OnRep_ResourceState()
{
	const bool IsDepleted = (EDC1ResourceActorState::Depleted == ResourceState);

	ApplyResourceState(true == IsDepleted && true == IsRecentDeplete());
}

void ADC1ResourceActor::ApplyResourceState(bool InPlayFX)
{
	if (EDC1ResourceActorState::Active == ResourceState)
	{
		ResourceMesh->SetHiddenInGame(false);
		ResourceMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		ApplyScale();

		return;
	}

	if (true == InPlayFX)
	{
		PlayDepleteFX();
	}

	ResourceMesh->SetHiddenInGame(true);
	ResourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADC1ResourceActor::ApplyScale()
{
	if (EDC1ResourceScaleType::Fixed == ScaleType)
	{
		return;
	}

	constexpr float MidThreshold = 0.7f;
	constexpr float MinThreshold = 0.4f;

	constexpr float MaxRatio = 1.0f;
	constexpr float MidRatio = 0.8f;
	constexpr float MinRatio = 0.6f;
	
	float Ratio = GetHealthRatio();
	const float Scale = (Ratio >= MidThreshold) ?	MaxRatio :
						(Ratio >= MinThreshold) ?	MidRatio : 
													MinRatio;

	ResourceMesh->SetRelativeScale3D(InitialMeshScale * Scale);
}

void ADC1ResourceActor::PlayDepleteFX()
{
	if (NM_DedicatedServer == GetNetMode() || false == ::IsValid(ResourceMesh))
	{
		return;
	}

	if (true == ::IsValid(DestroyClass) && true == ::IsValid(DestroyGeometryCollection))
	{
		const FTransform SpawnTransform = ResourceMesh->GetComponentTransform();
		ADC1DestroyActor* DestroyActor = GetWorld()->SpawnActorDeferred<ADC1DestroyActor>(DestroyClass, SpawnTransform);
		if (true == ::IsValid(DestroyActor))
		{
			DestroyActor->InitDestroy(DestroyGeometryCollection);
			DestroyActor->FinishSpawning(SpawnTransform);
		}
	}

	if (true == ::IsValid(DestroyDustFX))
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, DestroyDustFX, ResourceMesh->Bounds.Origin, GetActorRotation());
	}
}

bool ADC1ResourceActor::IsRecentDeplete() const
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (false == ::IsValid(GameState) || DepletedServerTime < 0.0)
	{
		return false;
	}

	return (GameState->GetServerWorldTimeSeconds() - DepletedServerTime) <= RecentDepleteWindow;
}

float ADC1ResourceActor::GetHealthRatio() const
{
	const float CurrentMaxHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());

	// 클라 첫 복제 전 ( Dormant 중 ) 은 0 / 0
	if (CurrentMaxHealth <= 0.f)
	{
		return 1.f;
	}

	const float CurrentHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetHealthAttribute());
	return FMath::Clamp(CurrentHealth / CurrentMaxHealth, 0.f, 1.f);
}
