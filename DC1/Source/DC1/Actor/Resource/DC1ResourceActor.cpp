// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/Resource/DC1ResourceActor.h"
#include "Actor/Destroy/DC1DestroyActor.h"
#include "AbilitySystem/Effects/DC1GameplayEffect_Damage.h"
#include "AbilitySystem/Attribute/DC1AttributeSet.h"
#include "Component/DC1AbilitySystemComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Define/DC1Enums.h"
#include "GameplayEffectExtension.h"
#include "GameFramework/GameStateBase.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "Item/Pickup/DC1PickupItem.h"
#include "Character/Pal/DC1PalCharacter.h"
#include "Tags/Effect/DC1GameplayTag_Effect.h"
#include "Tags/Pal/DC1GameplayTag_Pal.h"
#include "Tags/State/DC1GameplayTag_State.h"
#include "NiagaraFunctionLibrary.h"
#include "Net/UnrealNetwork.h"

// TEMP v3 - DebugAssignNearbyPals ( ST 완성 후 함께 삭제 )
#include "EngineUtils.h"

namespace
{
	// 고갈 후 이 시간 안에 받은 상태 변화만 방금 부서짐으로 보고 연출
	constexpr double RecentDepleteWindow = 1.0;

	// 한 번에 여러 개 스폰할 때 위로 쌓는 간격
	constexpr float DropStackSpacing = 20.f;

	// 맞은 지점에서 표면 바깥으로 띄우는 거리 - 픽업 크기 정도. 노드 콜리전에 박혀 스폰되지 않게
	constexpr float DropSurfaceOffset = 15.f;
}

ADC1ResourceActor::ADC1ResourceActor()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	// 레벨 배치 전제 - 맞기 전까지 복제하지 않음
	NetDormancy = DORM_Initial;
	SetNetUpdateFrequency(10.f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	ResourceMesh->SetupAttachment(SceneRoot);
	ResourceMesh->SetCollisionProfileName(DC1CollisionProfile::Resource);
	ResourceMesh->SetGenerateOverlapEvents(true);

	AbilitySystemComponent = CreateDefaultSubobject<UDC1AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UDC1AttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ADC1ResourceActor::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

FGameplayTag ADC1ResourceActor::GetWorkTypeTag() const
{
	// Active에서만 작업 가능한 타입 반환
	return (EDC1ResourceActorState::Active == ResourceState) ? WorkTypeTag : FGameplayTag::EmptyTag;
}

bool ADC1ResourceActor::AssignPalWorker(ADC1PalCharacter* InPal)
{
	if (false == HasAuthority() || false == ::IsValid(InPal))
	{
		return false;
	}

	// 파괴된 펠이 자리를 영구히 먹지 않게 갱신
	PalWorkers.RemoveAll([](const TWeakObjectPtr<ADC1PalCharacter>& InWorker) { return false == InWorker.IsValid(); });

	if (true == IsPalWorkerAssigned(InPal))
	{
		return true;
	}

	if (EDC1ResourceActorState::Active != ResourceState || false == DC1WorkTarget::CanPalWorkAt(InPal, WorkTypeTag, GetActorLocation()))
	{
		return false;
	}

	PalWorkers.Add(InPal);
	OnPalWorkersChanged.Broadcast();

	return true;
}

void ADC1ResourceActor::UnassignPalWorker(ADC1PalCharacter* InPal)
{
	if (PalWorkers.Remove(InPal) > 0)
	{
		OnPalWorkersChanged.Broadcast();
	}
}

bool ADC1ResourceActor::IsPalWorkerAssigned(const ADC1PalCharacter* InPal) const
{
	return PalWorkers.Contains(InPal);
}

void ADC1ResourceActor::ApplyWorkHit(const ADC1Character* InWorker, float InMultiplier)
{
	// 고갈된 프레임의 남은 타격 return
	if (false == HasAuthority() || EDC1ResourceActorState::Active != ResourceState)
	{
		return;
	}

	// 배정된 펠만
	const ADC1PalCharacter* Pal = Cast<ADC1PalCharacter>(InWorker);
	if (false == ::IsValid(Pal) || false == IsPalWorkerAssigned(Pal))
	{
		return;
	}

	const float WorkerSpeed = DC1WorkTarget::GetWorkerSpeed(Pal, WorkTypeTag);
	const float Damage = WorkerSpeed * InMultiplier;
	if (Damage <= 0.f)
	{
		return;
	}

	UAbilitySystemComponent* SourceASC = Pal->GetAbilitySystemComponent();
	if (false == ::IsValid(SourceASC))
	{
		return;
	}

	// GetDropOrigin이 펠 쪽 표면에 드랍
	const FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = SourceASC->MakeOutgoingSpec(UDC1GameplayEffect_Damage::StaticClass(), 1.f, Context);
	if (false == Spec.IsValid())
	{
		return;
	}

	// 체력 감소는 AttributeSet의 PostGameplayEffectExecute가
	Spec.Data->SetSetByCallerMagnitude(DC1EffectTags::SetByCaller_Damage, Damage);

	const float OldHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetHealthAttribute());
	SourceASC->ApplyGameplayEffectSpecToTarget(*Spec.Data.Get(), AbilitySystemComponent);
	const float NewHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetHealthAttribute());

	UE_LOG(LogTemp, Log, TEXT("[DC1] %s work hit : %s speed %.1f x %.2f = %.1f, HP %.1f -> %.1f"), *GetName(), *GetNameSafe(Pal), WorkerSpeed, InMultiplier, Damage, OldHealth, NewHealth);
}

void ADC1ResourceActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	// 첫 OnRep이 BeginPlay보다 먼저 올 수 있음
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

	HealthChangedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).AddUObject(this, &ThisClass::HandleHealthChanged);

	// 서버만 - Health 콜백엔 GE 컨텍스트가 없음 ( PostGameplayEffectExecute의 SetHealth라서 GEModData == nullptr )
	if (true == HasAuthority())
	{
		DamageAppliedHandle = AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetDamageAttribute()).AddUObject(this, &ThisClass::HandleDamageApplied);
	}

	ApplyResourceState(false);

	// TEMP v3 - 3초마다 근처 거점 펠 배정 + GA 실행 ( 리스폰 후 재배정까지 확인 )
	if (true == HasAuthority() && DebugAssignRadius > 0.f)
	{
		GetWorldTimerManager().SetTimer(DebugAssignTimer, this, &ThisClass::DebugAssignNearbyPals, 3.f, true);
	}
}

void ADC1ResourceActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(RespawnTimer);
	GetWorldTimerManager().ClearTimer(DebugAssignTimer);

	if (true == ::IsValid(AbilitySystemComponent))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UDC1AttributeSet::GetDamageAttribute()).Remove(DamageAppliedHandle);
	}
	
	HealthChangedHandle.Reset();
	DamageAppliedHandle.Reset();

	ClearPalWorkers();

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

	// Damage 콜백이 먼저 계산해 둔 위치 ( 같은 GE 안에서 Damage → PostGameplayEffectExecute → Health 순 )
	const FVector DropOrigin = (true == HasPendingDropOrigin) ? PendingDropOrigin : GetDropOrigin(InData);
	HasPendingDropOrigin = false;

	AccumulateDrop(InData.OldValue - InData.NewValue, DropOrigin);

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

		// 배정 전원 해제 → 펠 Work GA 종료
		ClearPalWorkers();
	}

	// Standalone 표시
	ApplyResourceState(IsDepleted);

	ForceNetUpdate();
}

void ADC1ResourceActor::Respawn()
{
	AccumulatedDropDamage = 0.f;

	const float CurrentMaxHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetMaxHealthAttribute());
	AbilitySystemComponent->SetNumericAttributeBase(UDC1AttributeSet::GetHealthAttribute(), CurrentMaxHealth);

	SetResourceState(EDC1ResourceActorState::Active);
	SleepForReplication();
}

void ADC1ResourceActor::HandleDamageApplied(const FOnAttributeChangeData& InData)
{
	// PostGameplayEffectExecute - SetDamage(0) 재호출 무시
	if (InData.NewValue <= 0.f || nullptr == InData.GEModData)
	{
		return;
	}

	PendingDropOrigin = GetDropOrigin(InData);
	HasPendingDropOrigin = true;
}

FVector ADC1ResourceActor::GetDropOrigin(const FOnAttributeChangeData& InData) const
{
	const FBoxSphereBounds& Bounds = ResourceMesh->Bounds;
	const FVector Fallback = Bounds.Origin + FVector::UpVector * Bounds.BoxExtent.Z;

	if (nullptr == InData.GEModData)
	{
		return Fallback;
	}

	const FGameplayEffectContextHandle& Context = InData.GEModData->EffectSpec.GetContext();

	// 히트를 싣는 공격이 Resource를 맞힌 히트면 그 자리
	const FHitResult* HitResult = Context.GetHitResult();
	if (nullptr != HitResult && this == HitResult->GetActor() && false == HitResult->bStartPenetrating)
	{
		return HitResult->ImpactPoint + HitResult->ImpactNormal * DropSurfaceOffset;
	}

	const AActor* Causer = Context.GetEffectCauser();
	if (false == ::IsValid(Causer))
	{
		Causer = Context.GetInstigator();
	}

	if (false == ::IsValid(Causer))
	{
		return Fallback;
	}

	// 이 Resource 콜리전만 트레이스 - 공격자를 향한 표면
	// GetClosestPointOnCollision은 볼록 도형만 지원해 복합 콜리전 노드에서 실패 -> LineTraceComponent로 변경
	FHitResult SurfaceHit;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(DC1ResourceDropOrigin), true);

	// 공격자 높이에서 Resource 피벗 축으로 수평 -> 나무처럼 높은 Resource Spawn 위치 맞춤
	// 범위 중심이 아니라 피벗 - 가지가 한쪽으로 쏠린 나무는 범위 중심이 줄기 밖이라 빗나간다
	// 높이는 Resource 범위 안으로 - 공격자가 더 높거나 낮아도 표면을 지남
	const FVector CauserLocation = Causer->GetActorLocation();
	const FVector Pivot = ResourceMesh->GetComponentLocation();
	const float TraceZ = FMath::Clamp(CauserLocation.Z, Bounds.Origin.Z - Bounds.BoxExtent.Z, Bounds.Origin.Z + Bounds.BoxExtent.Z);
	const FVector Start(CauserLocation.X, CauserLocation.Y, TraceZ);
	const FVector End(Pivot.X, Pivot.Y, TraceZ);

	if (true == ResourceMesh->LineTraceComponent(SurfaceHit, Start, End, Params))
	{
		return SurfaceHit.ImpactPoint + SurfaceHit.ImpactNormal * DropSurfaceOffset;
	}

	// 수평으로 빗나가면 ( 중심축이 비어 있는 모양 ) Resource 중심으로 한 번 더
	if (true == ResourceMesh->LineTraceComponent(SurfaceHit, CauserLocation, Bounds.Origin, Params))
	{
		return SurfaceHit.ImpactPoint + SurfaceHit.ImpactNormal * DropSurfaceOffset;
	}

	return Fallback;
}

void ADC1ResourceActor::AccumulateDrop(float InDamage, const FVector& InOrigin)
{
	// 회복 GE
	if (InDamage <= 0.f)
	{
		return;
	}

	AccumulatedDropDamage += InDamage;

	const int32 DropTimes = FMath::FloorToInt32(AccumulatedDropDamage / DropDamage);
	AccumulatedDropDamage -= DropTimes * DropDamage;

	UE_LOG(LogTemp, Log, TEXT("[DC1] %s drop : damage %.1f -> %d drop(s), remain %.1f"), *GetName(), InDamage, DropTimes, AccumulatedDropDamage);

	// 맞은 지점 ( 공격자 쪽 표면 ) 에서 위로 간격만 둠 - 이후 물리로 떨어지며 겹치면 살짝만 밀림
	for (int32 Index = 0; Index < DropTimes; ++Index)
	{
		const FVector Location = InOrigin + FVector::UpVector * (Index * DropStackSpacing);
		ADC1PickupItem::SpawnDrop(GetWorld(), DropItemID, DropCountPerDrop, Location);
	}
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

	// 클라 첫 복제 전 ( Dormant 중 ) 은 0 나누기
	if (CurrentMaxHealth <= 0.f)
	{
		return 1.f;
	}

	const float CurrentHealth = AbilitySystemComponent->GetNumericAttribute(UDC1AttributeSet::GetHealthAttribute());
	return FMath::Clamp(CurrentHealth / CurrentMaxHealth, 0.f, 1.f);
}

void ADC1ResourceActor::ClearPalWorkers()
{
	if (true == PalWorkers.IsEmpty())
	{
		return;
	}

	PalWorkers.Empty();
	OnPalWorkersChanged.Broadcast();
}

void ADC1ResourceActor::DebugAssignNearbyPals()
{
	// TEMP v3 - ST 대신 배정 → SetWorkTarget → Work GA. 이미 일하는 펠은 GA 활성 실패로 그냥 지나감 ( ST 완성 후 삭제 )
	if (EDC1ResourceActorState::Active != ResourceState)
	{
		return;
	}

	const FVector Center = GetActorLocation();
	const float RadiusSq = FMath::Square(DebugAssignRadius);

	for (TActorIterator<ADC1PalCharacter> It(GetWorld()); It; ++It)
	{
		ADC1PalCharacter* Pal = *It;
		if (false == ::IsValid(Pal) || FVector::DistSquared(Center, Pal->GetActorLocation()) > RadiusSq)
		{
			continue;
		}

		if (false == AssignPalWorker(Pal))
		{
			UE_LOG(LogTemp, Warning, TEXT("[DC1] %s DebugAssign Rejected - [%s]"), *GetName(), *GetNameSafe(Pal));
			continue;
		}

		Pal->SetWorkTarget(this);

		// 프로젝트 규약 - ActivationTag( 스펙 동적 태그 ) 캐시로 실행. 엔진 TryActivateAbilitiesByTag는 GA 에셋 태그만 본다
		UDC1AbilitySystemComponent* ASC = Cast<UDC1AbilitySystemComponent>(Pal->GetAbilitySystemComponent());
		if (false == ::IsValid(ASC) || true == ASC->HasMatchingGameplayTag(DC1StateTags::State_Working))
		{
			continue;
		}

		if (false == ASC->ActivateAbilityByTag(DC1PalTags::Ability_Pal_Work))
		{
			UE_LOG(LogTemp, Warning, TEXT("[DC1] %s DebugAssign GA Failed - [%s] ( DA_Ability_Pal에 Ability.Pal.Work 항목 확인 )"), *GetName(), *GetNameSafe(Pal));
		}
	}
}
